#include "tangent.h"

void computeVertexTangents(const MatrixXu &F, const MatrixXf &V, const MatrixXf &UV, MatrixXf &DPDU, MatrixXf &DPDV) {
	DPDU.resize(V.rows(), V.cols());
	DPDU.setZero();
	DPDV.resize(V.rows(), V.cols());
	DPDV.setZero();

	using nanogui::Vector2f;
	const float RCPOVERFLOW_FLT = 2.93873587705571876e-39f;

	uint32_t trianglesCount = F.cols();
	for (uint32_t f = 0; f < trianglesCount; ++f) {
		for (int i = 0; i < 3; ++i) {
			Vector3f v0 = V.col(F(i, f)),
	     			 v1 = V.col(F((i + 1) % 3, f)),
				     v2 = V.col(F((i + 2) % 3, f));
			Vector2f uv0 = UV.col(F(i, f)),
				     uv1 = UV.col(F((i + 1) % 3, f)),
				     uv2 = UV.col(F((i + 2) % 3, f));

			Vector3f dP1 = v1 - v0, dP2 = v2 - v0;
			Vector2f dUV1 = uv1 - uv0, dUV2 = uv2 - uv0;
			Vector3f n = dP1.cross(dP2);
			float length = n.norm();

			Vector3f dpdu, dpdv;

			float determinant = dUV1.x() * dUV2.y() - dUV1.y() * dUV2.x();
			if (determinant == 0) {
				coordinate_system(n / length, dpdu, dpdv);
			}
			else {
				float invDet = 1.0f / determinant;
				dpdu = ( dUV2.y() * dP1 - dUV1.y() * dP2) * invDet;
				dpdv = (-dUV2.x() * dP1 + dUV1.x() * dP2) * invDet;
			}

			float angle = fast_acos(dP1.dot(dP2) / std::sqrt(dP1.squaredNorm() * dP2.squaredNorm()));
			DPDU.col(F(i, f)) += dpdu * angle;
			DPDV.col(F(i, f)) += dpdv * angle;
		}
	}

	uint32_t vertexCount = V.cols();
	for (uint32_t v = 0; v < vertexCount; ++v) {
		float norm = DPDU.col(v).norm();
		if (norm < RCPOVERFLOW_FLT) {
			// ...
			std::cout << "unhandle case in computing tangent." << std::endl;
		}
		else {
			DPDU.col(v) /= norm;
		}

		norm = DPDV.col(v).norm();
		if (norm < RCPOVERFLOW_FLT) {
			std::cout << "unhandle case in computing tangent." << std::endl;
		}
		else {
			DPDV.col(v) /= norm;
		}
	}
}

void computeFaceTangents(const MatrixXu &F, const MatrixXf &V, const MatrixXf &UV, MatrixXf &DPDU, MatrixXf &DPDV) {
	uint32_t trianglesCount = F.cols();

	using nanogui::Vector2f;

	for (uint32_t f = 0; f < trianglesCount; ++f) {
		Vector3f points[3] = { V.col(F(0, f)), V.col(F(1, f)), V.col(F(2, f)) };
		Vector2f uvs[3] = { UV.col(F(0, f)), UV.col(F(1, f)), UV.col(F(2, f)) };

		Vector3f dP1 = points[1] - points[0];
		Vector3f dP2 = points[2] - points[0];
		Vector2f dUV1 = uvs[1] - uvs[0];
		Vector2f dUV2 = uvs[2] - uvs[0];
		Vector3f n = dP1.cross(dP2);
		float length = n.norm();

		Vector3f dpdu, dpdv;

		float determinant = dUV1.x() * dUV2.y() - dUV1.y() * dUV2.x();
		if (determinant == 0) {
			/* The user-specified parameterization is degenerate. Pick
			arbitrary tangents that are perpendicular to the geometric normal */
			coordinate_system(n / length, dpdu, dpdv);
			std::cout << "Warning: degenerate parmaters (u,v) found, use face normal to compute tangent space." << std::endl;
		}
		else {
			float invDet = 1.0f / determinant;
			dpdu = ( dUV2.y() * dP1 - dUV1.y() * dP2) * invDet;
			dpdv = (-dUV2.x() * dP1 + dUV1.x() * dP2) * invDet;
		}

		DPDU.col(f) = dpdu;
		DPDV.col(f) = dpdv;
	}
}