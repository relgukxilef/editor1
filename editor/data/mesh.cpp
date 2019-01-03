#include "mesh.h"

using namespace glm;

namespace ge1 {

    bool mesh::pick_vertex(mat4 matrix, vec2 ndc, unsigned int& vertex_index) {
        unsigned int i = 0;
        for (auto& vertex : vertex_positions) {
            vec4 vertex_ndc = matrix * vec4(vertex, 1);
            vertex_ndc /= vertex_ndc.w;
            // TODO: respect aspect ratio
            if (length(vec2(vertex_ndc) - ndc) < 0.1f) {
                vertex_index = i;
                return true;
            }
            i++;
        }

        return false;
    }

    void mesh::add_vertex(vec3 position) {
        vertex_positions.push_back(position);
        get_vertex_selections().push_back(false);
    }

    void mesh::delete_face(unsigned int face) {
        for (unsigned int i = 0; i < 3; i++) {
            auto from = face_vertices.size() - 3 + i;
            auto to = face * 3 + i;
            vertex_face_vertices.erase({face_vertices[from], from});

            if (from != to) {
                vertex_face_vertices.erase({face_vertices[to], to});
                vertex_face_vertices.insert({face_vertices[from], to});
                face_vertices[to] = face_vertices[from];
                face_vertex_positions[to] = face_vertex_positions[from];
            }
        }

        for (unsigned int i = 0; i < 3; i++) {
            face_vertex_positions.pop_back();
            face_vertices.pop_back();
        }
    }
}
