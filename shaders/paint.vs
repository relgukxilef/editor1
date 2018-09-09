#version 410 core

in vec3 position;

out vec3 vs_position;

void main(void) {
	vs_position = position;
		//projection * view * model * vec4(position, 1.0);
}
