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

struct RoomBox
{
	int right;
	int top;
	int left;
	int bottom;
};

struct Room
{
	std::vector<RoomSide> sides;
	std::vector<RoomBox> boxes;
	int minX, minY;
	int maxX, maxY;
	int rightmostBox, leftmostBox;
	int topmostBox, bottommostBox;
	SDL_Surface* saveToSurface();
};

class RoomGenerator
{
public:
	RoomGenerator();
	RoomGenerator(int seed);
	Room generate();

	int minimumArea;
private:
	std::default_random_engine generator;
};