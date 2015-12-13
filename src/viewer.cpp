#include "viewer.h"
#include "resources.h"
#include "meshio.h"

#include <iostream>
#include <string>

using std::cout;
using std::endl;

Viewer::Viewer() : Screen(Eigen::Vector2i(1024, 758), "Shell Maps Viewer") {
	/* ui */
	Window *window = new Window(this, "Load Mesh");
	window->setPosition(Eigen::Vector2i(15, 15));
	window->setLayout(new GroupLayout);

	Button *b = new Button(window, "Open");
	b->setCallback([&] {
		std::string meshFileName = file_dialog({ {"obj", "..."}, }, false);
		loadInput(meshFileName);
	});

	performLayout(mNVGContext);

	/* openGL */
	mShader.init("simple_shader",
		(const char *)shader_simple_vert,
		(const char *)shader_simple_frag);

	// tmp test
	/*
	Vector3f p0(0.0f, 0.0f, 0.0f);
	Vector3f p1(1.0f, 0.0f, 0.0f);
	Vector3f p2(0.0f, 1.0f, 0.0f);

	float fa = 0.5f * (p1 - p0).cross(p2 - p1).norm();
	cout << fa << endl;
*/
}

Viewer::~Viewer() {
	mShader.free();
}

void Viewer::repaint() {
	glfwPostEmptyEvent();
}

bool Viewer::resizeEvent(const Vector2i &size) {
	mCamera.arcball.setSize(mSize);
	repaint();
	return true;
}

bool Viewer::scrollEvent(const Vector2i & p, const Vector2f & rel) {
	if (!Screen::scrollEvent(p, rel)) {
		mCamera.zoom = std::max(0.1, mCamera.zoom * (rel.y() > 0 ? 1.1 : 0.9));
		repaint();
	}
	return true;
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

bool Viewer::mouseMotionEvent(const Vector2i & p, const Vector2i & rel, int button, int modifiers) {
	if (!Screen::mouseMotionEvent(p, rel, button, modifiers)) {
		if (mCamera.arcball.motion(p)) {
			repaint();
		}
		else if (mTranslate) {
			Matrix4f model, view, proj;
			computeCameraMatrices(model, view, proj);
			float zval = project(mMeshStats.mWeightedCenter.cast<float>(), view * model, proj, mSize).z();
			Vector3f pos1 = unproject(Vector3f(p.x(), mSize.y() - p.y(), zval), view * model, proj, mSize);
			Vector3f pos0 = unproject(Vector3f(mTranslateStart.x(), mSize.y() - mTranslateStart.y(), zval), view * model, proj, mSize);
			mCamera.modelTranslation = mCamera.modelTranslation_start + (pos1 - pos0);

			repaint();
		}
	}
	return true;
}

bool Viewer::mouseButtonEvent(const Vector2i & p, int button, bool down, int modifiers) {
	if (!Screen::mouseButtonEvent(p, button, down, modifiers)) {
		if (button == GLFW_MOUSE_BUTTON_1 && modifiers == 0) {
			mCamera.arcball.button(p, down);
		}
		else if (button == GLFW_MOUSE_BUTTON_2 ||
			(button == GLFW_MOUSE_BUTTON_1 && modifiers == GLFW_MOD_SHIFT)) {
			mCamera.modelTranslation_start = mCamera.modelTranslation;
			mTranslate = true;
			mTranslateStart = p;
		}
	}

	if (button == GLFW_MOUSE_BUTTON_1 && !down) {
		mCamera.arcball.button(p, false);
	}
	if (!down) {
		mTranslate = false;
	}
	return true;
}

void Viewer::draw(NVGcontext * ctx) {
	Screen::draw(ctx);
}

void Viewer::drawContents() {
	Matrix4f model, view, proj;
	computeCameraMatrices(model, view, proj);

	std::function<void(uint32_t, uint32_t)> drawFunctor[LayerCount];

	drawFunctor[InputMeshWireFrame] = [&](uint32_t offset, uint32_t count) {
		mShader.bind();
		mShader.setUniform("modelViewProj", Matrix4f(proj * view * model));
		mShader.setUniform("intensity", 0.6f);

		mShader.drawIndexed(GL_TRIANGLES, offset, count);
	};

	uint32_t drawAmount[LayerCount];
	drawAmount[InputMeshWireFrame] = mMesh.F().cols();

	const int drawOrder[] = {
		InputMeshWireFrame
	};

	for (uint32_t j = 0; j < sizeof(drawOrder) / sizeof(int); ++j) {
		uint32_t i = drawOrder[j];
		drawFunctor[i](0, drawAmount[i]);
	}
}

void Viewer::loadInput(std::string & meshFileName) {
	MatrixXf V;
	MatrixXu F;

	loadObj(meshFileName, F, V);
	mMeshStats = computeMeshStats(F, V);

	mMesh.free();
	mMesh.setF(std::move(F));
	mMesh.setV(std::move(V));

	mShader.bind();
	mShader.uploadAttrib("position", mMesh.V());
	mShader.uploadIndices(mMesh.F());

	mCamera.modelTranslation = -mMeshStats.mWeightedCenter.cast<float>();
	mCamera.modelZoom = 3.0f / (mMeshStats.mAABB.max - mMeshStats.mAABB.min).cwiseAbs().maxCoeff();
}

void Viewer::computeCameraMatrices(Matrix4f & model, Matrix4f & view, Matrix4f & proj) {
	view = lookAt(mCamera.eye, mCamera.center, mCamera.up);
	float fH = std::tan(mCamera.viewAngle / 360.0f * M_PI) * mCamera.dnear;
	float fW = fH * (float)mSize.x() / (float)mSize.y();

	proj = frustum(-fW, fW, -fH, fH, mCamera.dnear, mCamera.dfar);
	model = mCamera.arcball.matrix();
	model = scale(model, Vector3f::Constant(mCamera.zoom * mCamera.modelZoom));
	model = translate(model, mCamera.modelTranslation);
}
