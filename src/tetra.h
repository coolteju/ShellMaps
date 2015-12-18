#pragma once

#include "mycommon.h"
#include "tangant.h"

using nanogui::MatrixXu;
using nanogui::MatrixXf;

class TetrahedronMesh {
public:
	TetrahedronMesh() {
		mVertexCount = mTetrahedronCount = 0;
		mVtxPosition.resize(0, 0);
		mVtxTexcoord.resize(0, 0);
		mVtxNormal.resize(0, 0);
		mTetra.resize(0, 0);
		mVtxTangent = nullptr;
	}

	~TetrahedronMesh() {
		mVertexCount = mTetrahedronCount = 0;
		mVtxPosition.resize(0, 0);
		mVtxTexcoord.resize(0, 0);
		mVtxNormal.resize(0, 0);
		mTetra.resize(0, 0);
		if (mVtxTangent) delete[] mVtxTangent;
	}

//	void setTetrahedronMesh(const MatrixXf &vPosition, const MatrixXf &vNormal, const MatrixXf &vTexcoord, const TangentSpace vTangent[], const MatrixXu &tetra);

protected:
	uint32_t mVertexCount, mTetrahedronCount;
	MatrixXf mVtxPosition, mVtxTexcoord, mVtxNormal;
	TangentSpace *mVtxTangent;
	MatrixXu mTetra;
};