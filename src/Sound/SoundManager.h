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

	void playClipAtSource(AudioClip clip, unsigned sourceHandle);

	void update();
private:
	struct LogicalSource
	{
		glm::vec3 position;
		float volume;
		int priority;
	};

	struct Source
	{
		ALuint alSource;
		unsigned logicalSourceHandle;
		bool wasPlaying;
		bool playing;
	};

	ALCdevice* device;
	ALCcontext* context;

	HandlePool<LogicalSource> sourcePool;
	std::vector<Source> sources;

	std::deque<size_t> freeSources;
	unsigned sourceCount;

	const static unsigned maxSources;
};