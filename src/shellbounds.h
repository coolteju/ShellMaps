#pragma once

#include "mycommon.h"

using nanogui::MatrixXu;
using nanogui::MatrixXf;

/* Generate a tight mesh 'box', bounding shell space between the base surface and offset surface */
extern void generateShellBoundSimple(const MatrixXu &bF, const MatrixXf &bV, const MatrixXf &oV, MatrixXu &boundF, MatrixXf &boundV);