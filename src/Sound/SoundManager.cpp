
#include "SoundManager.h"

#include <cassert>
#include <al.h>

const unsigned SoundManager::maxSources = 256;

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
		sources[i].wasPlaying = false;
	}

	return true;
}

void SoundManager::setListenerTransform(const Transform& transform)
{
	alListener3f(AL_POSITION, transform.getPosition().x, transform.getPosition().y, transform.getPosition().z);

	glm::vec3 at = transform.getPosition() + transform.getForward();
	ALfloat orientation[6] = { at.x, at.y, at.z, 0.0f, 1.0f, 0.0f };
	alListenerfv(AL_ORIENTATION, orientation);
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
	std::experimental::optional<LogicalSource&> source = sourcePool.get(handle);
	if (!source) {
		return;
	}

	source->position = position;
}

void SoundManager::setSourceVolume(unsigned handle, float volume)
{
	std::experimental::optional<LogicalSource&> source = sourcePool.get(handle);
	if (!source) {
		return;
	}

	source->volume = volume;
}

void SoundManager::setSourcePriority(unsigned handle, int priority)
{
	std::experimental::optional<LogicalSource&> source = sourcePool.get(handle);
	if (!source) {
		return;
	}

	source->priority = priority;
}

void SoundManager::playClipAtSource(AudioClip clip, unsigned sourceHandle)
{
	std::experimental::optional<LogicalSource&> sourceOpt = sourcePool.get(sourceHandle);
	if (!sourceOpt) {
		return;
	}

	LogicalSource& logicalSource = *sourceOpt;

	// Get a free source
	bool sourceFound = false;
	unsigned sourceIndex = 0;

	if (freeSources.size() > 0) {
		sourceFound = true;
		sourceIndex = freeSources.back();
		freeSources.pop_back();
	} else {
		// No free sources, see if we can override one
		for (unsigned i = 0; i <= sources.size(); i++) {
			std::experimental::optional<LogicalSource&> otherSourceOpt = sourcePool.get(sources[i].logicalSourceHandle);
			assert(otherSourceOpt);

			LogicalSource& otherSource = *otherSourceOpt;
			if (otherSource.priority < logicalSource.priority) {
				sourceIndex = i;
				sourceFound = true;
				break;
			}
		}
	}

	if (!sourceFound) {
		return;
	}

	Source& source = sources[sourceIndex];

	ALuint alSource = source.alSource;
	alSourcei(alSource, AL_BUFFER, clip.buffer);
	alSourcef(alSource, AL_GAIN, logicalSource.volume);
	alSource3f(alSource, AL_POSITION, logicalSource.position.x, logicalSource.position.y, logicalSource.position.z);
	alSourcePlay(alSource);

	source.logicalSourceHandle = sourceHandle;
	source.playing = true;
}

void SoundManager::update()
{
	for (unsigned i = 0; i < sources.size(); i++) {
		if (!sources[i].playing) {
			continue;
		}

		ALuint alSource = sources[i].alSource;
		ALint isPlaying = AL_FALSE;
		alGetSourcei(alSource, AL_PLAYING, &isPlaying);

		if (sources[i].wasPlaying && isPlaying == AL_FALSE) {
			sources[i].playing = false;
			freeSources.push_back(i);
		}
		sources[i].wasPlaying = (isPlaying == AL_TRUE);
	}
}
