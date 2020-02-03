#version 410 core

in vec3 position, normal;
in vec2 texture_coordinates;
in float unit, slice;

out vec3 vertex_normal;
out vec2 vertex_texture_coordinates;
flat out uint vertex_unit, vertex_slice;

uniform view_properties {
    mat4 view_projection;
};

uniform mat4 model; // TODO: turn into attribute

void main(void) {
    gl_Position = view_projection * model * vec4(position, 1.0);

	vertex_normal = mat3(model) * normal;
	vertex_texture_coordinates = texture_coordinates;
	//vertex_texture_index = texture_index;
	vertex_unit = uint(unit);
	vertex_slice = uint(slice);
}
