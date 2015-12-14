#include "shellmapshelper.h"

void generateOffsetSurface(const MatrixXu &F, const MatrixXf &V, MatrixXu &oF, MatrixXf &oV, const float offset) {
	oF = F;
	oV = V;

	MatrixXf N;	// vertex normals
	computeVertexNormals(F, V, N);

	// Warning: ignore self-intersection here!!
	oV += offset * N;
}