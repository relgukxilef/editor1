#include "mesh.h"

#include "editor/algorithm/selection.h"
#include "editor/algorithm/mapping.h"

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
        vertex_selections.push_back(false);
    }

    void mesh::delete_face(unsigned int face) {
        for (unsigned int i = 0; i < 3; i++) {
            auto last = face_vertices.size() - 1;
            auto to = face * 3 + i;

            mapping_erase_key(vertex_face_vertices, face_vertices, to);

            face_vertex_positions[to] = face_vertex_positions[last];
            face_vertex_positions.pop_back();
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

        mapping_erase_value(vertex_face_vertices, face_vertices, last, vertex);

        vertex_positions[vertex] = vertex_positions[last];
        vertex_positions.pop_back();

        selection_erase(selected_vertices, vertex_selections, vertex);
    }

    void mesh::set_vertex_selection(unsigned int vertex, bool selected) {
        selection_set(selected_vertices, vertex_selections, vertex, selected);
    }
}
