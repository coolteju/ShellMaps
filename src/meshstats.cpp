#include "meshstats.h"

MeshStats computeMeshStats(const MatrixXu &F, const MatrixXf &V) {
	MeshStats stats;
	uint32_t trianglesCount = F.cols();

	for (uint32_t f = 0; f < trianglesCount; f++) {
		Vector3f v[3] = { V.col(F(0, f)), V.col(F(1, f)), V.col(F(2, f)) };
		Vector3f faceCenter = Vector3f::Zero();
		AABB aabb;

		for (int i = 0; i < 3; i++) {
			faceCenter += v[i];
			aabb.expandBy(v[i]);

			float edgeLength = (v[i] - v[i == 2 ? 0 : (i + 1)]).norm();
			stats.mAverageEdgeLength += edgeLength;
			stats.mMaximumEdgeLength = std::max(stats.mMaximumEdgeLength, (double)edgeLength);
			stats.mMinimumEdgeLength = std::min(stats.mMinimumEdgeLength, (double)edgeLength);
		}

		stats.mAABB.expandBy(aabb);

		faceCenter *= 1.0f / 3.0f;
		double faceArea = 0.5f * (v[1] - v[0]).cross(v[2] - v[0]).norm();
		stats.mSurfaceArea += faceArea;
		stats.mWeightedCenter += faceArea * faceCenter;
	}

	stats.mWeightedCenter /= stats.mSurfaceArea;
	stats.mAverageEdgeLength /= trianglesCount * 3;

	return stats;
}