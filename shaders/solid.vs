#version 410 core

in vec3 position, color;

out vec3 vertex_color;

uniform view_properties {
    mat4 view_projection;
};

uniform mat4 model; // TODO: turn into attribute

void main(void) {
    gl_Position = view_projection * model * vec4(position, 1.0);
	vertex_color = color;
}
