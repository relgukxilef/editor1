#version 410 core

in vec3 vertex_color;

out vec4 color;

float edge(float edge, float width, float x) {
    return clamp((x - edge + width * 0.5) / width, 0, 1);
}

void main(void) {
    float radius = length(gl_PointCoord - vec2(0.5)) * 2;
    if (radius > 1) {
        discard;
    }

    float width = fwidth(radius);

    color = vec4(
        vertex_color * (1 - edge(3.0/4.0, width, radius)), 1
    );
}
