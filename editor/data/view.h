#pragma once

#include <glm/glm.hpp>

namespace ge1 {

    struct view {
        view() = default;

        glm::mat4 view_matrix, projection_matrix;
    };
}
