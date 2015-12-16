#include "adjacenttriangles.h"

void buildEdgeAdjacentTrianglesTable(const MatrixXu &F, EdgeToAdjacentTrianglesMap &adjacentMap) {
	int trianglesCount = F.cols();

	for (int f = 0; f < trianglesCount; ++f) {
		uint32_t points[3] = { F(0, f), F(1, f), F(2, f) };
		std::string edges[3];

		for (int i = 0; i < 3; i++) {
			int j = (i == 2) ? 0 : i + 1;
			points[i] < points[j] ? (edges[i] = std::to_string(points[i]) + std::to_string(points[j]))
				: (edges[i] = std::to_string(points[j]) + std::to_string(points[i]));
		}

		for (int i = 0; i < 3; i++) {
			EdgeToAdjacentTrianglesMap::iterator it = adjacentMap.find(edges[i]);

			if (it == adjacentMap.end()) {
				adjacentMap[edges[i]] = std::make_pair(f, -1);
			}
			else {
				if (it->second.second != -1) {
					std::cerr << "Invalid triangle mesh: over 2 triangles share 1 edge" << std::endl;
				}
				else {
					it->second.second = f;
				}
			}
		}
	}
}

int lookupEdgeAdjacentTriangle(uint32_t triangle, uint32_t p0, uint32_t p1, const EdgeToAdjacentTrianglesMap &adjacentMap) {
	std::string edge;

	if (p0 > p1) std::swap(p0, p1);
	edge = std::to_string(p0) + std::to_string(p1);

	EdgeToAdjacentTrianglesMap::const_iterator it = adjacentMap.find(edge);

	if (it != adjacentMap.end()) {
		int tri0 = it->second.first, tri1 = it->second.second;

		// TODO: consider to add more cout to print related information if errors occur.
		if (tri0 != -1 && static_cast<uint32_t>(tri0) != triangle) return tri0;
		if (tri1 != -1 && static_cast<uint32_t>(tri1) != triangle) return tri1;
		
//		if (tri0 == -1 && tri1 == -1) std::cerr << "The adjacent map does not have adjacent trianles with the given points" << std::endl;

		return -1;
	}
	else {
		std::cerr << "The adjacent map does not have the query edge!" << std::endl;
		return -1;
	}
}

