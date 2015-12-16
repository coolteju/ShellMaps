#pragma once

#include "mycommon.h"
#include <unordered_map>

using nanogui::MatrixXf;
using nanogui::MatrixXu;

/* 
   Edge to adjacent triangles mapping, supporting triangles mesh.  Map edge(string: pId0_str + pId1_str)
   to the two adjacent triangles ids(pair<int, int>). pId0 must be smaller than pId1, thus creating the same
   key for an edge.
   If an edge only have one adjacent triangle, 'id', then the pair value is <id, -1>.
*/
typedef std::unordered_map<std::string, std::pair<int, int>> EdgeToAdjacentTrianglesMap;


extern void buildEdgeAdjacentTrianglesTable(const MatrixXu &F, EdgeToAdjacentTrianglesMap &adjacentMap);

/* Lookup the adjacent triangle with given current triangle id and the edge. Return adjancent triangle id if exists, or return -1. */
extern int lookupEdgeAdjacentTriangle(uint32_t triangle, uint32_t p0, uint32_t p1, const EdgeToAdjacentTrianglesMap &adjacentMap);