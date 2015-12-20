#include "meshio.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void loadObj(const std::string &filename, MatrixXu &F, MatrixXf &V, MatrixXf &UV) {
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	std::cout << "--Load mesh file ..." << std::endl;

	bool ret = tinyobj::LoadObj(shapes, materials, err, filename.c_str());

	if (!err.empty()) std::cerr << err << std::endl;
	if (!ret) exit(1);

	if (shapes.empty()) std::cerr << "no shape in this mesh file: <" << filename << ">." << std::endl;

	tinyobj::shape_t shape = shapes[0];		// only handle the first shape

	// load F
	assert(shape.mesh.indices.size() % 3 == 0);
	size_t FSize = shape.mesh.indices.size() / 3;
	F.resize(3, FSize);
	memcpy(F.data(), shape.mesh.indices.data(), sizeof(uint32_t) * FSize * 3);	// F.data: uint32, indices: unsigned int

	// load V
	assert(shape.mesh.positions.size() % 3 == 0);
	size_t VSize = shape.mesh.positions.size() / 3;
	V.resize(3, VSize);
	memcpy(V.data(), shape.mesh.positions.data(), sizeof(float) * VSize * 3);

	// load UV
	assert(shape.mesh.texcoords.size() % 2 == 0);
	size_t UVSize = shape.mesh.texcoords.size() / 2;
	UV.resize(2, UVSize);
	memcpy(UV.data(), shape.mesh.texcoords.data(), sizeof(float) * UVSize * 2);

	std::cout << "++Load mesh file done." << std::endl;
}