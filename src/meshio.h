/*
	meshio.h: Mesh file input/output routines
*/

#pragma once

#include "mycommon.h"

using nanogui::Vector3f;
using nanogui::Vector2f;
using nanogui::MatrixXf;
using nanogui::MatrixXu;

extern void loadObj(const std::string &filename, MatrixXu &F, MatrixXf &V);