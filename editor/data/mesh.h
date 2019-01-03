#pragma once

#include <vector>
#include <set>
#include <unordered_set>

#include <glm/glm.hpp>

#include "vertex_array.h"
#include "editor/algorithm/selection.h"
#include "buffer_vector.h"

namespace ge1 {

    struct mesh {
        mesh() = default;

        bool pick_vertex(glm::mat4 matrix, glm::vec2 ndc, unsigned int& vertex);

        void resize_vertex_buffer(unsigned int size);
        void add_vertex(glm::vec3 position);

        void delete_face(unsigned int face);

        selection_vector<
            buffer_vector<unsigned char>, std::unordered_set<unsigned int>
        > get_vertex_selections();
        selection_set<
            buffer_vector<unsigned char>, std::unordered_set<unsigned int>
        > get_selected_vertices();

        buffer_vector<glm::vec3> vertex_positions;
        buffer_vector<unsigned char> vertex_selections;

        buffer_vector<glm::vec3> face_vertex_positions;
        std::vector<unsigned int> face_vertices;

        std::set<std::pair<unsigned int, unsigned int>> vertex_face_vertices;
        std::unordered_set<unsigned int> selected_vertices;

        // don't share buffers between meshes for now
        unique_vertex_array vertex_array;

        unique_vertex_array face_vertex_array;
    };

    inline selection_vector<
        buffer_vector<unsigned char>,
        std::unordered_set<unsigned int>
    > mesh::get_vertex_selections() {
        return make_selection_vector(vertex_selections, selected_vertices);
    }

    inline selection_set<
        buffer_vector<unsigned char>, std::unordered_set<unsigned int>
    > mesh::get_selected_vertices() {
        return make_selection_set(vertex_selections, selected_vertices);
    }
}
