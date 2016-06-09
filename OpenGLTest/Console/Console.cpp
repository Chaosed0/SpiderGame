
#include "Console.h"

#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ft2build.h>
#include <freetype/freetype.h>

#include <algorithm>

#include "Font.h"

const float Console::xPadding = 5.0f;
const float Console::yPadding = 2.0f;
const float Console::linePadding = 2.0f;
const unsigned int Console::maxBufferLines = 30;

Console::Console(float width, float height, float windowWidth, float windowHeight)
	: width(width), height(height), bottom(windowHeight - height), buffer(maxBufferLines)
{
	Font font;
	font.loadCharacters("assets/font/Inconsolata.otf", 50);
}

void Console::addCallback(const std::string& functionName, Callback callback)
{
	callbackMap.setCallback(functionName, callback);
}

void Console::inputChar(char c)
{
	this->input.push_back(c);
}

void Console::endLine()
{
	if (input.empty()) {
		return;
	}

	this->print(input);

	size_t firstSpace = input.find_first_of(' ');
	std::string functionName;
	std::string args;
	if (firstSpace == std::string::npos) {
		functionName = input;
		args = "";
	} else {
		functionName = input.substr(0, firstSpace);
		args = input.substr(firstSpace);
	}

	CallbackMap::Error error = callbackMap.call(functionName, args);

	if (error == CallbackMap::CALLBACK_NOT_FOUND) {
		print("Error: Didn't find callback with name " + functionName);
	} else if (error == CallbackMap::CALLBACK_BAD_ARGS) {
		print("Error: Bad arguments passed to function " + functionName);
	}

	this->input.clear();
}

void Console::backspace()
{
	if (!input.empty()) {
		input.pop_back();
	}
}

void Console::print(const std::string& message)
{
	this->buffer[this->bufferEnd] = message;
	this->bufferEnd = (this->bufferEnd+1)%buffer.size();
	this->numBufferedLines = std::min(this->buffer.size(), this->numBufferedLines+1);
}

void Console::draw()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	backShader.use();
	glBindVertexArray(backVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	// Activate corresponding render state	
	textShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(glyphVao);

	// Iterate through all characters
	drawLine("> " + input, (unsigned int)(this->bottom + yPadding + lineHeight));
	for (unsigned int i = 0; i < numBufferedLines; i++) {
		int index = bufferEnd-i-1;
		if (index < 0) {
			index = buffer.size() + index;
		}
		float top = this->bottom + yPadding + (i+2) * (linePadding + lineHeight);
		drawLine(buffer[index], (unsigned int)top);
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Console::drawLine(std::string line, unsigned int top)
{
}