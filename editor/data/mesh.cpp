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

    void mesh::add_edge(std::array<unsigned int, 2> vertex) {
        auto edge_vertex = vertex_edge_vertices.lower_bound({vertex[0], 0});
        while (
            edge_vertex != vertex_edge_vertices.upper_bound({vertex[0] + 1, 0})
        ) {
            auto edge = edge_vertex->second / 2;
            if (
                edge_vertices[edge * 2 + 1 - edge_vertex->second % 2] ==
                vertex[1]
            ) {
                // edge already exists
                return;
            }
            ++edge_vertex;
        }

        for (unsigned int i = 0; i < 2; i++) {
            edge_vertex_positions.push_back(vertex_positions[vertex[i]]);
            vertex_edge_vertices.insert({vertex[i], edge_vertices.size()});
            edge_vertices.push_back(vertex[i]);
        }
    }

    void mesh::add_face(std::array<unsigned int, 3> vertex) {
        // TODO: check whether face already exists
        for (unsigned int i = 0; i < 3; i++) {
            face_vertex_positions.push_back(vertex_positions[vertex[i]]);
            vertex_face_vertices.insert({vertex[i], face_vertices.size()});
            face_vertices.push_back(vertex[i]);

            add_edge({vertex[i], vertex[(i + 1) % 3]});
        }
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

    void mesh::delete_edge(unsigned int edge) {
        for (unsigned int i = 0; i < 2; i++) {
            auto last = edge_vertices.size() - 1;
            auto to = edge * 2 + i;

            mapping_erase_key(vertex_edge_vertices, edge_vertices, to);

            edge_vertex_positions[to] = edge_vertex_positions[last];
            edge_vertex_positions.pop_back();
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

        auto edge_vertex = vertex_edge_vertices.lower_bound({vertex, 0});
        while (
            edge_vertex != vertex_edge_vertices.lower_bound({vertex + 1, 0})
        ) {
            delete_edge(edge_vertex->second / 2);
            edge_vertex = vertex_edge_vertices.lower_bound({vertex, 0});
        }

        unsigned int last = vertex_positions.size() - 1;

        mapping_erase_value(vertex_face_vertices, face_vertices, last, vertex);
        mapping_erase_value(vertex_edge_vertices, edge_vertices, last, vertex);

        vertex_positions[vertex] = vertex_positions[last];
        vertex_positions.pop_back();

        selection_erase(selected_vertices, vertex_selections, vertex);
    }

    void mesh::set_vertex_selection(unsigned int vertex, bool selected) {
        selection_set(selected_vertices, vertex_selections, vertex, selected);
    }

    void mesh::set_vertex_position(unsigned int vertex, vec3 position) {
        vertex_positions[vertex] = position;

        mapping_for_each(vertex_face_vertices, vertex, [=](unsigned int face){
            face_vertex_positions[face] = position;
        });

        mapping_for_each(vertex_edge_vertices, vertex, [=](unsigned int edge){
            edge_vertex_positions[edge] = position;
        });
    }
}
