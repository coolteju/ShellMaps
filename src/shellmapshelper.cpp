#include "shellmapshelper.h"
#include "normal.h"
#include "adjacenttriangles.h"

void generateOffsetSurface(const MatrixXu &F, const MatrixXf &V, MatrixXu &oF, MatrixXf &oV, const float offset) {
	oF = F;
	oV = V;

	MatrixXf N;	// vertex normals
	computeVertexNormals(F, V, N);

	// Warning: ignore self-intersection here!!
	oV += offset * N;
}

void generateOffsetSurface(const MatrixXu &F, const MatrixXf &V, const MatrixXf &N, MatrixXu &oF, MatrixXf &oV, const float offset) {
	oF = F;
	oV = V;

	// N: vertex normals
	// Warning: ignore self-intersection here!!
	oV += offset * N;
}

void computePrimsSplittingPattern(const MatrixXu &F, MatrixXu &P) {
	P.resize(F.rows(), F.cols());
//	P.setZero();	// Pattern = None
	memset(P.data(), SPLIT_PATTERN_NONE, P.size() * sizeof(P(0, 0)));

	std::cout << "--Compute prims splitting pattern ..." << std::endl;

	EdgeToAdjacentTrianglesMap adjacentMap;
	buildEdgeAdjacentTrianglesTable(F, adjacentMap);

	/* Get edge pattern based on triangel and edge, supporting adjacent triangle query */
	auto getEdgePattern = [&F, &P](uint32_t f, uint32_t p0, uint32_t p1) -> SPLIT_PATTERN { 
		for (int i = 0; i < 3; ++i) {
			int j = (i == 2 ? 0 : i + 1);
			if ((F(i, f) == p0 && F(j, f) == p1) || (F(i, f) == p1 && F(j, f) == p0)) {
				return static_cast<SPLIT_PATTERN>(P(i, f));
			}
		}
		std::cerr << "Find Edge pattern error: the face does not have the input edge/points" << std::endl;
		return SPLIT_PATTERN_NONE;
	};

	auto setEdgePattern = [&F, &P](uint32_t f, uint32_t p0, uint32_t p1, SPLIT_PATTERN p) {
		for (int i = 0; i < 3; ++i) {
			int j = (i == 2 ? 0 : i + 1);
			if ((F(i, f) == p0 && F(j, f) == p1) || (F(i, f) == p1 && F(j, f) == p0)) {
				P(i, f) = static_cast<uint32_t>(p);
			}
		}
	};

	auto getAdjacentTriangles = [&F, &adjacentMap](uint32_t f, int adjacentTriangles[3]) {
		uint32_t points[3] = { F(0, f), F(1, f), F(2, f) };

		std::string edges[3];
		for (int i = 0; i < 3; i++) {
			int j = (i == 2) ? 0 : i + 1;
			points[i] < points[j] ? (edges[i] = std::to_string(points[i]) + std::to_string(points[j]))
				: (edges[i] = std::to_string(points[j]) + std::to_string(points[i]));
		}

		adjacentTriangles[0] = adjacentTriangles[1] = adjacentTriangles[2] = -1;
		for (int i = 0; i < 3; ++i) {
			adjacentTriangles[i] = lookupEdgeAdjacentTriangle(f, points[i], points[i == 2 ? 0 : i + 1], adjacentMap);
		}
	};

	/* Get three share edge pattern of the three adjacent triangles */
	auto getAdjacentEdgePatterns = [&F, &P, &adjacentMap, &getEdgePattern, &getAdjacentTriangles](uint32_t f, SPLIT_PATTERN adjacentEdgePatterns[3]) {
		int adjacentTriangles[3];
		getAdjacentTriangles(f, adjacentTriangles);

		uint32_t points[3] = { F(0, f), F(1, f), F(2, f) };

		adjacentEdgePatterns[0] = adjacentEdgePatterns[1] = adjacentEdgePatterns[2] = SPLIT_PATTERN_NONE;
		for (int i = 0; i < 3; ++i) {
			if (adjacentTriangles[i] != -1) {
				adjacentEdgePatterns[i] = getEdgePattern(adjacentTriangles[i], points[i], points[i == 2 ? 0 : i + 1]);
			}
		}
	};

	/* Get three edge patterns of one triangle */
	auto getTriangleEdgePatterns = [&P](uint32_t f, SPLIT_PATTERN triangleEdgePatterns[3]) {
		for (int i = 0; i < 3; ++i) triangleEdgePatterns[i] = static_cast<SPLIT_PATTERN>(P(i, f));
	};

	uint32_t trianglesCount = F.cols();
	for (uint32_t f = 0; f < trianglesCount; ++f) {
		SPLIT_PATTERN adjacentEdgePatterns[3];
		getAdjacentEdgePatterns(f, adjacentEdgePatterns);

		SPLIT_PATTERN patterns[3];

		std::function<void()> assignSplittingPattern = [&]() {
			uint8_t cN = 0, cR = 0, cF = 0;

			/* Assign opposite pattern if adjacent edge pattern exist, or remain none pattern. */
			for (int i = 0; i < 3; ++i) {
				if (adjacentEdgePatterns[i] == SPLIT_PATTERN_R) patterns[i] = SPLIT_PATTERN_F, ++cF;
				else if (adjacentEdgePatterns[i] == SPLIT_PATTERN_F) patterns[i] = SPLIT_PATTERN_R, ++cR;
				else if (adjacentEdgePatterns[i] == SPLIT_PATTERN_NONE) patterns[i] = SPLIT_PATTERN_NONE, ++cN;
			}

			bool inconsistent = (cR == 3 || cF == 3);
			if (! inconsistent) {
				/* Remaining edges' pattern filling strategy:
				NNN -> RFF,
				FNN	-> RR,	// only 1 F,
				RNN	-> FF
				FFN	-> R
				RRN	-> F
				RFN	-> R */
				SPLIT_PATTERN remain[3] = { SPLIT_PATTERN_NONE, SPLIT_PATTERN_NONE, SPLIT_PATTERN_NONE };

				if (cN == 3) { remain[0] = SPLIT_PATTERN_R, remain[1] = remain[2] = SPLIT_PATTERN_F; }
				else if (cN == 2 && cF == 1) { remain[0] = remain[1] = SPLIT_PATTERN_R; }
				else if (cN == 2 && cR == 1) { remain[0] = remain[1] = SPLIT_PATTERN_F; }
				else if (cN == 1 && cF == 2) { remain[0] = SPLIT_PATTERN_R; }
				else if (cN == 1 && cR == 2) { remain[0] = SPLIT_PATTERN_F; }
				else if (cN == 1 && cF == 1 && cR == 1) { remain[0] = SPLIT_PATTERN_R; }

				auto FillRemainPattern = [&patterns](SPLIT_PATTERN p[3]) {
					uint8_t c = 0;
					for (int i = 0; i < 3; ++i) {
						if (patterns[i] == SPLIT_PATTERN_NONE) {
							patterns[i] = p[c];
							++c;
						}
					}
				};
				FillRemainPattern(remain);
				P(0, f) = patterns[0], P(1, f) = patterns[1], P(2, f) = patterns[2]; 
			}
			else {
				// Firstly assign the inconsistent pattern, and then flip one edge to sovle it.
				P(0, f) = patterns[0], P(1, f) = patterns[1], P(2, f) = patterns[2];	// three Rs or thee Fs

				// Solve inconsistency, RRR->RRF, FFF->FFR
				// DFS style to solve it
				bool *visited = new bool[F.cols()];
				memset(visited, false, sizeof(bool) * F.cols());

				std::function<bool(uint32_t f)> solveInconsistencyRecursively;	// can not use auto below, must be declearation directly to capture
				solveInconsistencyRecursively = [&F, &P, &adjacentMap, &visited, &solveInconsistencyRecursively,
					&getAdjacentEdgePatterns, &getAdjacentTriangles, &getTriangleEdgePatterns, &setEdgePattern](uint32_t f) -> bool {

					/* Can solve it directly by assigning another suitable pattern?
					This can work if there exists free edges which have no adjacent triangle or have not been assign a splitting pattern. */
					SPLIT_PATTERN adjacentEdgePatterns[3];
					getAdjacentEdgePatterns(f, adjacentEdgePatterns);

					uint8_t cN = 0;
					int freeEdge = -1;	// It means there is no adjacent(or have not assign a pattern) triangle which share this edge
					for (int i = 0; i < 3; i++) { 
						if (adjacentEdgePatterns[i] == SPLIT_PATTERN_NONE) {
							++cN;
							freeEdge = i;
							break;
						}
					}

					if (cN > 0) {
						// Solve it directly, just flip the pattern on free edge
						if (P(freeEdge, f) == SPLIT_PATTERN_R) P(freeEdge, f) = SPLIT_PATTERN_F;
						else  P(freeEdge, f) = SPLIT_PATTERN_R;

						return true;
					}
					else {
						/* Or can solve it by just flipping one edge pattern of this and adjacent triangle?
						For example:
						This triangle: (FFF)(inconsistent), one adjacent triangle has edge patterns: (RRF), then the inconsistency can be solved by
						flipping the share edge's pattern, the result is, this triangle(RFF), adjacent triangle(FRF).
						*/
						int adjacentTriangles[3];
						getAdjacentTriangles(f, adjacentTriangles);

						// The adjacentTriangles here are  nonzero, check this explictly if results are not correct.
						SPLIT_PATTERN ajacentTrianglesEdgePatterns[3][3];
						for (int i = 0; i < 3; ++i)  getTriangleEdgePatterns(adjacentTriangles[i], ajacentTrianglesEdgePatterns[i]);

						uint8_t rR;
						if (P(0, f) == SPLIT_PATTERN_R) rR = 1;
						else rR = 2;

						int edgeId = -1;
						int freeAdjacentTriangle = -1;
						for (int i = 0; i < 3; ++i) {
							uint8_t cR = 0;
							for (int j = 0; j < 3; ++j) { if (ajacentTrianglesEdgePatterns[i][j] == SPLIT_PATTERN_R) ++cR; }

							if (cR == rR) {
								edgeId = i;
								freeAdjacentTriangle = adjacentTriangles[i];
								break;
							}
						}

						if (freeAdjacentTriangle >= 0) {
							SPLIT_PATTERN p = static_cast<SPLIT_PATTERN>(P(edgeId, f));
							SPLIT_PATTERN flip = (p == SPLIT_PATTERN_R ? SPLIT_PATTERN_F : SPLIT_PATTERN_R);

							P(edgeId, f) = static_cast<uint32_t>(flip);
							if (p == SPLIT_PATTERN_NONE) {
								std::cerr << "Error: set an edge with pattern None! at adjacent face(" << freeAdjacentTriangle
									<< ") when deal with face(" << f << ")." << std::endl;
								std::cout << "Information: ----------------------" << std::endl;
								std::cout << "edge patterns on face(" << f << ") are: (" << P(0, f) << P(1, f) << P(2, f) << ")." << std::endl;
								std::cout << "edge patterns on adjacent face(" << freeAdjacentTriangle << ") are: (" << P(0, freeAdjacentTriangle) << P(1, freeAdjacentTriangle) << P(2, freeAdjacentTriangle) << ")." << std::endl;
								std::cout << "-----------------------------------" << std::endl;
							}
							setEdgePattern(freeAdjacentTriangle, F(edgeId, f), F((edgeId == 2 ? 0 : edgeId + 1), f), p);

							return true;
						}
						else {
							// Start DFS search, random flip one share edge's pattern of adjacent unvisited triangle
							visited[f] = true;

							for (int i = 0; i < 3; ++i) {
								if (!visited[adjacentTriangles[i]]) {
									SPLIT_PATTERN p = static_cast<SPLIT_PATTERN>(P(i, f));
									SPLIT_PATTERN flip = (p == SPLIT_PATTERN_R ? SPLIT_PATTERN_F : SPLIT_PATTERN_R);

									P(i, f) = static_cast<uint32_t>(flip);
									setEdgePattern(adjacentTriangles[i], F(i, f), F((i == 2 ? 0 : i + 1), f), p);

									// check if occurs new inconsistency?
									// it does make new inconsistency, since all possible sovling situations have been considered?!
									if (solveInconsistencyRecursively(adjacentTriangles[i])) return true;
									else {
										P(i, f) = p;
										setEdgePattern(adjacentTriangles[i], F(i, f), F((i == 2 ? 0 : i + 1), f), flip);
									}
								}
							}

							return false;
						}
					}
				};
				solveInconsistencyRecursively(f);
			}
		};
		assignSplittingPattern();
	}

	/* Check if pattern P is consistency */
	#if 1
	std::cout << "check if the resulting pattern P is consistent(correct): " << std::endl;
	bool consistent = true;
	for (uint32_t f = 0; f < trianglesCount; ++f) {
		SPLIT_PATTERN edgePatterns[3];
		getTriangleEdgePatterns(f, edgePatterns);

		bool hasUnrecognizedPattern = false;

		uint8_t cR = 0, cF = 0, cN = 0;
		for (int i = 0; i < 3; ++i) {
			if (SPLIT_PATTERN_R == edgePatterns[i]) cR++;
			else if (SPLIT_PATTERN_F == edgePatterns[i]) cF++;
			else if (SPLIT_PATTERN_NONE == edgePatterns[i]) cN++;
			else {
				hasUnrecognizedPattern = true;
				std::cout << "No, edge(" << i << ") at face(" << f << ") has unrecognized pattern(" << edgePatterns[i] << ")." << std::endl;
				break;
			}
		}
		if (hasUnrecognizedPattern) { consistent = false; break; }

		if (cN > 0) {
//			std::cout << "No, edge(" << i << ") at face(" << f << ") has no pattern." << std::endl;
			std::cout << "No, face(" << f << ") has no pattern on one edge." << std::endl;
			std::cout << "Information: ------------------" << std::endl;
			std::cout << "Edge patterns: (" << edgePatterns[0] << "," << edgePatterns[1] << "," << edgePatterns[2] << ")." << std::endl;
			std::cout << "face points: (" << F(0, f) << "," << F(1, f) << "," << F(2, f) << ")." << std::endl;
			std::cout << "-------------------------------" << std::endl;
			break;
		}
		else if (cR == 3 || cF == 3) {
			std::string tmp = (cR == 3 ? "RRR" : "FFF");
			std::cout << "No, face(" << f << ") has inconsistent pattern(" << tmp << ")."  << std::endl;
			consistent = false;
			break;
		}

		SPLIT_PATTERN adjacentEdgePatterns[3];
		getAdjacentEdgePatterns(f, adjacentEdgePatterns);

		// Declear it as int, since the triangle may not have complete three adjacent triangles, if not, set -1 respectively.
		int adjacentTriangles[3];
		getAdjacentTriangles(f, adjacentTriangles);

		bool hasSamePattern = false;
		for (int i = 0; i < 3; ++i) {
			if (edgePatterns[i] == adjacentEdgePatterns[i]) {
				hasSamePattern = true;
				std::cout << "No, edge(" << i << ") at face(" << f << ") has same pattern(" << edgePatterns[i]
					<< ") with adjacent face(" << adjacentTriangles[i] << ")." << std::endl;
				break;
			}
		}
		if (hasSamePattern) { consistent = false; break; }
	}
	if (consistent) { std::cout << "Yes" << std::endl; }
	#endif

	std::cout << "++Compute prims splitting pattern done." << std::endl;
}

