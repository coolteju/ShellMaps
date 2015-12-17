#include "adjacenttriangles.h"

void buildEdgeAdjacentTrianglesTable(const MatrixXu &F, EdgeToAdjacentTrianglesMap &adjacentMap) {
	std::cout << "--Build edge adjacent triangles lookup table..." << std::endl;
	int trianglesCount = F.cols();

	for (int f = 0; f < trianglesCount; ++f) {
		uint32_t points[3] = { F(0, f), F(1, f), F(2, f) };
		Edge edges[3];

		for (int i = 0; i < 3; i++) {
			int j = (i == 2) ? 0 : i + 1;
			if (points[i] < points[j]) { edges[i].p0 = points[i]; edges[i].p1 = points[j]; }
			else { edges[i].p0 = points[j]; edges[i].p1 = points[i]; }
		}

		for (int i = 0; i < 3; i++) {
			EdgeToAdjacentTrianglesMap::iterator it = adjacentMap.find(edges[i]);

			if (it == adjacentMap.end()) {
				adjacentMap[edges[i]] = std::make_pair(f, -1);
			}
			else {
				if (it->second.second != -1) {
					std::cerr << "Invalid triangle mesh: over 2 triangles share 1 edge" << std::endl;
					std::cout << "Information: -------------------------------\n";
					std::cout << "faces: (" << it->second.first << ", " << it->second.second << ", " << f << ").\n";
					uint16_t t = it->second.first;
					std::cout << "face(" << t << "): (" << F(0, t) << "," << F(1, t) << "," << F(2, t) << ").\n";
					t = it->second.second;
					std::cout << "face(" << t << "): (" << F(0, t) << "," << F(1, t) << "," << F(2, t) << ").\n";
					t = f;
					std::cout << "face(" << t << "): (" << F(0, t) << "," << F(1, t) << "," << F(2, t) << ").\n";
					std::cout << "--------------------------------------------\n";
				}
				else {
					it->second.second = f;
				}
			}
		}
	}
	std::cout << "++Build edge adjacent table done" << std::endl;
}

int lookupEdgeAdjacentTriangle(uint32_t triangle, uint32_t p0, uint32_t p1, const EdgeToAdjacentTrianglesMap &adjacentMap) {
	if (p0 > p1) std::swap(p0, p1);
	Edge edge(p0, p1);

	EdgeToAdjacentTrianglesMap::const_iterator it = adjacentMap.find(edge);

	if (it != adjacentMap.end()) {
		int tri0 = it->second.first, tri1 = it->second.second;

		// TODO: consider to add more cout to print related information if errors occur.
		if (tri0 != -1 && static_cast<uint32_t>(tri0) != triangle) return tri0;
		if (tri1 != -1 && static_cast<uint32_t>(tri1) != triangle) return tri1;
		
		if (tri0 == -1 && tri1 == -1) std::cerr << "The adjacent map does not have adjacent trianles with the given points" << std::endl;

		return -1;
	}
	else {
		std::cerr << "The adjacent map does not have the query edge!" << std::endl;
		return -1;
	}
}

