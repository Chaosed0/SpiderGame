
#include "Console.h"

#include <Windows.h>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ft2build.h>
#include <freetype/freetype.h>

#include <algorithm>

const float Console::xPadding = 5.0f;
const float Console::yPadding = 2.0f;
const float Console::linePadding = 2.0f;
const unsigned int Console::maxBufferLines = 30;
const unsigned int Console::maxLineSize = 250;

Console::Console(float width, float height, float windowWidth, float windowHeight)
	: width(width), height(height), bottom(windowHeight - height), buffer(maxBufferLines)
{
	font.loadCharacters("assets/font/Inconsolata.otf", 12);
	font.saveAtlasToFile("font.bmp");

	lineHeight = 15.0f;
	bufferEnd = 0;
	numBufferedLines = 0;
	
	glm::vec3 backQuadVerts[6];
	backQuadVerts[0] = glm::vec3(0.0f, windowHeight - height, 0.0f);
	backQuadVerts[1] = glm::vec3(0.0f, windowHeight, 0.0f);
	backQuadVerts[2] = glm::vec3(width, windowHeight - height, 0.0f);
	backQuadVerts[3] = glm::vec3(0.0f, windowHeight, 0.0f);
	backQuadVerts[4] = glm::vec3(width, windowHeight, 0.0f);
	backQuadVerts[5] = glm::vec3(width, windowHeight - height, 0.0f);
	
	glGenVertexArrays(1, &glyphVao);
	glGenBuffers(1, &glyphQuad);
	glGenVertexArrays(1, &backVao);
	glGenBuffers(1, &backQuad);
	
	glBindVertexArray(glyphVao);
	glBindBuffer(GL_ARRAY_BUFFER, glyphQuad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4 * maxLineSize, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glBindVertexArray(backVao);
	glBindBuffer(GL_ARRAY_BUFFER, backQuad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(backQuadVerts), backQuadVerts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glm::mat4 projection = glm::ortho(0.0f, windowWidth, 0.0f, windowHeight);
	
	backShader.compileAndLink("Shaders/back.vert", "Shaders/singlecolor.frag");
	backShader.use();
	glUniform4f(backShader.getUniformLocation("material.color"), 0.2f, 0.8f, 0.2f, 0.5f);
	glUniformMatrix4fv(backShader.getUniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);
	
	textShader.compileAndLink("Shaders/text.vert", "Shaders/text.frag");
	textShader.use();
	glUniform3f(textShader.getUniformLocation("textColor"), 0.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(textShader.getUniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);

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
	this->numBufferedLines = std::min((unsigned int)this->buffer.size(), this->numBufferedLines+1);
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

	glBindTexture(GL_TEXTURE_2D, font.getTextureId());
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
}

void Console::drawLine(std::string line, unsigned int top)
{
	unsigned int x = (unsigned int)xPadding;
	unsigned int y = top;
	unsigned int lineSize = std::min((unsigned int)line.size(), maxLineSize);
	float scale = 1.0f;
	glm::ivec2 textureSize = font.getTextureSize();

	// six verts * four floats, per character
	GLfloat vertices[6 * 4 * maxLineSize];

	for (unsigned int i = 0; i < lineSize; i++) {
		Character ch = font.getCharacter(line[i]);

		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;

		// Normalize texture coordinates
		float texLeft = ch.origin.x / (float)textureSize.x;
		float texRight = (ch.origin.x + ch.size.x) / (float)textureSize.x;
		float texBot = ch.origin.y / (float)textureSize.y;
		float texTop = (ch.origin.y + ch.size.y) / (float)textureSize.y;
		GLfloat charVerts[24] = {
			xpos,     ypos + h,   texLeft, texBot,
			xpos,     ypos,       texLeft, texTop,
			xpos + w, ypos,       texRight, texTop,

			xpos,     ypos + h,   texLeft, texBot,
			xpos + w, ypos,       texRight, texTop,
			xpos + w, ypos + h,   texRight, texBot
		};

		memcpy(vertices + i*24, charVerts, sizeof(charVerts));

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (unsigned)((ch.advance >> 6) * scale); // Bitshift by 6 to get value in pixels (2^6 = 64)
	}

	// Update content of VBO memory
	glBindBuffer(GL_ARRAY_BUFFER, glyphQuad);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 24 * lineSize, vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Render quads
	glDrawArrays(GL_TRIANGLES, 0, lineSize * 6);
}