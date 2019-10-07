#include "rotate_view.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

using namespace glm;

namespace ge1 {

    namespace {
        mat4 turntable_rotate(mat4 m, vec2 rotation, float depth) {
            m[3].z += depth;
            auto transposed = transpose(mat3(m));
            auto center = -transposed * m[3];

            m = rotate(m, rotation.x, {0, 0, 1});
            m = rotate(m, rotation.y, {transpose(m)[0]});

            m[3] = vec4(mat3(m) * center, 1);
            m[3].z -= depth;
            return m;
        }
    }

    operation::status rotate_view::trigger(context& c, double x, double y) {
        const auto& view_matrix = c.current_view->view_matrix;

        origin = turntable_rotate(view_matrix, vec2{x, y} * -0.005f, 5);

        return status::running;
    }

    operation::status rotate_view::mouse_move_event(
        context& c, double x, double y
    ) {
        auto& view_matrix = c.current_view->view_matrix;

        view_matrix = turntable_rotate(origin, vec2{x, y} * 0.005f, 5);

        glBindBuffer(
            GL_COPY_WRITE_BUFFER, c.current_view->view_properties_buffer
        );
        glBufferSubData(
            GL_COPY_WRITE_BUFFER, 0, 16 * sizeof(float),
            value_ptr(
                c.current_view->projection_matrix *
                c.current_view->view_matrix
            )
        );

        return status::running;
    }

    operation::status rotate_view::mouse_button_event(
        context&, int, int action, int
    ) {
        if (action == GLFW_RELEASE) {
            return status::finished;
        } else {
            return status::running;
        }
    }
}
