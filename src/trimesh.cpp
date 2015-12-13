#include "trimesh.h"

TriMesh::TriMesh() {

}

TriMesh::~TriMesh() {

}

void TriMesh::free() {
	mV.resize(0, 0);
	mF.resize(0, 0);
	mUV.resize(0, 0);
	mVN.resize(0, 0);
}