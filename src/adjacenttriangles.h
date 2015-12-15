#pragma once

#include "mycommon.h"
#include <unordered_map>

using nanogui::MatrixXf;
using nanogui::MatrixXu;

typedef std::unordered_map<std::string, std::pair<int, int>> AdjacentTrianglesTable;

struct EdgeAdjacentTrianglesTable {
	AdjacentTrianglesTable adjacent;

	int getAdjacentTriangle(const uint32_t trianlge, const uint32_t edgeStartPoint, const uint32_t edgeEndPoint);

	EdgeAdjacentTrianglesTable() { }
};

extern EdgeAdjacentTrianglesTable buildEdgeAdjacentTrianglesTable(const MatrixXu &F, const MatrixXf &V);