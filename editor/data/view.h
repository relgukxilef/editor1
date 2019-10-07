#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

namespace ge1 {

    struct view {
        view() = default;

        glm::mat4 view_matrix, projection_matrix;
        GLuint view_properties_buffer;

        inline void update_view_matrix();
    };

    void view::update_view_matrix() {
        glBindBuffer(
            GL_COPY_WRITE_BUFFER, view_properties_buffer
        );
        glBufferSubData(
            GL_COPY_WRITE_BUFFER, 0, 16 * sizeof(float),
            value_ptr(
                projection_matrix *
                view_matrix
            )
        );
    }
}
