#pragma once

#include <nanogui\nanogui.h>
#include <nanogui\glutil.h>
#include "trimesh.h"
#include "meshstats.h"
#include "shellmapshelper.h"

using namespace nanogui;

class Viewer : public Screen {
public:
	Viewer();
	virtual ~Viewer();

	virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);
	bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);
	bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
	virtual void draw(NVGcontext *ctx);
	virtual void drawContents();

protected:
	void repaint();
	bool resizeEvent(const Vector2i &size);
	bool scrollEvent(const Vector2i &p, const Vector2f &rel);

	void loadInput(std::string &meshFileName);
	void computeCameraMatrices(Matrix4f &model, Matrix4f &view, Matrix4f &proj);

	void generateOffsetMesh();
	void shareGLBuffers();
	void setMeshOffset(double offset);

	void printInformation();

protected:
	struct CameraParameters {
		Arcball arcball;
		float zoom = 1.0f, viewAngle = 45.0f;
		float dnear = 0.05f, dfar = 100.0f;
		Vector3f eye = Vector3f(0.0f, 0.0f, 5.0f);
		Vector3f center = Vector3f(0.0f, 0.0f, 0.0f);
		Vector3f up = Vector3f(0.0f, 1.0f, 5.0f);
		Vector3f modelTranslation = Vector3f::Zero();
		Vector3f modelTranslation_start = Vector3f::Zero();
		float modelZoom = 1.0f;
	};

	/* Camera */
	CameraParameters mCamera;
	bool mTranslate;
	Vector2i mTranslateStart;

	/* Data being processing */
	TriMesh mMesh;
	MeshStats mMeshStats;
	TriMesh mOffsetMesh;
	double mOffset;

	/* OpenGL objects */
	GLShader mShader;
	GLShader mOffsetShader;

	/* GUI-related */
	enum Layers {
		InputMeshWireFrame,
		OffsetMeshWireFrame,
		EdgePatternLabel,
		LayerCount
	};

	CheckBox *mLayers[LayerCount];
	Slider *mOffsetSlider;
	TextBox *mOffsetBox;
};