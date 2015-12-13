#version 330

out vec4 color;
uniform float intensity;

void main() {
	color = vec4(1.0 * intensity, 0.2, 0.2, 1);
}

