#include "meshio.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <unordered_map>

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

	std::cout << "load mesh done. (V=" << V.cols() << ", F=" << F.cols() << ", UV=" << UV.cols() << std::endl;
}

void loadObjShareVertexNotShareTexcoord(const std::string &filename, MatrixXu &F, MatrixXf &V, MatrixXf &UV) {
	/// Vertex indices used by the OBJ format
	struct obj_vertex {
		uint32_t p = (uint32_t)-1;
		uint32_t n = (uint32_t)-1;
		uint32_t uv = (uint32_t)-1;

		inline obj_vertex() { }

		inline obj_vertex(const std::string &string) {
			std::vector<std::string> tokens = str_tokenize(string, '/', true);

			if (tokens.size() < 1 || tokens.size() > 3)
				throw std::runtime_error("Invalid vertex data: \"" + string + "\"");

			p = str_to_uint32_t(tokens[0]);

			if (tokens.size() >= 2 && !tokens[1].empty())
				uv = str_to_uint32_t(tokens[1]);

#if 0
			if (tokens.size() >= 3 && !tokens[2].empty())
				n = str_to_uint32_t(tokens[2]);
#endif
		}

		inline bool operator==(const obj_vertex &v) const {
//			return v.p == p && v.n == n && v.uv == uv;
			return v.p == p;	// Same vertex have same p in different faces, but with different uv(Index).
								// Of course, the uv(value) are same, see mesh files in Berkeley Garment Library.
								// http://graphics.berkeley.edu/resources/GarmentLibrary/index.html
		}
	};

	/// Hash function for obj_vertex
	struct obj_vertexHash : std::unary_function<obj_vertex, size_t> {
		std::size_t operator()(const obj_vertex &v) const {
			size_t hash = std::hash<uint32_t>()(v.p);
//			hash = hash * 37 + std::hash<uint32_t>()(v.uv);
//			hash = hash * 37 + std::hash<uint32_t>()(v.n);
			return hash;
		}
	};

	typedef std::unordered_map<obj_vertex, uint32_t, obj_vertexHash> VertexMap;

	std::ifstream is(filename);
	if (is.fail())
		throw std::runtime_error("Unable to open OBJ file \"" + filename + "\"!");
	std::cout << "Loading \"" << filename << "\" .. ";
	std::cout.flush();
	Timer<> timer;

	std::vector<Vector3f>   positions;
	std::vector<Vector2f>   texcoords;
	//std::vector<Vector3f>   normals;
	std::vector<uint32_t>   indices;
	std::vector<obj_vertex> vertices;
	VertexMap vertexMap;

	std::string line_str;
	while (std::getline(is, line_str)) {
		std::istringstream line(line_str);

		std::string prefix;
		line >> prefix;

		if (prefix == "v") {
			Vector3f p;
			line >> p.x() >> p.y() >> p.z();
			positions.push_back(p);
		}
		else if (prefix == "vt") {
			Vector2f tc;
			line >> tc.x() >> tc.y();
			texcoords.push_back(tc);
		}
		else if (prefix == "vn") {
			/*
			Vector3f n;
			line >> n.x() >> n.y() >> n.z();
			normals.push_back(n);
			*/
		}
		else if (prefix == "f") {
			std::string v1, v2, v3, v4;
			line >> v1 >> v2 >> v3 >> v4;
			obj_vertex tri[6];
			int nVertices = 3;

			tri[0] = obj_vertex(v1);
			tri[1] = obj_vertex(v2);
			tri[2] = obj_vertex(v3);

			if (!v4.empty()) {
				/* This is a quad, split into two triangles */
				tri[3] = obj_vertex(v4);
				tri[4] = tri[0];
				tri[5] = tri[2];
				nVertices = 6;
			}
			/* Convert to an indexed vertex list */
			for (int i = 0; i<nVertices; ++i) {
				const obj_vertex &v = tri[i];
				VertexMap::const_iterator it = vertexMap.find(v);
				if (it == vertexMap.end()) {
					vertexMap[v] = (uint32_t)vertices.size();
					indices.push_back((uint32_t)vertices.size());
					vertices.push_back(v);
				}
				else {
					indices.push_back(it->second);
				}
			}
		}
	}

	F.resize(3, indices.size() / 3);
	memcpy(F.data(), indices.data(), sizeof(uint32_t)*indices.size());

	V.resize(3, vertices.size());
	for (uint32_t i = 0; i<vertices.size(); ++i)
		V.col(i) = positions.at(vertices[i].p - 1);

	if (texcoords.size() > 0) {
		UV.resize(2, vertices.size());
		for (uint32_t i = 0; i < vertices.size(); ++i)
			UV.col(i) = texcoords.at(vertices[i].uv - 1);
	}

	std::cout << "done. (V=" << V.cols() << ", F=" << F.cols() << ", UV=" << UV.cols() << ", took "
		<< timeString(timer.value()) << ")" << std::endl;
}

void writeObj(const std::string filename, const MatrixXu &F, const MatrixXf &V) {
	std::cout << "Writing \"" << filename << "\" (V=" << V.cols()
		<< ", F=" << F.cols() << ") ..." << std::endl;
	std::ofstream os(filename);
	if (os.fail()) {
		throw std::runtime_error("Unable to open OBJ file \"" + filename + "\"!");
	}

	for (uint32_t v = 0; v < V.cols(); ++v) {
		os << "v " << V(0, v) << " " << V(1, v) << " " << V(2, v) << std::endl;
	}

	for (uint32_t f = 0; f < F.cols(); ++f) {
		os << "f " << F(0, f) + 1 << " " << F(1, f) + 1 << " " << F(2, f) + 1 << std::endl;
	}

	os.close();
	std::cout << "done." << std::endl;
}