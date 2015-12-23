#pragma once

#include "mycommon.h"

using nanogui::MatrixXu;
using nanogui::MatrixXf;

extern void computeVertexTangents(const MatrixXu &F, const MatrixXf &V, const MatrixXf &UV, MatrixXf &Dpdu, MatrixXf &Dpdv, bool angleWeight = true);
extern void computeFaceTangents(const MatrixXu &F, const MatrixXf &V, const MatrixXf &UV, MatrixXf &Dpdu, MatrixXf &Dpdv);