
#include "Sound/AudioClip.h"

#include <sndfile.h>

#include <al.h>

#include <memory>
#include <array>
#include <cassert>

AudioClip::AudioClip()
{
	buffer = AL_INVALID_VALUE;
}

AudioClip::AudioClip(const std::string& fileName)
{
	SF_INFO finfo;
	std::unique_ptr<SNDFILE, int(*)(SNDFILE*)> file(sf_open(fileName.c_str(), SFM_READ, &finfo), &sf_close);

	// TODO: This is pretty terrible (though effective!)
	if (file == nullptr) {
		throw "Audio file couldn't be loaded: " + fileName;
	}

	std::vector<uint16_t> data;
	std::array<int16_t, 4096> readBuf;

	sf_count_t readSize = 0;
	while((readSize = sf_read_short(file.get(), readBuf.data(), readBuf.size())) != 0)
	{
		data.insert(data.end(), readBuf.begin(), readBuf.begin() + (size_t)readSize);
	}

	alGenBuffers(1, &buffer);
	assert(buffer != AL_INVALID_VALUE);

	alBufferData(buffer, finfo.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
		data.data(), data.size() * sizeof(uint16_t), finfo.samplerate);
}