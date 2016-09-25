#pragma once

#include <string>
#include <vector>

struct AudioClip
{
	AudioClip();
	AudioClip(const std::string& file);
	unsigned buffer;
};