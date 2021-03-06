
#include "Sound/SoundManager.h"

#include <cassert>
#include <al.h>
#include <alc.h>

const unsigned SoundManager::maxSources = 256;
LogicalSource SoundManager::invalidSource = { glm::vec3(0.0f), 0.0f, INT_MIN, false };

struct SoundManager::Impl {
	Impl::Impl() : device(nullptr), context(nullptr) { }
	ALCdevice* device;
	ALCcontext* context;
};

struct SoundManager::Source
{
	ALuint alSource;
	SourceHandle logicalSourceHandle;
	ClipHandle clipHandle;
	bool startPlaying;
	bool playing;
};

SoundManager::SoundManager()
	: impl(new Impl), listenerVolume(0.99f)
{ }

SoundManager::~SoundManager()
{
	if (impl->context != nullptr) {
		alcDestroyContext(impl->context);
	}

	if (impl->device != nullptr) {
		alcCloseDevice(impl->device);
	}
}

bool SoundManager::initialize()
{
	impl->device = alcOpenDevice(NULL);
	if (alGetError() != AL_NO_ERROR) {
		return false;
	}

	impl->context = alcCreateContext(impl->device, NULL);
	alcMakeContextCurrent(impl->context);
	if (alGetError() != AL_NO_ERROR) {
		return false;
	}

	std::vector<ALuint> alSources;
	sourceCount = maxSources;
	do {
		alSources.resize(sourceCount);
		alGenSources(sourceCount, alSources.data());
		sourceCount /= 2;
	} while(sourceCount != 0 && alGetError() != AL_NO_ERROR);

	if (sourceCount <= 1) {
		return false;
	}

	sourceCount *= 2;
	sources.resize(sourceCount);

	for (size_t i = 0; i < sourceCount; i++) {
		freeSources.push_back(i);

		sources[i].alSource = alSources[i];
		sources[i].logicalSourceHandle = 0;
		sources[i].playing = false;
		sources[i].startPlaying = false;

		alSourcef(alSources[i], AL_REFERENCE_DISTANCE, 2.5f);
	}

	return true;
}

void SoundManager::setListenerTransform(const glm::vec3& position, const glm::quat& rotation)
{
	this->listenerPosition = position;
	this->listenerRotation = rotation;
}

SoundManager::SourceHandle SoundManager::getSourceHandle()
{
	LogicalSource source;
	source.position = glm::vec3(0.0f);
	source.priority = 0;
	source.volume = 1.0f;
	source.rolloffFactor = 1.0f;
	source.dirty = true;
	return sourcePool.getNewHandle(source);
}

void SoundManager::setSourcePosition(const SourceHandle& handle, glm::vec3 position)
{
	LogicalSource& source = sourcePool.get(handle).value_or(invalidSource);
	if (source.position != position) {
		source.position = position;
		source.dirty = true;
	}
}

void SoundManager::setSourceVolume(const SourceHandle& handle, float volume)
{
	LogicalSource& source = sourcePool.get(handle).value_or(invalidSource);
	if (source.volume != volume) {
		source.volume = volume;
		source.dirty = true;
	}
}

void SoundManager::setSourcePriority(const SourceHandle& handle, int priority)
{
	LogicalSource& source = sourcePool.get(handle).value_or(invalidSource);
	if (source.priority != priority) {
		source.priority = priority;
		source.dirty = true;
	}
}

void SoundManager::setListenerVolume(float volume)
{
	this->listenerVolume = volume;
}

