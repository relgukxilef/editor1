#include "rotate_view.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

using namespace glm;

namespace ge1 {

    operation::status rotate_view::trigger(context& c, double x, double y) {

        vec2 mouse_ndc = {x / c.width * 2 - 1, 1 - y / c.height * 2};

        previous_mouse = {x, y};

        return status::running;
    }

    operation::status rotate_view::mouse_move_event(
        context& c, double x, double y
    ) {
        auto delta = (vec2(x, y) - previous_mouse) * 0.005f;
        previous_mouse = {x, y};

        auto& view_matrix = c.current_view->view_matrix;

        view_matrix = rotate(
            view_matrix, delta.x, {0, 0, 1}
        );
        view_matrix = rotate(
            view_matrix, delta.y,
            {transpose(view_matrix) * vec4(1, 0, 0, 0)}
        );

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
