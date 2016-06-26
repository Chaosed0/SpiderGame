#pragma once

#include <glm/glm.hpp>

#include <SDL.h>

#include <vector>
#include <random>

struct RoomSide
{
	int x0, y0, x1, y1;
	glm::ivec2 normal;
};

struct Box
{
	int right;
	int top;
	int left;
	int bottom;
};

struct Room
{
	std::vector<RoomSide> sides;
	std::vector<Box> boxes;
	int minX, minY;
	int maxX, maxY;
	SDL_Surface* saveToSurface();
};

class RoomGenerator
{
public:
	RoomGenerator(int seed);
	Room generate();
private:
	std::default_random_engine generator;
};