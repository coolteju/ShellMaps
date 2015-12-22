/*
	meshio.h: Mesh file input/output routines
*/

#pragma once

#include "mycommon.h"

using nanogui::Vector3f;
using nanogui::Vector2f;
using nanogui::MatrixXf;
using nanogui::MatrixXu;

/* Simply call tiny object loader. Since the shell maps here assumes the underlying mesh shares vertex(using indices) and
 needs vertex texcoords, you may adjust the mesh's representation after calling this function. */
extern void loadObj(const std::string &filename, MatrixXu &F, MatrixXf &V, MatrixXf &UV);

/* Load mesh which shares vertex and does not share texcoords(but the uv values are same), for example: the meshes in Berkeley Garment Library.
http://graphics.berkeley.edu/resources/GarmentLibrary/index.html */
extern void loadObjShareVertexNotShareTexcoord(const std::string &filename, MatrixXu &F, MatrixXf &V, MatrixXf &UV);