#pragma once

#include "mycommon.h"
//#include "tangant.h"

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
		mVtxTangentDpdu.resize(0, 0);
		mVtxTangentDpdv.resize(0, 0);
	}

	~TetrahedronMesh() {
		mVertexCount = mTetrahedronCount = 0;
		mVtxPosition.resize(0, 0);
		mVtxTexcoord.resize(0, 0);
		mVtxNormal.resize(0, 0);
		mTetra.resize(0, 0);
		mVtxTangentDpdu.resize(0, 0);
		mVtxTangentDpdv.resize(0, 0);
	}

	void setTetrahedronMesh(const MatrixXf &vPosition, const MatrixXf &vNormal, const MatrixXf &vTexcoord, const MatrixXf &vDpdu, const MatrixXf &vDpdv, const MatrixXu &tetra) {
		mVertexCount = vPosition.cols();
		mTetrahedronCount = tetra.cols();

		mVtxPosition = vPosition, mVtxNormal = vNormal, mVtxTexcoord = vTexcoord;
		mVtxTangentDpdu = vDpdu, mVtxTangentDpdv = vDpdv;
		mTetra = tetra;
	}

protected:
	uint32_t mVertexCount, mTetrahedronCount;
	MatrixXf mVtxPosition, mVtxTexcoord, mVtxNormal;
	MatrixXf mVtxTangentDpdu, mVtxTangentDpdv;
	MatrixXu mTetra;
};