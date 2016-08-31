#pragma once

#include "Sound/AudioClip.h"
#include "HandlePool.h"
#include "Transform.h"

#include <glm/glm.hpp>
#include <memory>
#include <deque>

#include <al.h>
#include <alc.h>

class SoundManager
{
public:
	SoundManager();
	~SoundManager();

	bool initialize();
	unsigned getSourceHandle();

	void setListenerTransform(const Transform& transform);

	void setSourcePosition(unsigned handle, glm::vec3 position);
	void setSourceVolume(unsigned handle, float volume);
	void setSourcePriority(unsigned handle, int priority);

	unsigned playClipAtSource(AudioClip clip, unsigned sourceHandle);
	void stopClip(unsigned clipHandle);
	bool clipValid(unsigned clipHandle);

	void update();
private:
	struct LogicalSource
	{
		glm::vec3 position;
		float volume;
		int priority;
		bool dirty;
	};

	struct Source
	{
		ALuint alSource;
		unsigned logicalSourceHandle;
		unsigned clipHandle;
		bool startPlaying;
		bool playing;
	};

	ALCdevice* device;
	ALCcontext* context;

	HandlePool<LogicalSource> sourcePool;
	HandlePool<size_t> clipPool;
	std::vector<Source> sources;

	std::deque<size_t> freeSources;
	unsigned sourceCount;
	Transform listenerTransform;

	const static unsigned maxSources;
	static LogicalSource invalidSource;
};