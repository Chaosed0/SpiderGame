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
	glm::ivec2 getCenter();
};

struct Room
{
	std::vector<RoomSide> sides;
	std::vector<RoomBox> boxes;
	std::vector<std::vector<int>> boxAdjacencyList;
	int minX, minY;
	int maxX, maxY;
	int rightmostBox, leftmostBox;
	int topmostBox, bottommostBox;

	int boxForCoordinate(glm::vec2 coord);
	SDL_Surface* saveToSurface();
};

class RoomGenerator
{
public:
	RoomGenerator();
	RoomGenerator(int seed);
	Room generate();

	std::normal_distribution<float> boxSizeRand;

	unsigned minimumArea;
private:
	std::default_random_engine generator;
	unsigned getBoxSize();

	unsigned minBoxSize;
	unsigned approxMaxBoxSize;
};