#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "vertex_array.h"

namespace ge1 {

    struct mesh {
        mesh() = default;

        bool pick_vertex(glm::mat4 matrix, glm::vec2 ndc, unsigned int& vertex);

        std::vector<glm::vec3> vertex_positions;
        std::vector<unsigned short> vertex_selection;

        // don't share buffers between meshes for now
        unique_vertex_array vertex_array;
        unique_buffer vertex_position_buffer;
        unique_buffer vertex_selection_buffer;

        unsigned int vertex_position_capacity = 0;
    };
}
