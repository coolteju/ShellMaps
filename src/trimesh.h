#pragma once

#include "mycommon.h"
#include <vector>

using nanogui::Vector3f;
using nanogui::Vector2f;

struct TangentSpace {
	Vector3f dpdu;
	Vector3f dpdv;

	inline TangentSpace	() { }
};


class TriMesh {
public:
	TriMesh();
	virtual ~TriMesh();

	inline const Vector3f* getVertexNormals();

protected:
	void computeVertexNormals();
	void computeFaceNormals();

private:
	std::string mName;
	Vector3f *mVertices;			// mesh vertices
	Vector3u *mTriangles;			// triangles indices
	Vector3f *mVertexNormals;		// per vertex outer direction vector, computed based on ajacent facets' normals which share the vertex
	Vector2f *mTexcoords;			// per vertex texcoords

	bool hasFaceNormals;
	bool hasVertexNormals;
};
