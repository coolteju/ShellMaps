#include "viewer.h"
#include "resources.h"
#include "meshio.h"

#include <iostream>
#include <string>

using std::cout;
using std::endl;

Viewer::Viewer() : Screen(Eigen::Vector2i(1024, 758), "Shell Maps Viewer") {
	/* ui */
	Window *window = new Window(this, "Operation Panel");
	window->setPosition(Eigen::Vector2i(15, 15));
	window->setLayout(new GroupLayout);

	/* mesh loader */
	new Label(window, "load obj mesh", "sans-bold");
	Button *b = new Button(window, "Open");
	b->setCallback([&] {
		std::string meshFileName = file_dialog({ {"obj", "..."}, }, false);
		loadInput(meshFileName);
	});

	/* offset panel */
	new Label(window, "offset value", "sans-bold");
	Widget *offsetPanel = new Widget(window);
	offsetPanel->setLayout(
		new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 10));
	mOffsetSlider = new Slider(offsetPanel);
	mOffsetSlider->setValue(0.5f);
	mOffsetSlider->setId("offsetSlider");
	mOffsetSlider->setFixedWidth(60);

	mOffsetBox = new TextBox(offsetPanel);
	mOffsetBox->setFixedSize(Vector2i(80, 25));
	mOffsetBox->setValue("0");
	mOffsetBox->setId("offsetBox");

	mOffsetSlider->setCallback([&](float value) {
		double min = std::log(1e-5f);
		double max = std::log(5 * mMeshStats.mMaximumEdgeLength);
		double offset = std::exp((1 - value) * min + value * max);

		char tmp[10];
		snprintf(tmp, sizeof(tmp), "%.3f", offset);
		mOffsetBox->setValue(tmp);
	});

	mOffsetSlider->setFinalCallback([&](float value) {
		double min = std::log(1e-5f);
		double max = std::log(5 * mMeshStats.mMaximumEdgeLength);
		float offset = std::exp((1 - value) * min + value * max);

		setMeshOffset(offset);
	});

	// Generate offset mesh button
	new Label(window, "generate offset mesh", "sans-bold");
	b = new Button(window, "Generate");
	b->setCallback([&] {
		generateOffsetMesh();
	});

	new Label(window, "compute split pattern", "sans-bold");
	b = new Button(window, "Compute");
	b->setCallback([&] {
		computeSplittingPattern();
	});

	/* gui layers */
	auto layerCB = [&](bool) {
		repaint();
	};

	new Label(window, "Select render layers", "sans-bold");
	mLayers[InputMeshWireFrame] = new CheckBox(window, "Input Mesh Wireframe", layerCB);
	mLayers[InputMeshWireFrame]->setChecked(true);
	mLayers[OffsetMeshWireFrame] = new CheckBox(window, "Offset Mesh Wireframe", layerCB);
	mLayers[EdgePatternLabel] = new CheckBox(window, "Split Pattern Label on Base Mesh", layerCB);

	window = new Window(this, "Information");
	window->setPosition(Vector2i(280, 15));
	window->setLayout(new GroupLayout);

	new Label(window, "Information", "sans-bold");
	b = new Button(window, "Print info");
	b->setCallback([&] {
		printInformation();
	});


	performLayout(mNVGContext);

	/* openGL */
	mShader.init("simple_shader",
		(const char *)shader_simple_vert,
		(const char *)shader_simple_frag);

	mOffsetShader.init("simple_shader",
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
		mShader.setUniform("vertexColor", Vector3f(1.0, 1.0, 1.0));

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		mShader.drawIndexed(GL_TRIANGLES, offset, count);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	};

	drawFunctor[OffsetMeshWireFrame] = [&](uint32_t offset, uint32_t count) {
		mOffsetShader.bind();
		mOffsetShader.setUniform("modelViewProj", Matrix4f(proj * view * model));
		mOffsetShader.setUniform("vertexColor", Vector3f(0.5, 0.5, 1.0));

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		mOffsetShader.drawIndexed(GL_TRIANGLES, offset, count);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	};

	drawFunctor[EdgePatternLabel] = [&](uint32_t offset, uint32_t count) {
		nvgBeginFrame(mNVGContext, mSize[0], mSize[1], mPixelRatio);
		nvgFontSize(mNVGContext, 14.0f);
		nvgFontFace(mNVGContext, "sans-bold");
		nvgTextAlign(mNVGContext, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		const MatrixXf &V = mMesh.V();
		const MatrixXu &F = mMesh.F();
		const MatrixXu &P = splitPattern;
		nvgFillColor(mNVGContext, Color(200, 200, 255, 200));

		for (uint32_t f = offset; f < offset + count; ++f) {
			Vector3f points[3] = { V.col(F(0, f)), V.col(F(1, f)), V.col(F(2, f)) };

			for (int i = 0; i < 3; ++i) {
				int j = (i == 2 ? 0 : i + 1);
				int k = (j == 2 ? 0 : j + 1);

				float wk = 0.01f, wj = (1.0f - wk) / 2.0f, wi = wj;
				Vector4f pos;
				pos << (wi*points[i] + wj*points[j] + wk*points[k]).cast<float>(), 1.0f;
				Eigen::Vector3f coord = project(Vector3f((model * pos).head<3>()), view, proj, mSize);

				// TODO: Set different colors for "R" and "N", but it may be inefficient to add nvgFillColor in below loop?
				std::string patternLabel = "N";
				if (SPLIT_PATTERN_R == static_cast<SPLIT_PATTERN>(P(i, f))) {
					patternLabel = "R";
				}
				else if (SPLIT_PATTERN_F == static_cast<SPLIT_PATTERN>(P(i, f))) {
					patternLabel = "F";
				}

				// ignore occlusion here
				nvgText(mNVGContext, coord.x(), mSize[1] - coord.y(), patternLabel.c_str(), nullptr);
			}
			nvgEndFrame(mNVGContext);
		}
	};

	uint32_t drawAmount[LayerCount];
	drawAmount[InputMeshWireFrame] = mMesh.F().cols();
	drawAmount[OffsetMeshWireFrame] = mOffsetMesh.F().cols();
	drawAmount[EdgePatternLabel] = splitPattern.cols();

	bool checked[LayerCount];
	for (int i = 0; i < LayerCount; ++i) {
		checked[i] = mLayers[i]->checked();
	}

	const int drawOrder[] = {
		InputMeshWireFrame,
		OffsetMeshWireFrame,
		EdgePatternLabel
	};

	for (uint32_t j = 0; j < sizeof(drawOrder) / sizeof(int); ++j) {
		uint32_t i = drawOrder[j];

		if (checked[i]) {
			drawFunctor[i](0, drawAmount[i]);
		}
	}
}

