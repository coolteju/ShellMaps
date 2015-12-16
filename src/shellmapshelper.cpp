#include "shellmapshelper.h"

void generateOffsetSurface(const MatrixXu &F, const MatrixXf &V, MatrixXu &oF, MatrixXf &oV, const float offset) {
	oF = F;
	oV = V;

	MatrixXf N;	// vertex normals
	computeVertexNormals(F, V, N);

	// Warning: ignore self-intersection here!!
	oV += offset * N;
}

void solvePatternAssiganmentInconsistency(const MatrixXu &F, MatrixXu &P, uint32_t f) {
	bool *visited = new bool[F.cols()];
}

void computePrimsSplittingPattern(const MatrixXu &F, MatrixXu &P) {
	P.resize(F.rows(), F.cols());
//	P.setZero();	// Pattern = None
	memset(P.data(), SPLIT_PATTERN_NONE, P.size() * sizeof(P(0, 0)));

	EdgeToAdjacentTrianglesMap adjacentMap;
	buildEdgeAdjacentTrianglesTable(F, adjacentMap);

	std::function<SPLIT_PATTEN(uint32_t, uint32_t, uint32_t)> getEdgePattern;
	getEdgePattern = [&F, &P](uint32_t f, uint32_t p0, uint32_t p1) { 
		for (int i = 0; i < 3; i++) {
			int j = (i == 2 ? 0 : i + 1);
			if ((F(i, f) == p0 && F(j, f) == p1) || (F(i, f) == p1 && F(j, f) == p0)) {
				return static_cast<SPLIT_PATTEN>(P(i, f));
			}
		}
		std::cerr << "Find Edge pattern error: the face does not have the input edge/points" << std::endl;
		return SPLIT_PATTERN_NONE;
	};

	uint32_t trianglesCount = F.cols();
	for (uint32_t f = 0; f < trianglesCount; ++f) {
		uint32_t points[3] = { F(0, f), F(1, f), F(2, f) };

		std::string edges[3];
		for (int i = 0; i < 3; i++) {
			int j = (i == 2) ? 0 : i + 1;
			points[i] < points[j] ? (edges[i] = std::to_string(points[i]) + std::to_string(points[j]))
				: (edges[i] = std::to_string(points[j]) + std::to_string(points[i]));
		}

		int adjacentTriangles[3] = { -1, -1 , -1 };
		for (int i = 0; i < 3; ++i) {
			adjacentTriangles[i] = lookupEdgeAdjacentTriangle(f, points[i], points[i == 2 ? 0 : i + 1], adjacentMap);
		}

		SPLIT_PATTEN adjacentEdgePatterns[3] = { SPLIT_PATTERN_NONE, SPLIT_PATTERN_NONE, SPLIT_PATTERN_NONE };
		for (int i = 0; i < 3; ++i) {
			if (adjacentTriangles[i] != -1) {
				adjacentEdgePatterns[i] = getEdgePattern(adjacentTriangles[i], points[i], points[i == 2 ? 0 : i + 1]);
			}
		}

		SPLIT_PATTEN patterns[3] = { SPLIT_PATTERN_NONE, SPLIT_PATTERN_NONE, SPLIT_PATTERN_NONE };

		std::function<void()> assignSplittingPattern = [&]() {
			uint8_t cN = 0, cR = 0, cF = 0;

			/* Assign opposite pattern if adjacent edge pattern exist, or remain none pattern. */
			for (int i = 0; i < 3; ++i) {
				if (adjacentEdgePatterns[i] == SPLIT_PATTERN_R) patterns[i] = SPLIT_PATTERN_F, ++cF;
				else if (adjacentEdgePatterns[i] == SPLIT_PATTERN_F) patterns[i] = SPLIT_PATTERN_R, ++cR;
				else if (adjacentEdgePatterns[i] == SPLIT_PATTERN_NONE) patterns[i] = SPLIT_PATTERN_NONE, ++cN;
			}

			bool inconsistent = (cR == 3 || cF == 3);
			if (inconsistent) {
				// solve inconsistency, RRR->RRF, FFF->FFR
				solvePatternAssiganmentInconsistency();
			}
			else {
				/* remaining edges' pattern assignment strategy:
					NNN -> RFF,
					FNN	-> RR,	// only 1 F, 
					RNN	-> FF
					FFN	-> R
					RRN	-> F
					RFN	-> R */
				SPLIT_PATTEN remain[3] = { SPLIT_PATTERN_NONE, SPLIT_PATTERN_NONE, SPLIT_PATTERN_NONE };

				if (cN == 3) { remain[0] = SPLIT_PATTERN_R, remain[1] = remain[2] = SPLIT_PATTERN_F; }
				else if (cN == 2 && cF == 1) { remain[0] = remain[1] = SPLIT_PATTERN_R; }
				else if (cN == 2 && cR == 1) { remain[0] = remain[1] = SPLIT_PATTERN_F; }
				else if (cN == 1 && cF == 2) { remain[0] = SPLIT_PATTERN_R; }
				else if (cN == 1 && cR == 2) { remain[0] = SPLIT_PATTERN_F; }
				else if (cN == 1 && cF == 1 && cR == 1) { remain[0] = SPLIT_PATTERN_R; }

				std::function<void(SPLIT_PATTEN [])> assignRemainPattern = [&](SPLIT_PATTEN p[3]) {
					uint8_t c = 0;
					for (int i = 0; i < 3; ++i) {
						if (patterns[i] == SPLIT_PATTERN_NONE) {
							patterns[i] = p[c];
							++c;
						}
					}
				};
				assignRemainPattern(remain);
			}
		};
	}
}