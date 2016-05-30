#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <sstream>

#include "Shader.h"

struct ConsoleCallback
{
	ConsoleCallback(std::function<void(std::vector<std::string>)> function, unsigned int args)
		: function(function), args(args) {}
	std::function<void(std::vector<std::string>)> function;
	unsigned int args;
};

class Console
{
public:
	Console(float width, float height, float windowWidth, float windowHeight);
	bool addCallback(const std::string& functionName, std::function<void(std::vector<std::string>)> function, int args);
	void inputChar(char c);
	void endLine();
	void draw();
private:
	std::stringstream input;
	std::vector<std::string> buffer;
	unsigned int bufferEnd, numBufferedLines;

	float width, height;
	float bottom;
	float lineHeight;

	std::unordered_map<std::string, ConsoleCallback> functionNameMap;
	Shader textShader;
	Shader backShader;
	unsigned int glyphVao, glyphQuad;
	unsigned int backVao, backQuad;

	const static float xPadding;
	const static float yPadding;
	const static float linePadding;
	const static unsigned int maxBufferLines;
	
	void drawLine(std::string line, unsigned int top);
};