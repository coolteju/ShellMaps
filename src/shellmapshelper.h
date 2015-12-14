#pragma once

#include "mycommon.h"
#include "normal.h"

using nanogui::MatrixXf;
using nanogui::MatrixXu;

extern void generateOffsetSurface(const MatrixXu &F, const MatrixXf &V, MatrixXu &oF, MatrixXf &oV, const float offset);