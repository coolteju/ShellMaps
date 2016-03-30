#include "viewer.h"
#include "resources.h"
#include "meshio.h"
#include "shellmapshelper.h"
#include "normal.h"
#include "tangent.h"
#include "shellbounds.h"

#include <iostream>
#include <string>
#include <iomanip>

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
		if (meshFileName.size() > 0) {
			loadInput(meshFileName);
			updateMesh();
		}
	});

	b = new Button(window, "Flip");
	b->setCallback([&] {
		for (uint32_t f = 0; f < inF.cols(); f++) {
			std::swap(inF(1, f), inF(2, f));
		}
		updateMesh();
	});

	b = new Button(window, "scale! factor from stdin");
	b->setCallback([&] {
		std::cout << "input scale factor: " << std::endl;
		float s = 1.0;
		std::cin >> s;
		if (s <= 0) {
			std::cout << "invalid scale factor." << std::endl;
		}
		inV *= (s / meshScale);
		meshScale = s;
		std::cout << "mesh has been scaled x" << meshScale << "!." << std::endl;
		updateMesh();
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

	new Label(window, "construct tetrahedron mesh", "sans-bold");
	b = new Button(window, "Construct");
	b->setCallback([&] {
		constructTetrahedronMesh();
	});

	/// TODO: add save shell and bounds panel
	new Label(window, "save shell and bounding shape", "sans-bold");
	b = new Button(window, "Save shell");
	b->setCallback([&] {
		string fileName = file_dialog({ {"dat", "..."}, }, true);
		saveShellToMitsuba(fileName, mShell);
	});

	b = new Button(window, "Save bound");
	b->setCallback([&] {
		string fileName = file_dialog({ {"obj", "Wavefront OBJ" }, }, true);
		MatrixXu boundF;
		MatrixXf boundV;
		generateShellBoundSimple(mMesh.F(), mMesh.V(), mOffsetMesh.V(), boundF, boundV);
		writeObj(fileName, boundF, boundV);
	});

	/* gui layers */
	auto layerCB = [&](bool) {
		repaint();
	};

	new Label(window, "Select render layers", "sans-bold");
	mLayers[InputMeshWireFrame] = new CheckBox(window, "Input Mesh Wireframe", layerCB);
	mLayers[InputMeshWireFrame]->setChecked(true);
	mLayers[FaceLabel] = new CheckBox(window, "Face label", layerCB);
	mLayers[VertexLabel] = new CheckBox(window, "Vertex label", layerCB);
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
	
	drawFunctor[FaceLabel] = [&](uint32_t offset, uint32_t count) {
		nvgBeginFrame(mNVGContext, mSize[0], mSize[1], mPixelRatio);
		nvgFontSize(mNVGContext, 14.0f);
		nvgFontFace(mNVGContext, "sans-bold");
		nvgTextAlign(mNVGContext, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		const MatrixXf &V = mMesh.V();
		const MatrixXu &F = mMesh.F();
		nvgFillColor(mNVGContext, Color(200, 200, 255, 200));

		for (uint32_t i = offset; i<offset + count; ++i) {
			Vector4f pos;
			pos << (1.0f / 3.0f) * (V.col(F(0, i)) + V.col(F(1, i)) +
				V.col(F(2, i))).cast<float>(), 1.0f;

			Eigen::Vector3f coord = project(Vector3f((model * pos).head<3>()), view, proj, mSize);
			nvgText(mNVGContext, coord.x(), mSize[1] - coord.y(), std::to_string(i).c_str(), nullptr);
		}
		nvgEndFrame(mNVGContext);
	};

	drawFunctor[VertexLabel] = [&](uint32_t offset, uint32_t count) {
		nvgBeginFrame(mNVGContext, mSize[0], mSize[1], mPixelRatio);
		nvgFontSize(mNVGContext, 14.0f);
		nvgFontFace(mNVGContext, "sans-bold");
		nvgTextAlign(mNVGContext, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		const MatrixXf &V = mMesh.V();
		nvgFillColor(mNVGContext, Color(255, 100, 200, 200));
		for (uint32_t i = offset; i<offset + count; ++i) {
			Vector4f pos;
			pos << V.col(i).cast<float>(), 1.0f;

			Eigen::Vector3f coord = project(Vector3f((model * pos).head<3>()), view, proj, mSize);
			nvgText(mNVGContext, coord.x(), mSize[1] - coord.y(), std::to_string(i).c_str(), nullptr);
		}
		nvgEndFrame(mNVGContext);
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

				const float wk = 0.01f, wj = (1.0f - wk) / 2.0f, wi = wj;
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
	drawAmount[FaceLabel] = mMesh.F().cols();
	drawAmount[VertexLabel] = mMesh.V().cols();
	drawAmount[EdgePatternLabel] = splitPattern.cols();

	bool checked[LayerCount];
	for (int i = 0; i < LayerCount; ++i) {
		checked[i] = mLayers[i]->checked();
	}

	const int drawOrder[] = {
		InputMeshWireFrame,
		OffsetMeshWireFrame,
		FaceLabel,
		VertexLabel,
		EdgePatternLabel
	};

	for (uint32_t j = 0; j < sizeof(drawOrder) / sizeof(int); ++j) {
		uint32_t i = drawOrder[j];

		if (checked[i]) {
			drawFunctor[i](0, drawAmount[i]);
		}
	}
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

void Viewer::shareGLBuffers() {
	mOffsetShader.bind();
	mOffsetShader.shareAttrib(mShader, "indices");
}

void Viewer::printInformation() {
	cout << "Base mesh:" << "\n";
	cout << "Vertex count: " << mMesh.V().cols() << "\n";
	cout << "Triangles: " << mMesh.F().cols() << "\n";
	cout << "UV count:  " << mMesh.UV().cols() << "\n";
	cout << "minEdgeLenth, maxEdgeLength, avgEdgeLength: " << mMeshStats.mMinimumEdgeLength << ", "
		<< mMeshStats.mMaximumEdgeLength << ", "
		<< mMeshStats.mAverageEdgeLength << "\n";
	cout << "surfaceArea: " << mMeshStats.mSurfaceArea << "\n";

	cout << "-------------------" << "\n";
	cout << "Offset:" << "\n";
	cout << "offset value: " << mOffset << "\n";

	cout << "---------Vertex Infor-----" << "\n";
	uint32_t list[] = {0, 1, 2 };

	cout << setprecision(7);

	for (auto v : list) {
		cout << "||||||||||||||||||" << endl;
//		std::cout << "vertex id: " << v << endl;
//		cout << "pos: " << mMesh.V()(0, v) << " " << mMesh.V()(1, v) << " " << mMesh.V()(2, v) << endl;
//		cout << "uv:  " << mMesh.UV()(0, v) << " " << mMesh.UV()(1, v) << endl;
//		cout << "n:   " << mMesh.N()(0, v) << " " << mMesh.N()(1, v) << " " << mMesh.N()(2, v) << endl;
//		cout << "dpdu:" << mMesh.DPDU()(0, v) << " " << mMesh.DPDU()(1, v) << " " << mMesh.DPDU()(2, v) << endl;
//		cout << "dpdu:" << mMesh.DPDV()(0, v) << " " << mMesh.DPDV()(1, v) << " " << mMesh.DPDV()(2, v) << endl;
	//	cout << "off: " << mOffsetMesh.V()(0, v) << " " << mOffsetMesh.V()(1, v) << " " << mOffsetMesh.V()(2, v) << endl;
		cout << "||||||||||||||||||\n" << endl;
	}
	cout << "------------------------------" << endl;

//	cout << "F0: " << mMesh.F().col(0) << endl;
//	cout << "F1: " << mMesh.F().col(1) << endl;

	cout << endl;
}

void Viewer::loadInput(std::string & meshFileName) {
	inF.resize(0, 0);
	inV.resize(0, 0);
	inUV.resize(0, 0);

	loadObjShareVertexNotShareTexcoord(meshFileName, inF, inV, inUV);
	meshScale = 1.0;
	if (inUV.cols() > 0) {
		resizeUV();
	}
}

void Viewer::updateMesh() {
	MatrixXf N, DPDU, DPDV;

	computeVertexNormals(inF, inV, N, true);
	if (inUV.cols() > 0)
		computeVertexTangents(inF, inV, inUV, DPDU, DPDV, true);

	mMesh.free();
	mMesh.setF(std::move(inF));
	mMesh.setV(std::move(inV));
	mMesh.setN(std::move(N));
	if (inUV.cols() > 0) {
		mMesh.setUV(std::move(inUV));
		mMesh.setDPDU(std::move(DPDU));
		mMesh.setDPDV(std::move(DPDV));
	}

	mShader.bind();
	mShader.uploadAttrib("position", mMesh.V());
	mShader.uploadIndices(mMesh.F());

	mMeshStats = computeMeshStats(mMesh.F(), mMesh.V());
	mCamera.modelTranslation = -mMeshStats.mWeightedCenter.cast<float>();
	mCamera.modelZoom = 3.0f / (mMeshStats.mAABB.max - mMeshStats.mAABB.min).cwiseAbs().maxCoeff();

	// Initialize offset
	setMeshOffset(mMeshStats.mAverageEdgeLength);
}

void Viewer::resizeUV() {
	/* check if UV is between [0, 1] */
	// TODO: it's better to compute tangents first and then resize uv coordinates.
	if (inUV.cols() > 0) {
		float uMin = 1000000.0, vMin = 1000000.0, uMax = -1000000, vMax = -1000000;

		for (int v = 0; v < inUV.cols(); ++v) {
			if (inUV(0, v) < uMin) uMin = inUV(0, v);
			if (inUV(0, v) > uMax) uMax = inUV(0, v);
			if (inUV(1, v) < vMin) vMin = inUV(1, v);
			if (inUV(1, v) > vMax) vMax = inUV(1, v);
		}

		std::cout << "[u, v] is between (" << uMin << ", " << vMin << ") and (" << uMax << ", " << vMax << ").\n";
		std::cout << "first v: " << inUV(0, 0) << ", " << inUV(1, 0) << endl;

		if (uMin < 0.0 || uMax > 1.0 || vMin < 0.0 || vMax > 1.0) {
			std::cout << "resizing u,v .." << std::endl;

			float uR = uMax - uMin;
			float vR = vMax - vMin;

			float s = std::max(uR, vR);
			inUV /= s;
			float tu = uMin / s;
			float tv = vMin / s;

			inUV.row(0) -= MatrixXf::Constant(1, inUV.cols(), tu - 0.000001);
			inUV.row(1) -= MatrixXf::Constant(1, inUV.cols(), tv - 0.000001);

			std::cout << "after, first v: " << inUV(0, 0) << ", " << inUV(1, 0) << endl;

			uMax /= s;
			uMax -= (tu - 0.000001);
			vMax /= s;
			vMax -= (tv - 0.000001);

			std::cout << "after resizing, uv is between (" << 0.0 << ", " << 0.0 << ") and (" << uMax << ", " << vMax << ").\n";
		}
	}
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

void Viewer::generateOffsetMesh() {
	MatrixXf oV;
	MatrixXu oF;
	MatrixXf oUV;
	MatrixXf oN;
	MatrixXf oDPDU, oDPDV;
	float offset = (mOffset >= 0.0f ? mOffset : 0.0f);

	//	generateOffsetSurface(mMesh.F(), mMesh.V(), oF, oV, offset);
	generateOffsetSurface(mMesh.F(), mMesh.V(), mMesh.N(), oF, oV, offset);

	mOffsetMesh.free();
	mOffsetMesh.setF(std::move(oF));	// same with base mesh
	mOffsetMesh.setV(std::move(oV));

	mOffsetShader.bind();
	mOffsetShader.uploadAttrib("position", mOffsetMesh.V());
	//	mOffsetShader.uploadIndices(mOffsetMesh.F());

	// share indices
	shareGLBuffers();
}

void Viewer::computeSplittingPattern() {
	computePrimsSplittingPattern(mMesh.F(), splitPattern);
}

void Viewer::constructTetrahedronMesh() {
	constructTetrahedronMeshSimple(mMesh.F(), mMesh.V(), mOffsetMesh.V(),
		mMesh.UV(), mMesh.N(), mMesh.DPDU(), mMesh.DPDV(), splitPattern, mShell);
}