void constructTetrahedronMeshSimple(const MatrixXu &bF, const MatrixXf &bV, const MatrixXf &oV,
	const MatrixXf &bUV, const MatrixXf &bN, const MatrixXf &bDPDU, const MatrixXf &bDPDV,
	const MatrixXu &P, TetrahedronMesh &tetrahedronMesh) {
	MatrixXu F, T;	// T: tetra
	MatrixXf V, N, UV, DPDU, DPDV;

	std::cout << "--Construct tetrahedron mesh ..." << std::endl;

	MatrixXu oF;
	oF.resize(bF.rows(), bF.cols());
	oF.setConstant(bV.cols());
	oF += bF;

	MatrixXf bUV3, oUV3;
	bUV3.resize(3, bUV.cols());
	oUV3.resize(3, bUV.cols());
	for (uint32_t uv = 0; uv < bUV.cols(); ++uv) {
		bUV3.col(uv) << bUV.col(uv), 0.0f;
		oUV3.col(uv) << bUV.col(uv), 1.0f;
	}

	auto combine = [](MatrixXf &dst, const MatrixXf &b, const MatrixXf &o) {
		dst.resize(b.rows(), b.cols() + o.cols());
		memcpy(dst.data(), b.data(), sizeof(b(0, 0)) * b.size());
		memcpy(reinterpret_cast<uint8_t *>(dst.data()) + sizeof(float) * b.size(), o.data(), sizeof(float) * o.size());
	};

	F.resize(bF.rows(), bF.cols() + oF.cols());
	memcpy(F.data(), bF.data(), sizeof(uint32_t) * bF.size());
	memcpy(reinterpret_cast<uint8_t *>(F.data()) + sizeof(uint32_t) * bF.size(), oF.data(), sizeof(uint32_t) * oF.size());
	combine(V, bV, oV);
	combine(N, bN, bN);
	combine(UV, bUV3, oUV3);
	combine(DPDU, bDPDU, bDPDU);
	combine(DPDV, bDPDV, bDPDV);

	auto constructTetrahedraFromPrimsSimple = [](const MatrixXu &F, const MatrixXu &oF, const MatrixXu &P, MatrixXu &T) {
		uint32_t trianglesCount = F.cols();
		T.resize(4, 3 * trianglesCount);

		for (uint32_t f = 0; f < trianglesCount; ++f) { // for each trianlge of base mesh, that means, for each prim
			uint32_t bP[3] = { F(0, f), F(1, f), F(2, f) };
			uint32_t oP[3] = { oF(0, f), oF(1, f), oF(2, f) };
			uint32_t p[3] = { P(0, f), P(1, f), P(2, f) };

			/* Construct each of the three tetrahedra in a prism by iterating counter clockwise edge tag and
			the next counter clockwise edge tag. */
			for (int i = 0; i < 3; ++i) {
				int j = (i == 2 ? 0 : i + 1);
				int k = (j == 2 ? 0 : j + 1);
				uint32_t t = 3 * f + i;	// tehetradron id

				if (SPLIT_PATTERN_F == static_cast<SPLIT_PATTERN>(p[i]) && SPLIT_PATTERN_F == static_cast<SPLIT_PATTERN>(p[j])) {
					T(0, t) = oP[i], T(1, t) = bP[j], T(2, t) = oP[j], T(3, t) = bP[k];
				}
				else if (SPLIT_PATTERN_F == static_cast<SPLIT_PATTERN>(p[i]) && SPLIT_PATTERN_R == static_cast<SPLIT_PATTERN>(p[j])) {
					T(0, t) = oP[i], T(1, t) = bP[j], T(2, t) = oP[j], T(3, t) = oP[k];
				}
				else if (SPLIT_PATTERN_R == static_cast<SPLIT_PATTERN>(p[i]) && SPLIT_PATTERN_R == static_cast<SPLIT_PATTERN>(p[j])) {
					T(0, t) = bP[i], T(1, t) = bP[j], T(2, t) = oP[j], T(3, t) = oP[k];
				}
				else if (SPLIT_PATTERN_R == static_cast<SPLIT_PATTERN>(p[i]) && SPLIT_PATTERN_F == static_cast<SPLIT_PATTERN>(p[j])) {
					T(0, t) = bP[i], T(1, t) = bP[j], T(2, t) = oP[j], T(3, t) = bP[k];
				}
				else {
					std::cerr << "Invalid prism splitting pattern found." << std::endl;
					return;
				}
			}
		}
	};
	constructTetrahedraFromPrimsSimple(bF, oF, P, T);

	tetrahedronMesh.setTetrahedronMesh(std::move(V), std::move(N), std::move(UV), std::move(DPDU), std::move(DPDV), std::move(T));
	std::cout << "++Construct tetrahedron mesh done." << std::endl;
}

