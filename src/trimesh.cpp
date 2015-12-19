#include "trimesh.h"

TriMesh::TriMesh() {
	mV.resize(0, 0);
	mF.resize(0, 0);
	mN.resize(0, 0);
	mUV.resize(0, 0);
	mDPDU.resize(0, 0);
	mDPDV.resize(0, 0);
}

TriMesh::~TriMesh() {
	mV.resize(0, 0);
	mF.resize(0, 0);
	mN.resize(0, 0);
	mUV.resize(0, 0);
	mDPDU.resize(0, 0);
	mDPDV.resize(0, 0);
}

void TriMesh::free() {
	mV.resize(0, 0);
	mF.resize(0, 0);
	mN.resize(0, 0);
	mUV.resize(0, 0);
	mDPDU.resize(0, 0);
	mDPDV.resize(0, 0);
}