#version 410 core

in vec3 vertex_normal;
in vec2 vertex_texture_coordinates;
flat in uint vertex_unit, vertex_slice;

out vec3 color;

vec3 rgb_to_srgb(vec3 rgb);

uniform sampler2DArray textures[4];

void main(void) {
    float light = dot(normalize(vertex_normal), vec3(0, 0, 1)) * 0.5 + 0.5;
    vec4 t = texture(
        textures[vertex_unit],
        vec3(vertex_texture_coordinates, vertex_slice)
    );
    color = t.rgb; //* pow(light, 1.0 / 2.2);
}
