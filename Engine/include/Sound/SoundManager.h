#pragma once

#include "Sound/AudioClip.h"
#include "HandlePool.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <deque>

struct LogicalSource
{
	glm::vec3 position;
	float volume;
	int priority;
	bool dirty;
};

class SoundManager
{
public:
	using SourceHandle = HandlePool<LogicalSource>::Handle;
	using ClipHandle = HandlePool<size_t>::Handle;

	SoundManager();
	~SoundManager();

	bool initialize();
	SourceHandle getSourceHandle();

	void setListenerTransform(const glm::vec3& position, const glm::quat& rotation);

	void setSourcePosition(const SourceHandle& handle, glm::vec3 position);
	void setSourceVolume(const SourceHandle& handle, float volume);
	void setSourcePriority(const SourceHandle& handle, int priority);

	void setListenerVolume(float volume);

	ClipHandle playClipAtSource(const AudioClip& clip, const SourceHandle& sourceHandle);
	void stopClip(const ClipHandle& clipHandle);
	bool clipValid(const ClipHandle& clipHandle);

	void update();
private:
	void freeSource(unsigned sourceIndex);

	HandlePool<LogicalSource> sourcePool;
	HandlePool<size_t> clipPool;

	struct Source;
	std::vector<Source> sources;

	std::deque<size_t> freeSources;
	unsigned sourceCount;
	glm::vec3 listenerPosition;
	glm::quat listenerRotation;
	float listenerVolume;

	struct Impl;
	std::unique_ptr<Impl> impl;

	const static unsigned maxSources;
	static LogicalSource invalidSource;
};