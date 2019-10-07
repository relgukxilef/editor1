#include "dolly_view.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace ge1;


operation::status dolly_view::trigger(context& c, double, double y) {
    offset = c.current_view->view_matrix[3].z + static_cast<float>(y) * 0.002f;

    return status::running;
}

operation::status dolly_view::mouse_move_event(context& c, double, double y) {
    c.current_view->view_matrix[3].z = offset - static_cast<float>(y) * 0.002f;

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

operation::status dolly_view::mouse_button_event(
    context&, int, int action, int
) {
    if (action == GLFW_RELEASE) {
        return status::finished;
    } else {
        return status::running;
    }
}
