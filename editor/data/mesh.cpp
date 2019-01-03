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
            auto last = face_vertices.size() - 3 + i;
            auto to = face * 3 + i;
            vertex_face_vertices.erase({face_vertices[last], last});

            if (last != to) {
                vertex_face_vertices.erase({face_vertices[to], to});
                vertex_face_vertices.insert({face_vertices[last], to});
                face_vertices[to] = face_vertices[last];
                face_vertex_positions[to] = face_vertex_positions[last];
            }
        }

        for (unsigned int i = 0; i < 3; i++) {
            face_vertex_positions.pop_back();
            face_vertices.pop_back();
        }
    }

    void mesh::delete_vertex(unsigned int vertex) {
        auto face_vertex = vertex_face_vertices.lower_bound({vertex, 0});
        while (
            face_vertex != vertex_face_vertices.lower_bound({vertex + 1, 0})
        ) {
            delete_face(face_vertex->second / 3);
            face_vertex = vertex_face_vertices.lower_bound({vertex, 0});
        }

        unsigned int last = vertex_positions.size() - 1;

        if (last != vertex) {
            auto face_vertex = vertex_face_vertices.lower_bound({last, 0});
            while (face_vertex != vertex_face_vertices.end()) {
                face_vertices[face_vertex->second] = vertex;

                vertex_face_vertices.erase({last, face_vertex->second});
                vertex_face_vertices.insert({vertex, face_vertex->second});

                face_vertex = vertex_face_vertices.lower_bound({last, 0});
            }
            vertex_positions[vertex] = vertex_positions[last];
            get_vertex_selections()[vertex] = get_vertex_selections()[last];
        }

        vertex_positions.pop_back();
        get_vertex_selections().pop_back();
    }
}
