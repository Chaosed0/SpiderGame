
#include "Console.h"

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
const glm::vec4 Console::textColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
const glm::vec4 Console::backColor = glm::vec4(0.0f, 0.7f, 0.2f, 0.5f);

Console::Console(std::shared_ptr<Font> font, glm::vec2 size)
	: size(size), font(font),
	buffer(maxBufferLines), bufferEnd(0), numBufferedLines(0),
	backQuad(std::make_shared<UIQuad>(size)), inputLabel(std::make_shared<Label>(font)),
	bufferLabels(maxBufferLines)
{
	backQuad->material.setProperty("color", backColor);
	lineHeight = 15.0f;
	for (unsigned i = 0; i < bufferLabels.size(); i++) {
		bufferLabels[i] = std::make_shared<Label>(font);
	}

	this->repositionLabels();
	this->inputLabel->setText("> ");
}

void Console::addCallback(const std::string& functionName, Callback callback)
{
	callbackMap.setCallback(functionName, callback);
}

void Console::inputChar(char c)
{
	this->input.push_back(c);
	this->inputLabel->setText("> " + this->input);
}

void Console::backspace()
{
	if (!input.empty()) {
		input.pop_back();
	}
	this->inputLabel->setText("> " + this->input);
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
	this->inputLabel->setText("> ");
}

void Console::addToRenderer(UIRenderer& uiRenderer, Shader backShader, Shader textShader)
{
	uiRenderer.getEntityHandle(backQuad, backShader);
	uiRenderer.getEntityHandle(inputLabel, textShader);
	for (unsigned i = 0; i < bufferLabels.size(); i++) {
		uiRenderer.getEntityHandle(bufferLabels[i], textShader);
	}
}

void Console::setVisible(bool visible)
{
	float alpha = 0.0f;
	if (visible) {
		alpha = 1.0f;
	}

	glm::vec4 localTextColor(textColor);
	glm::vec4 localBackColor(backColor);
	localTextColor.a *= alpha;
	localBackColor.a *= alpha;

	backQuad->material.setProperty("color", localBackColor);
	inputLabel->material.setProperty("textColor", localTextColor);
	for (unsigned i = 0; i < bufferLabels.size(); i++) {
		bufferLabels[i]->material.setProperty("textColor", localTextColor);
	}
}

void Console::print(const std::string& message)
{
	this->buffer[this->bufferEnd] = message;
	this->bufferLabels[this->bufferEnd]->setText(message);
	this->bufferEnd = (this->bufferEnd+1)%buffer.size();
	this->numBufferedLines = std::min((unsigned int)this->buffer.size(), this->numBufferedLines+1);

	this->repositionLabels();
}

void Console::repositionLabels()
{
	float bottom = size.y - yPadding;
	this->inputLabel->transform.setPosition(glm::vec3(this->xPadding, bottom, 0.0f));

	for (unsigned i = 0; i < numBufferedLines; i++) {
		bottom -= linePadding + lineHeight;
		int index = (this->buffer.size() * 2 + (this->bufferEnd - 1 - i)) % this->buffer.size();
		this->bufferLabels[index]->transform.setPosition(glm::vec3(this->xPadding, bottom, 0.0f));
	}
}
