#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <sstream>

#include "Renderer/Shader.h"
#include "Callbacks.h"
#include "Renderer/UI/Font.h"
#include "Renderer/UI/Label.h"
#include "Renderer/UI/UIQuad.h"
#include "Renderer/UI/UIRenderer.h"

class Console
{
public:
	Console(std::shared_ptr<Font> font, glm::vec2 size);

	void addCallback(const std::string& functionName, Callback callback);
	void inputChar(char c);

	void backspace();
	void endLine();
	void setVisible(bool visible);

	void addToRenderer(UIRenderer& uiRenderer, Shader backShader, Shader textShader);

	void print(const std::string& message);
private:
	std::string input;
	std::vector<std::string> buffer;

	std::shared_ptr<Font> font;
	std::shared_ptr<Label> inputLabel;
	std::vector<std::shared_ptr<Label>> bufferLabels;

	std::shared_ptr<UIQuad> backQuad;

	CallbackMap callbackMap;

	unsigned int bufferEnd, numBufferedLines;

	glm::vec2 size;
	float lineHeight;

	Shader textShader;
	Shader backShader;

	const static float xPadding;
	const static float yPadding;
	const static float linePadding;
	const static unsigned int maxBufferLines;
	const static unsigned int maxLineSize;
	const static glm::vec4 textColor;
	const static glm::vec4 backColor;

	void repositionLabels();
};