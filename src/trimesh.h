#pragma once

#include "mycommon.h"
#include <vector>

using nanogui::Vector3f;
using nanogui::MatrixXf;
using nanogui::MatrixXu;

struct TangentSpace {
	Vector3f dpdu;
	Vector3f dpdv;

	inline TangentSpace	() { }
};


class TriMesh {
public:
	TriMesh();
	virtual ~TriMesh();

	void setName(const std::string& meshName) { mName = meshName; }
	void setV(MatrixXf &&V) { mV = std::move(V); }
	void setF(MatrixXu &&F) { mF = std::move(F); }
	void setUV(MatrixXf &&UV) { mUV = std::move(UV); }
	void setN(MatrixXf &&N) { mN = std::move(mN); }

	inline const MatrixXf& V() const { return mV; }
	inline const MatrixXu& F() const { return mF; }
	inline const MatrixXf& N() const { return mN; }
	inline MatrixXf& V() { return mV; }
	inline MatrixXu& F() { return mF; }
	inline MatrixXf& N() { return mN; }
	

	void free();
private:
	std::string mName;
	MatrixXf mV;			// mesh vertices
	MatrixXu mF;			// triangles indices
	MatrixXf mVN;			// per vertex outer direction vector, computed based on ajacent facets' normals which share the vertex
	MatrixXf mUV;			// per vertex texcoords
	MatrixXf mN;			// face normal

//	bool hasVertexNormals;
};
