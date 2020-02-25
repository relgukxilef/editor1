#version 430
layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0) readonly buffer edge_vertices_buffer {
    uint edge_vertices[];
};

layout(binding = 1) readonly buffer edge_neighbours_buffer {
    uint edge_neighbours[];
};

layout(binding = 2) readonly buffer vertices_buffer {
    float vertices[];
};

layout(binding = 3) writeonly buffer outline_faces_buffer {
    uint outline_faces[];
};

layout(binding = 4) buffer counter_buffer {
    uint counter;
};

layout(binding = 5) writeonly buffer vertex_outline_neighbours_buffer {
    uint vertex_outline_neighbours[];
};

uniform mat4 model_view_projection_matrix;

vec4 get_position(uint vertex) {
    uint index = vertex * 3;
    return model_view_projection_matrix * vec4(
        vertices[index], vertices[index + 1], vertices[index + 2], 1.0
    );
}

float side(vec4 a, vec4 b, vec4 c) {
    vec4 offset_a = a - c;
    vec4 offset_b = b - c;
    vec4 vertex = c;
    return cross(
        offset_a.xyz * vertex.w - vertex.xyz * offset_a.w,
        offset_b.xyz * vertex.w - vertex.xyz * offset_b.w
    ).z * vertex.w;
}

void main(void) {
    uint id = gl_GlobalInvocationID.x * 2;

    // clip space transformation is performed ~12 times per vertex
    uint start = edge_vertices[id];
    uint end = edge_vertices[id + 1];

    vec4 edge_a = get_position(start);
    vec4 edge_b = get_position(end);
    vec4 neighbour_a = get_position(edge_neighbours[id]);
    vec4 neighbour_b = get_position(edge_neighbours[id + 1]);

    // side is calculated three times per face
    // but since this shader is memory bound it might not be a good idea to
    // compute it in a separate step
    float side_a = side(edge_a, edge_b, neighbour_a);
    float side_b = side(edge_b, edge_a, neighbour_b);

    //float edge_side = side(edge_a, edge_b, (neighbour_a + neighbour_b) * 0.5);
    float edge_side = side_a - side_b;

    if (edge_side > 0) {
        uint temp = start;
        start = end;
        end = temp;
    }

    if (side_a * side_b <= 0) {// && (side_a >= 0 || side_b >= 0)) {
        // two vertices
        uint position = atomicAdd(counter, 1) * 2;

        // TODO: might be better to pack starts and ends
        // instead of interleaving them
        outline_faces[position] = edge_vertices[id];
        outline_faces[position + 1] = edge_vertices[id + 1];
        vertex_outline_neighbours[start * 2] = end;
        vertex_outline_neighbours[end * 2 + 1] = start;
    }
}
