/*
    normal.cpp: Helper routines for computing vertex normals

    This file is part of the implementation of

        Instant Field-Aligned Meshes
        Wenzel Jakob, Daniele Panozzo, Marco Tarini, and Olga Sorkine-Hornung
        In ACM Transactions on Graphics (Proc. SIGGRAPH Asia 2015)

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "normal.h"

void computeVertexNormals(const MatrixXu &F, const MatrixXf &V, MatrixXf &N) {
	std::cout << "--Computing vertex normals ..." << std::endl;
	std::cout.flush();

	uint32_t badFaces = 0;
	N.resize(V.rows(), V.cols());
	N.setZero();

	const float RCPOVERFLOW_FLT = 2.93873587705571876e-39f;

	uint32_t trianglesCount = F.cols();
	for (uint32_t f = 0; f < trianglesCount; ++f) {
		Vector3f fn = Vector3f::Zero();

		for (int i = 0; i < 3; ++i) {
			Vector3f v0 = V.col(F(i, f)),
				v1 = V.col(F((i + 1) % 3, f)),
				v2 = V.col(F((i + 2) % 3, f)),
				d0 = v1 - v0,
				d1 = v2 - v0;
					 
			if (i == 0) {
				fn = d0.cross(d1);
				float norm = fn.norm();
				if (norm < RCPOVERFLOW_FLT) {
					badFaces++;
					break;
				}
				fn /= norm;
			}

			/* "Computing Vertex Normals from Polygonal Facets"
			by Grit Thuermer and Charles A. Wuethrich, JGT 1998, Vol 3 */
			float angle = fast_acos(d0.dot(d1) / std::sqrt(d0.squaredNorm() * d1.squaredNorm()));
			N.col(F(i, f)) += fn * angle;
		}
	}

	uint32_t vertexCount = V.cols();
	for (uint32_t v = 0; v < vertexCount; ++v) {
		float norm = N.col(v).norm();
		if (norm < RCPOVERFLOW_FLT) {
			N.col(v) = Vector3f::UnitX();
		}
		else {
			N.col(v) /= norm;
		}
	}

	std::cout << "++Computing vertex normals done. (";
	if (badFaces > 0)
		std::cout << badFaces << " degenerate faces.";
	std::cout << ")" << std::endl;
}

void computeFaceNormals(const MatrixXu &F, const MatrixXf &V, MatrixXf &N) {
	N.resize(F.rows(), F.cols());
	N.setZero();

	uint32_t trianglesCount = F.cols();
	for (int f = 0; f < trianglesCount; ++f) {
		Vector3f points[3] = { V.col(F(0, f)), V.col(F(1, f)), V.col(F(2, f)) };
		Vector3f d0 = points[1] - points[0],
			d1 = points[2] - points[1];
		Vector3f n = d0.cross(d1);
		n /= n.norm();

		N.col(f) = n;
	}
}
