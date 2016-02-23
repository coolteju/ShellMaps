# A simple implementation of Shell Maps

'A shell map is a bijective mapping between shell space and texture space that can be used
to generate small-scale features on surfaces using a variety of modeling techniques. The 
method is based upon the generation of an offset surface and the construction of a tetrahedral
mesh that fills the space between the base surface and its offset'. (Defined in paper 
['Shell Maps'](http://dl.acm.org/citation.cfm?id=1073239), tog 05).

This is a personal implementation based on the original paper. The project builds on
[nanogui](https://github.com/wjakob/nanogui) for OpenGL viewing and
[Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page) for basic numerical computing.

## Description
The input can be a wavefront .obj file with uv coordinates, the procedure to generate shell
maps has following steps:

1. Offset surface generation. 
	- Compute vertex normals to generate an offset surface(mesh) 
	- Compute a tangent space for transforming vectors in textures
	- The surface intersection is not detected now

2. Prims and Tetrahedra Construction.
	- Generate prisms in shell space by connecting the vertices of triangles in base surface
	  with correspoding triangles in offset surface
	- Split prims into three tetrahedra to keep convexity

3. Maintaining Continous Tetrahedral Meshes
	- Each prism can be split into three tetrahedra in six ways. Some spliting ways may result
	  in inconsistency between adjacent prims. A DFS style searching is used to solving this.

Finally, a shell map can be generated. There is no well defined format to store this structure
in original paper. And this implementation save shell maps in the format found in code(src/volume/tetra.h)
of [ctcloth12](http://www.cs.cornell.edu/projects/ctcloth/data/).

## Compiling
You can complile this projcet like that in [nanogui](https://github.com/wjakob/nanogui). Since
nanogui currently works on Mac OS X (Clang) Linux (GCC or Clang) and Windows (Visual Studio ≥ 2015),
it requires a recent c++11 capable compiler. All dependencies are jointly built using a 
CMake-based build system.

## Usage
![Screenshot](https://github.com/dragonbook/nanogui/raw/master/resources/shellmaps.png "shell maps")

To get started, lauch the binary and select a .obj file using "Open" button.

Then, a simple workflow can be,
- Set offset value by adjusting slider under "offset value" panel
- Click "Generate" button to generate offset surface
- Click "Compute" button to compute splitting pattern
- Click "Construct" button to construct tetrahedron mesh
- Click "Save shell" button to save shell maps in a text file
- Click "Save bound" button to save the bounding mesh of shell space in a wavefront .obj file

Addtionally, several different rendering layers can be selected for viewing and debugging.