#version 410 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec4 left[];
in vec4 right[];
in vec2 parameter[];

out vec2 gs_parameter;
out float gs_u;

void main(void) {
    gl_Position = left[0];
    gs_parameter = parameter[0];
    gs_u = 0;
    EmitVertex();

    gl_Position = right[0];
    gs_parameter = parameter[0];
    gs_u = 1;
    EmitVertex();

    gl_Position = left[1];
    gs_parameter = parameter[1];
    gs_u = 0;
    EmitVertex();

    gl_Position = right[1];
    gs_parameter = parameter[1];
    gs_u = 1;
    EmitVertex();
}
