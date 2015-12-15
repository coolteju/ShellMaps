#pragma once

#include "mycommon.h"
#include "aabb.h"

using nanogui::Vector3f;
using nanogui::MatrixXf;
using nanogui::MatrixXu;

using namespace std;

struct MeshStats {
	AABB mAABB;
	Vector3f mWeightedCenter;
	double mSurfaceArea;
	double mMaximumEdgeLength;
	double mMinimumEdgeLength;
	double mAverageEdgeLength;

	MeshStats() : 
		mWeightedCenter(Vector3f::Zero()),
		mSurfaceArea(0.0f),
		mMaximumEdgeLength(0.0f),
		mMinimumEdgeLength(0.0f) { }
};

extern MeshStats computeMeshStats(const MatrixXu &F, const MatrixXf &V);