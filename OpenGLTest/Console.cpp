
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

const float Console::xPadding = 5.0f;
const float Console::yPadding = 2.0f;
const float Console::linePadding = 2.0f;
const unsigned int Console::maxBufferLines = 30;

struct Character {
	GLuint     textureID;  // ID handle of the glyph texture
	glm::ivec2 size;       // Size of glyph
	glm::ivec2 bearing;    // Offset from baseline to left/top of glyph
	FT_Pos     advance;    // Offset to advance to next glyph
};

static std::unordered_map<char, Character> characters;

void loadCharacters(const std::string& fontPath, int height)
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "ERROR::FREETYPE: Could not init FreeType Library");
		return;
	}

	FT_Face face;
	if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
		fprintf(stderr, "ERROR::FREETYPE: Failed to load font %s\n", fontPath.c_str());
		return;
	}

	FT_Set_Pixel_Sizes(face, 0, height);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

	for (int i = 0; i < 128; i++)
	{
		char c = (char)i;
		// Load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			fprintf(stderr, "ERROR::FREETYTPE: Failed to load Glyph %c", c);
			continue;
		}

		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
			);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		characters.insert(std::pair<GLchar, Character>(c, character));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

Console::Console(float width, float height, float windowWidth, float windowHeight)
	: width(width), height(height), bottom(windowHeight - height), buffer(maxBufferLines)
{
	if (characters.empty()) {
		loadCharacters("assets/font/Inconsolata.otf", 12);
	}

	lineHeight = FLT_MIN;
	for (unsigned int i = 0; i < characters.size(); i++) {
		lineHeight = std::max(lineHeight, (float)characters[i].size.y);
	}

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
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

	backShader.compileAndLink("back.vert", "back.frag");
	backShader.use();
	glUniform3f(backShader.getUniformLocation("backColor"), 0.2f, 0.8f, 0.2f);
	glUniformMatrix4fv(backShader.getUniformLocation("projection"), 1, GL_FALSE, &projection[0][0]);

	textShader.compileAndLink("text.vert", "text.frag");
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
	this->input << c;
}

void Console::endLine()
{
	std::string line = this->input.str();
	if (line.empty()) {
		return;
	}

	this->print(line);
	this->input.str("");

	size_t firstSpace = line.find_first_of(' ');
	std::string functionName;
	std::string args;
	if (firstSpace == std::string::npos) {
		functionName = line;
		args = "";
	} else {
		functionName = line.substr(0, firstSpace);
		args = line.substr(firstSpace);
	}

	CallbackMap::Error error = callbackMap.call(functionName, args);

	if (error == CallbackMap::CALLBACK_NOT_FOUND) {
		print("Error: Didn't find callback with name " + functionName);
	} else if (error == CallbackMap::CALLBACK_BAD_ARGS) {
		print("Error: Bad arguments passed to function " + functionName);
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
	std::string inputLine = input.str();
	drawLine("> " + inputLine, (unsigned int)(this->bottom + yPadding + lineHeight));
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
	unsigned int x = (unsigned int)xPadding;
	unsigned int y = top;
	float scale = 1.1f;
	for (unsigned int i = 0; i < line.size(); i++) {
		Character ch = characters[line[i]];

		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, glyphQuad);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
}