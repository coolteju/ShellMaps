#pragma once

#include "mycommon.h"
#include "normal.h"
#include "adjacenttriangles.h"

using nanogui::MatrixXf;
using nanogui::MatrixXu;

extern void generateOffsetSurface(const MatrixXu &F, const MatrixXf &V, MatrixXu &oF, MatrixXf &oV, const float offset);

enum SPLIT_PATTEN {
	SPLIT_PATTERN_NONE = 0,
	SPLIT_PATTERN_R,
	SPLIT_PATTERN_F,
	SPLIT_PATTEN_COUNT
};

/* The computing result Pattern P has the same dimenstions as F, P.col(i) records the splitting pattern for base triangle F.col(i) in prim(i).
	For example:
	P(0, i) = 1: edge0(p0->p1) in triangle i has splitting pattern R,
	P(0, i) = 2: edge0(p0->p1) in triangle i has splitting pattern F.
	P(0, i) = 0: edge0(p0->p1) in triangle i has not assigend a pattern.
*/
extern void computePrimsSplittingPattern(const MatrixXu &F, MatrixXu &P);