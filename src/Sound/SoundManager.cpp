
#include "SoundManager.h"

#include <cassert>
#include <al.h>

const unsigned SoundManager::maxSources = 256;
SoundManager::LogicalSource SoundManager::invalidSource = { glm::vec3(0.0f), 0.0f, INT_MIN, false };

SoundManager::SoundManager()
	: device(nullptr)
{ }

SoundManager::~SoundManager()
{
	if (context != nullptr) {
		alcDestroyContext(context);
	}

	if (device != nullptr) {
		alcCloseDevice(device);
	}
}

bool SoundManager::initialize()
{
	device = alcOpenDevice(NULL);
	if (alGetError() != AL_NO_ERROR) {
		return false;
	}

	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
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
	}

	return true;
}

void SoundManager::setListenerTransform(const Transform& transform)
{
	listenerTransform.setPosition(transform.getPosition());
	listenerTransform.setRotation(transform.getRotation());
}

unsigned SoundManager::getSourceHandle()
{
	LogicalSource source;
	source.position = glm::vec3(0.0f);
	source.priority = 0;
	source.volume = 1.0f;
	return sourcePool.getNewHandle(source);
}

void SoundManager::setSourcePosition(unsigned handle, glm::vec3 position)
{
	LogicalSource& source = sourcePool.get(handle).value_or(invalidSource);
	source.position = position;
	source.dirty = true;
}

void SoundManager::setSourceVolume(unsigned handle, float volume)
{
	LogicalSource& source = sourcePool.get(handle).value_or(invalidSource);
	source.volume = volume;
	source.dirty = true;
}

void SoundManager::setSourcePriority(unsigned handle, int priority)
{
	LogicalSource& source = sourcePool.get(handle).value_or(invalidSource);
	source.priority = priority;
	source.dirty = true;
}

unsigned SoundManager::playClipAtSource(AudioClip clip, unsigned sourceHandle)
{
	LogicalSource& logicalSource = sourcePool.get(sourceHandle).value_or(invalidSource);

	// Get a free source
	bool sourceFound = false;
	unsigned sourceIndex = UINT_MAX;

	if (freeSources.size() > 0) {
		sourceFound = true;
		sourceIndex = freeSources.back();
		freeSources.pop_back();
		printf("Using free source %ud\n", sourceIndex);
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
		return UINT_MAX;
	}

	Source& source = sources[sourceIndex];

	ALuint alSource = source.alSource;
	alSourcei(alSource, AL_BUFFER, clip.buffer);

	source.logicalSourceHandle = sourceHandle;
	source.playing = true;
	source.startPlaying = true;
	source.clipHandle = clipPool.getNewHandle(sourceIndex);

	return source.clipHandle;
}

void SoundManager::update()
{
	alcSuspendContext(context);
	for (unsigned i = 0; i < sources.size(); i++) {
		Source& source = sources[i];

		if (!source.playing) {
			continue;
		}

		ALuint alSource = source.alSource;

		LogicalSource& logicalSource = sourcePool.get(sources[i].logicalSourceHandle).value_or(invalidSource);
		if (logicalSource.dirty || source.startPlaying) {
			alSourcef(alSource, AL_GAIN, logicalSource.volume);
			alSource3f(alSource, AL_POSITION, logicalSource.position.x, logicalSource.position.y, logicalSource.position.z);
		}

		if (source.startPlaying) {
			alSourcePlay(alSource);
			source.startPlaying = false;
		}

		ALint sourceState = -1;
		alGetSourcei(alSource, AL_SOURCE_STATE, &sourceState);

		if (sourceState == AL_STOPPED) {
			source.playing = false;
			freeSources.push_back(i);
			clipPool.freeHandle(source.clipHandle);
		}
	}

	for (auto iter = sourcePool.begin(); iter != sourcePool.end(); ++iter) {
		iter->second.dirty = false;
	}

	alListener3f(AL_POSITION, listenerTransform.getPosition().x, listenerTransform.getPosition().y, listenerTransform.getPosition().z);

	glm::vec3 at = listenerTransform.getForward();
	ALfloat orientation[6] = { at.x, at.y, at.z, 0.0f, 1.0f, 0.0f };
	alListenerfv(AL_ORIENTATION, orientation);

	alcProcessContext(context);
}

void SoundManager::stopClip(unsigned clipHandle)
{
	std::experimental::optional<std::reference_wrapper<size_t>> clipSourceOpt = clipPool.get(clipHandle);
	if (clipSourceOpt) {
		Source& source = sources[*clipSourceOpt];

		alSourceStop(source.alSource);
		source.playing = false;
		freeSources.push_back(*clipSourceOpt);

		clipPool.freeHandle(clipHandle);
	}
}

bool SoundManager::clipValid(unsigned clipHandle)
{
	return (bool)clipPool.get(clipHandle);
}