SoundManager::ClipHandle SoundManager::playClipAtSource(const AudioClip& clip, const SourceHandle& sourceHandle, bool loop)
{
	LogicalSource& logicalSource = sourcePool.get(sourceHandle).value_or(invalidSource);

	// Get a free source
	bool sourceFound = false;
	unsigned sourceIndex = UINT_MAX;

	if (freeSources.size() > 0) {
		sourceFound = true;
		sourceIndex = freeSources.back();
		freeSources.pop_back();
	} else {
		// No free sources, see if we can override one
		for (unsigned i = 0; i <= sources.size(); i++) {
			LogicalSource& otherSource = sourcePool.get(sources[i].logicalSourceHandle).value_or(invalidSource);
			if (otherSource.priority < logicalSource.priority) {
				sourceFound = true;
				sourceIndex = i;
				break;
			}
		}
		printf("Overwriting source %ud\n", sourceIndex);
	}

	if (!sourceFound) {
		return clipPool.invalidHandle;
	}

	Source& source = sources[sourceIndex];

	ALuint alSource = source.alSource;
	alSourcei(alSource, AL_BUFFER, clip.buffer);

	if (loop) {
		alSourcei(alSource, AL_LOOPING, 1);
	}

	source.logicalSourceHandle = sourceHandle;
	source.playing = true;
	source.startPlaying = true;
	source.clipHandle = clipPool.getNewHandle(sourceIndex);

	return source.clipHandle;
}

void SoundManager::update()
{
	alcSuspendContext(impl->context);
	for (unsigned i = 0; i < sources.size(); i++) {
		Source& source = sources[i];

		if (!source.playing) {
			continue;
		}

		ALuint alSource = source.alSource;

		LogicalSource& logicalSource = sourcePool.get(sources[i].logicalSourceHandle).value_or(invalidSource);
		if (logicalSource.dirty || source.startPlaying) {
			alSourcef(alSource, AL_GAIN, logicalSource.volume);
			alSourcef(alSource, AL_ROLLOFF_FACTOR, logicalSource.rolloffFactor);
			alSource3f(alSource, AL_POSITION, logicalSource.position.x, logicalSource.position.y, logicalSource.position.z);
		}

		if (source.startPlaying) {
			alSourcePlay(alSource);
			source.startPlaying = false;
		}

		ALint sourceState = -1;
		alGetSourcei(alSource, AL_SOURCE_STATE, &sourceState);

		if (sourceState == AL_STOPPED) {
			freeSource(i);
		}
	}

	for (auto iter = sourcePool.begin(); iter != sourcePool.end(); ++iter) {
		iter->second.dirty = false;
	}

	alListener3f(AL_POSITION, listenerPosition.x, listenerPosition.y, listenerPosition.z);

	glm::vec3 at = listenerRotation * glm::vec3(0.0f, 0.0f, -1.0f);
	ALfloat orientation[6] = { at.x, at.y, at.z, 0.0f, 1.0f, 0.0f };
	alListenerfv(AL_ORIENTATION, orientation);

	if (listenerVolume != FLT_MAX) {
		alListenerf(AL_GAIN, listenerVolume);
		listenerVolume = FLT_MAX;
	}

	alcProcessContext(impl->context);
}

void SoundManager::stopClip(const ClipHandle& clipHandle)
{
	size_t invalidClip = sources.size();
	size_t clipSource = clipPool.get(clipHandle).value_or(invalidClip);
	if (clipSource < sources.size()) {
		Source& source = sources[clipSource];
		alSourceStop(source.alSource);
		freeSource(clipSource);
	}
}

void SoundManager::freeSource(unsigned sourceIndex)
{
	Source& source = sources[sourceIndex];
	source.playing = false;
	freeSources.push_back(sourceIndex);

	// If anyone tries to stop this clip from playing in the future, noop it
	*source.clipHandle = sources.size();
	source.logicalSourceHandle = sourcePool.invalidHandle;
	source.clipHandle = clipPool.invalidHandle;
}

bool SoundManager::clipValid(const ClipHandle& clipHandle)
{
	return (bool)clipPool.get(clipHandle);
}

void SoundManager::stopAllClips()
{
	for (unsigned i = 0; i < sources.size(); ++i) {
		alSourceStop(sources[i].alSource);
		sources[i].startPlaying = false;
	}
}
