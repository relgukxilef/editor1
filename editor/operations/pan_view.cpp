#include "pan_view.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace ge1;

operation::status pan_view::trigger(context& c, double x, double y) {
    const auto& view_matrix = c.current_view->view_matrix;

    offset =
        vec3(view_matrix[3]) -
        vec3(static_cast<float>(x), -static_cast<float>(y), 0) * 0.002f;

    return status::running;
}

operation::status pan_view::mouse_move_event(context& c, double x, double y) {
    c.current_view->view_matrix[3] = vec4(
        offset +
        vec3(static_cast<float>(x), -static_cast<float>(y), 0) * 0.002f,
        1
    );

    c.current_view->update_view_matrix();

    return status::running;
}

operation::status pan_view::mouse_button_event(context&, int, int action, int) {
    if (action == GLFW_RELEASE) {
        return status::finished;
    } else {
        return status::running;
    }
}
