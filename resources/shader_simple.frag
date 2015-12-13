#version 330

out vec4 color;
uniform vec3 vertexColor;

void main() {
	color = vec4(vertexColor, 1);
}