void saveShellToMitsuba(const std::string &filename, const TetrahedronMesh &shell) {
	FILE *fout = fopen(filename.c_str(), "wt");

	if (fout) {
		uint32_t vertexCount = shell.getVertexCount(), tetrahedronCount = shell.getTetrahedronCount();
		const MatrixXf &V = shell.V(), &UV = shell.UV(), &N = shell.N(), &DPDU = shell.DPDU(), &DPDV = shell.DPDV();
		const MatrixXu &T = shell.T();

		fprintf(fout, "%u %u", vertexCount, tetrahedronCount);
		uint32_t i;
		for (i = 0; i < vertexCount; ++i) {
			fprintf(fout, "%f %f %f\n", V(0, i), V(1, i), V(2, i));
			fprintf(fout, "%f %f %f\n", UV(0, i), UV(1, i), UV(2, i));
			fprintf(fout, "%f %f %f\n", N(0, i), N(1, i), N(2, i));
			fprintf(fout, "%f %f %f %f %f %f\n", DPDU(0, i), DPDU(1, i), DPDU(2, i), DPDV(0, i), DPDV(1, i), DPDV(2, i));
		}
		for (i = 0; i < tetrahedronCount; ++i) {
			fprintf(fout, "%u %u %u %u\n", T(0, i), T(1, i), T(2, i), T(3, i));
		}
	}

	fclose(fout);
}