#version 410 core

in vec3 position, color;

out vec3 vertex_color;

uniform mat4 model_view_projection_matrix;

void main(void) {
    gl_Position =
        model_view_projection_matrix * vec4(position, 1.0);
	vertex_color = color;
}
