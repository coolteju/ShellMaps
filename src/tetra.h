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

	void setTetrahedronMesh(MatrixXf &&V, MatrixXf &&N, MatrixXf &&UV, MatrixXf &&DPDU, MatrixXf &&DPDV, MatrixXu &&T) {
		mVertexCount = V.cols();
		mTetrahedronCount = T.cols();

		mVtxPosition = std::move(V), mVtxNormal = std::move(N), mVtxTexcoord = std::move(UV);
		mVtxTangentDpdu = std::move(DPDU), mVtxTangentDpdv = std::move(DPDV);
		mTetra = std::move(T);
	}

	inline uint32_t getVertexCount() const { return mVertexCount; }
	inline uint32_t getTetrahedronCount() const { return mTetrahedronCount; }

	inline const MatrixXf& V() const { return mVtxPosition; }
	inline const MatrixXf& UV() const { return mVtxTexcoord; }
	inline const MatrixXf& N() const { return mVtxNormal; }
	inline const MatrixXf& DPDU() const { return mVtxTangentDpdu; }
	inline const MatrixXf& DPDV() const { return mVtxTangentDpdv; }

	inline const MatrixXu& T() const { return mTetra; }

protected:
	uint32_t mVertexCount, mTetrahedronCount;
	MatrixXf mVtxPosition, mVtxTexcoord, mVtxNormal;
	MatrixXf mVtxTangentDpdu, mVtxTangentDpdv;
	MatrixXu mTetra;
};