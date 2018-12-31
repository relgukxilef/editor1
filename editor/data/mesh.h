#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <glm/glm.hpp>

#include "vertex_array.h"
#include "editor/algorithm/selection.h"

namespace ge1 {

    struct mesh {
        mesh() = default;

        bool pick_vertex(glm::mat4 matrix, glm::vec2 ndc, unsigned int& vertex);

        void resize_vertex_buffer(unsigned int size);
        void add_vertex(glm::vec3 position);

        selection_vector<
            std::vector<unsigned char>, std::unordered_set<unsigned int>
        > get_vertex_selections();
        selection_set<
            std::vector<unsigned char>, std::unordered_set<unsigned int>
        > get_selected_vertices();

        std::vector<glm::vec3> vertex_positions;
        std::vector<unsigned char> vertex_selections;

        std::vector<glm::vec3> face_vertex_positions;

        std::unordered_multimap<unsigned int, unsigned int>
            vertex_face_vertices;
        std::unordered_set<unsigned int> selected_vertices;

        // don't share buffers between meshes for now
        unique_vertex_array vertex_array;
        unique_buffer vertex_position_buffer;
        unique_buffer vertex_selection_buffer;

        unique_vertex_array face_vertex_array;
        unique_buffer face_vertex_position_buffer;

        unsigned int vertex_count = 0;
        unsigned int vertex_capacity = 0;

        unsigned int face_count = 0;
        unsigned int face_capacity = 0;
    };

    inline selection_vector<
        std::vector<unsigned char>,
        std::unordered_set<unsigned int>
    > mesh::get_vertex_selections() {
        return make_selection_vector(vertex_selections, selected_vertices);
    }

    inline selection_set<
        std::vector<unsigned char>, std::unordered_set<unsigned int>
    > mesh::get_selected_vertices() {
        return make_selection_set(vertex_selections, selected_vertices);
    }
}
