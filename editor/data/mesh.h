#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "vertex_array.h"

namespace ge1 {

    struct mesh {
        mesh() = default;

        std::vector<glm::vec3> vertex_positions;

        // don't share buffers between meshes for now
        unique_vertex_array vertex_array;
        unique_buffer vertex_position_buffer;
    };
}
