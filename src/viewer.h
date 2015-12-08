#pragma once

#include <nanogui\nanogui.h>
#include <nanogui\glutil.h>

using namespace nanogui;

class Viewer : public Screen {
public:
	Viewer();
	virtual ~Viewer();

	virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);
	virtual void draw(NVGcontext *ctx);
	virtual void drawContents();

private:
	GLShader mShader;
};