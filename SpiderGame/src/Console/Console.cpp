
#include "Console.h"

#include "Transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

const float Console::xPadding = 5.0f;
const float Console::yPadding = 5.0f;
const float Console::linePadding = 2.0f;
const unsigned int Console::maxBufferLines = 30;
const unsigned int Console::maxLineSize = 250;
const glm::vec4 Console::textColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
const glm::vec4 Console::backColor = glm::vec4(0.0f, 0.7f, 0.2f, 0.5f);

Console::Console(std::shared_ptr<Font> font, glm::vec2 size)
	: size(size), font(font),
	buffer(maxBufferLines), bufferEnd(0), numBufferedLines(0),
	backQuad(std::make_shared<UIQuad>(backColor, size)), inputLabel(std::make_shared<Label>(font)),
	bufferLabels(maxBufferLines), historyPointer(0)
{
	lineHeight = 15.0f;
	for (unsigned i = 0; i < bufferLabels.size(); i++) {
		bufferLabels[i] = std::make_shared<Label>(font);
	}

	this->history.push_back("");
	this->repositionLabels();
	this->updateInputLabel();
	this->setVisible(false);
}

void Console::addCallback(const std::string& functionName, Callback callback)
{
	callbackMap.setCallback(functionName, callback);
}

void Console::inputChar(char c)
{
	this->history[historyPointer].push_back(c);
	this->updateInputLabel();
}

void Console::backspace()
{
	if (!history[historyPointer].empty()) {
		this->history.back().pop_back();
		this->updateInputLabel();
	}
}

void Console::endLine()
{
	const std::string& command = this->history[historyPointer];
	if (command.empty()) {
		return;
	}

	// Make it show up in the history
	this->print(command);

	// Do a simple parse of the command
	size_t firstSpace = command.find_first_of(' ');
	std::string functionName;
	std::string args;
	if (firstSpace == std::string::npos) {
		functionName = command;
		args = "";
	} else {
		functionName = command.substr(0, firstSpace);
		args = command.substr(firstSpace);
	}

	// Execute the command
	CallbackMap::Error error = callbackMap.call(functionName, args);

	if (error == CallbackMap::CALLBACK_NOT_FOUND) {
		this->print("Error: Didn't find callback with name " + functionName);
	} else if (error == CallbackMap::CALLBACK_BAD_ARGS) {
		this->print("Error: Bad arguments passed to function " + functionName);
	}

	// If we're back in the history, update the latest command
	if (historyPointer != this->history.size()-1) {
		this->history.back() = command;
	}

	// Start anew
	this->history.push_back("");
	historyPointer = this->history.size() - 1;
	this->updateInputLabel();
}

void Console::recallHistory(bool up)
{
	if (up && historyPointer > 0) {
		--historyPointer;
	} else if (!up) {
		historyPointer = std::min(historyPointer+1, history.size()-1);
	}
	this->updateInputLabel();
}

void Console::addToRenderer(UIRenderer& uiRenderer, Shader backShader, Shader textShader)
{
	UIHandles.push_back(uiRenderer.getEntityHandle(backQuad, backShader));
	UIHandles.push_back(uiRenderer.getEntityHandle(inputLabel, textShader));
	for (unsigned i = 0; i < bufferLabels.size(); i++) {
		UIHandles.push_back(uiRenderer.getEntityHandle(bufferLabels[i], textShader));
	}
}

void Console::setVisible(bool visible)
{
	this->visible = visible;

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

bool Console::isVisible()
{
	return this->visible;
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
	this->inputLabel->transform = Transform(glm::vec3(this->xPadding, bottom, 1.0f)).matrix();

	// This rotates the labels' positions.
	// We could keep the labels at the same position and rotate the text, but setting
	// text is a more expensive operation than setting position.
	for (unsigned i = 0; i < numBufferedLines; i++) {
		bottom -= linePadding + lineHeight;
		int index = (this->buffer.size() * 2 + (this->bufferEnd - 1 - i)) % this->buffer.size();
		this->bufferLabels[index]->transform = Transform(glm::vec3(this->xPadding, bottom, 1.0f)).matrix();
	}
}

void Console::updateInputLabel()
{
	this->inputLabel->setText("> " + history[historyPointer]);
}
