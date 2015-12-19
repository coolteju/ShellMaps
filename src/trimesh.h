#pragma once

#include "mycommon.h"
#include <vector>

using nanogui::Vector3f;
using nanogui::MatrixXf;
using nanogui::MatrixXu;

class TriMesh {
public:
	TriMesh();
	virtual ~TriMesh();

	void setName(const std::string& meshName) { mName = meshName; }

	void setV(MatrixXf &&V) { mV = std::move(V); }
	void setF(MatrixXu &&F) { mF = std::move(F); }
	void setN(MatrixXf &&N) { mN = std::move(N); }
	void setUV(MatrixXf &&UV) { mUV = std::move(UV); }
	void setDPDU(MatrixXf &&DPDU) { mDPDU = std::move(DPDU); }
	void setDPDV(MatrixXf &&DPDV) { mDPDV = std::move(DPDV); }

	inline const MatrixXf& V() const { return mV; }
	inline const MatrixXu& F() const { return mF; }
	inline const MatrixXf& N() const { return mN; }
	inline const MatrixXf& UV() const { return mUV; }
	inline const MatrixXf& DPDU() const { return mDPDU; }
	inline const MatrixXf& DPDV() const { return mDPDV; }
	inline MatrixXf& V() { return mV; }
	inline MatrixXu& F() { return mF; }
	inline MatrixXf& N() { return mN; }
	inline MatrixXf& UV() { return mUV; }
	inline MatrixXf& DPDU() { return mDPDU; }
	inline MatrixXf& DPDV() { return mDPDV; }

	inline bool hasVertexNormals() const { return mN.cols() != 0; }
	inline bool hasVertexTexcoords() const { return mUV.cols() != 0; }
	inline bool hasUVTangents() const { return mDPDU.cols() != 0 && mDPDV.cols() != 0; }
	
	void free();
private:
	std::string mName;
	MatrixXf mV;			// mesh vertices
	MatrixXu mF;			// triangles indices
	MatrixXf mN;			// per vertex outer direction vector, computed based on ajacent facets' normals which share the vertex
	MatrixXf mUV;			// per vertex texcoords
	MatrixXf mDPDU, mDPDV;	// per vertex tangent space
};
