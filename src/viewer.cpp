#include "viewer.h"
#include "resources.h"

#include <iostream>
#include <string>

using std::cout;
using std::endl;

Viewer::Viewer() : Screen(Eigen::Vector2i(1024, 758), "Shell Maps Viewer") {
	/* ui */
	Window *window = new Window(this, "Load Mesh");
	window->setPosition(Eigen::Vector2i(15, 15));
	window->setLayout(new GroupLayout);

	std::string meshFileName = "1.png";

	Button *b = new Button(window, "Open");
	b->setCallback([&] {
//		cout << "Load mesh file: " << file_dialog(
//		{ {"obj", "..."}, }, false);
		meshFileName = file_dialog({ {"obj", "..."}, }, false);	// run-time error here, can not capute variables in this scope!!!
	});

	cout << meshFileName << std::endl;

	performLayout(mNVGContext);

	/* openGL */
	mShader.init("simple_shader",
		(const char *)shader_simple_vert,
		(const char *)shader_simple_frag);

	MatrixXu indices(3, 2); /* Draw 2 triangles */
	indices.col(0) << 0, 1, 2;
	indices.col(1) << 2, 3, 0;

	MatrixXf positions(3, 4);
	positions.col(0) << -1, -1, 0;
	positions.col(1) << 1, -1, 0;
	positions.col(2) << 1, 1, 0;
	positions.col(3) << -1, 1, 0;

	mShader.bind();
	mShader.uploadIndices(indices);
	mShader.uploadAttrib("position", positions);
	mShader.setUniform("intensity", 0.5f);
}

Viewer::~Viewer() {
	mShader.free();
}

bool Viewer::keyboardEvent(int key, int scancode, int action, int modifiers) {
	if (Screen::keyboardEvent(key, scancode, action, modifiers))
		return true;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		setVisible(false);
		return true;
	}
	return false;
}

void Viewer::draw(NVGcontext * ctx) {
	/* Animate the scrollbar */
//	mProgress->setValue(std::fmod((float)glfwGetTime() / 10, 1.0f));

	/* Draw the user interface */
	Screen::draw(ctx);
}

void Viewer::drawContents() {
	/* Draw the window contents using OpenGL */
	mShader.bind();

	Matrix4f mvp;
	mvp.setIdentity();
	mvp.topLeftCorner<3, 3>() = Matrix3f(Eigen::AngleAxisf((float)glfwGetTime(), Vector3f::UnitZ())) * 0.25f;

	mvp.row(0) *= (float)mSize.y() / (float)mSize.x();

	mShader.setUniform("modelViewProj", mvp);

	/* Draw 2 triangles starting at index 0 */
	mShader.drawIndexed(GL_TRIANGLES, 0, 2);
}
