#include "meshio.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void loadObj(const std::string &filename, std::vector<Vector3f> &V, std::vector<Vector3u> &F) {
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	bool ret = tinyobj::LoadObj(shapes, materials, err, filename.c_str());

	if (!err.empty()) std::cerr << err << std::endl;
	if (!ret) exit(1);

	if (shapes.empty()) std::cerr << "no shape in this mesh file: <" << filename << ">." << std::endl;

	tinyobj::shape_t shape = shapes[0];		// only handle the first shape

	// load V
	assert(shape.mesh.positions.size() % 3 == 0);
	size_t VSize = shape.mesh.positions.size() / 3;
	V.reserve(VSize);
	// row major or column major depended? ok, it's vector element, no dependency here!
	memcpy(V.data(), shape.mesh.positions.data(), sizeof(float) * VSize * 3);	//  both underlying data are float type

	// load F
	assert(shape.mesh.indices.size() % 3 == 0);
	size_t FSize = shape.mesh.indices.size() / 3;
	F.reserve(FSize);
	memcpy(F.data(), shape.mesh.indices.data(), sizeof(unsigned int) * FSize * 3);	// F.data: uint32, indices: unsigned int
}