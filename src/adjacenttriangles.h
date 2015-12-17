#pragma once

#include "mycommon.h"
#include <unordered_map>

using nanogui::MatrixXf;
using nanogui::MatrixXu;

struct Edge {
	uint32_t p0 = (uint32_t) -1;
	uint32_t p1 = (uint32_t) -1;

	inline Edge() { }
	inline Edge(uint32_t point0, uint32_t point1) : p0(point0), p1(point1) { }

	inline bool operator==(const Edge &e) const {
		return e.p0 == p0 && e.p1 == p1;
	}
};

struct EdgeHash : std::unary_function<Edge, size_t> {
	std::size_t operator()(const Edge &e) const {
		size_t hash = std::hash<uint32_t>()(e.p0);
		hash = hash * 37 + std::hash<uint32_t>()(e.p1);
		return hash;
	}
};

/* 
   Edge to adjacent triangles mapping, supporting triangles mesh.  Map edge(string: pId0_str + pId1_str)
   to the two adjacent triangles ids(pair<int, int>). pId0 must be smaller than pId1, thus creating the same
   key for an edge.
   If an edge only have one adjacent triangle, 'id', then the pair value is <id, -1>.
*/
typedef std::unordered_map<Edge, std::pair<int, int>, EdgeHash> EdgeToAdjacentTrianglesMap;


extern void buildEdgeAdjacentTrianglesTable(const MatrixXu &F, EdgeToAdjacentTrianglesMap &adjacentMap);

/* Lookup the adjacent triangle with given current triangle id and the edge. Return adjancent triangle id if exists, or return -1. */
extern int lookupEdgeAdjacentTriangle(uint32_t triangle, uint32_t p0, uint32_t p1, const EdgeToAdjacentTrianglesMap &adjacentMap);