void Viewer::loadInput(std::string & meshFileName) {
	MatrixXf V;
	MatrixXu F;
	MatrixXf N;	// face normals

	loadObj(meshFileName, F, V);
	computeFaceNormals(F, V, N);
	mMeshStats = computeMeshStats(F, V);

	mMesh.free();
	mMesh.setF(std::move(F));
	mMesh.setV(std::move(V));
	mMesh.setN(std::move(N));

	mShader.bind();
	mShader.uploadAttrib("position", mMesh.V());
	mShader.uploadIndices(mMesh.F());

	mCamera.modelTranslation = -mMeshStats.mWeightedCenter.cast<float>();
	mCamera.modelZoom = 3.0f / (mMeshStats.mAABB.max - mMeshStats.mAABB.min).cwiseAbs().maxCoeff();

	// Initialize offset
	setMeshOffset(mMeshStats.mAverageEdgeLength);
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

void Viewer::generateOffsetMesh() {
	MatrixXf oV;
	MatrixXu oF;
	float offset = (mOffset >= 0.0f ? mOffset : 0.0f);

	generateOffsetSurface(mMesh.F(), mMesh.V(), oF, oV, offset);

	mOffsetMesh.free();
	mOffsetMesh.setF(std::move(oF));	// same with base mesh
	mOffsetMesh.setV(std::move(oV));

	mOffsetShader.bind();
	mOffsetShader.uploadAttrib("position", mOffsetMesh.V());
//	mOffsetShader.uploadIndices(mOffsetMesh.F());

	// share indices
	shareGLBuffers();
}

void Viewer::shareGLBuffers() {
	mOffsetShader.bind();
	mOffsetShader.shareAttrib(mShader, "indices");
}

void Viewer::setMeshOffset(double offset) {
	char tmp[10];
	snprintf(tmp, sizeof(tmp), "%.3f", offset);

	double value = std::log(offset);
	double min = std::log(1e-5f);
	double max = std::log(5 * mMeshStats.mMaximumEdgeLength);

	mOffsetSlider->setValue((value - min) / (max - min));
	mOffsetBox->setValue(tmp);

	mOffset = offset;
}

void Viewer::printInformation() {
	cout << "Base mesh:" << "\n";
	cout << "Vertex count: " << mMesh.V().cols() << "\n";
	cout << "Triangles: " << mMesh.F().cols() << "\n";
	cout << "minEdgeLenth, maxEdgeLength, avgEdgeLength: " << mMeshStats.mMinimumEdgeLength << ", "
		<< mMeshStats.mMaximumEdgeLength << ", "
		<< mMeshStats.mAverageEdgeLength << "\n";

	cout << "-------------------" << "\n";
	cout << "Offset:" << "\n";
	cout << "offset value: " << mOffset << "\n";

	cout << endl;
}

void Viewer::computeSplittingPattern() {
	computePrimsSplittingPattern(mMesh.F(), splitPattern);
}

void Viewer::constructTetrahedra() {
	// TODO: compute tetrahedra and generate a tetra object(defined later)
}
