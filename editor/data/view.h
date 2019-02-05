#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

namespace ge1 {

    struct view {
        view() = default;

        glm::mat4 view_matrix, projection_matrix;
        GLuint view_properties_buffer;
    };
}
