#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <sstream>

#include "Renderer/Shader.h"
#include "Callbacks.h"
#include "Font.h"

class Console
{
public:
	Console(float width, float height, float windowWidth, float windowHeight);
	void addCallback(const std::string& functionName, Callback callback);
	void inputChar(char c);

	void backspace();
	void endLine();

	void draw();

	void print(const std::string& message);
private:
	std::string input;
	std::vector<std::string> buffer;
	Font font;
	unsigned int bufferEnd, numBufferedLines;

	float width, height;
	float bottom;
	float lineHeight;

	Shader textShader;
	Shader backShader;
	unsigned int glyphVao, glyphQuad;
	unsigned int backVao, backQuad;

	const static float xPadding;
	const static float yPadding;
	const static float linePadding;
	const static unsigned int maxBufferLines;
	
	void drawLine(std::string line, unsigned int top);

	CallbackMap callbackMap;
};