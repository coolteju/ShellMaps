/*
	meshio.h: Mesh file input/output routines
*/

#pragma once

#include "mycommon.h"

using nanogui::Vector3f;
using nanogui::Vector2f;

extern void loadObj(const std::string &filename, std::vector<Vector3f> &V, std::vector<Vector3u> &F);