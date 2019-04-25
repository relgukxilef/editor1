#include "drag_vertex.h"

#include <GLFW/glfw3.h>

using namespace glm;

namespace ge1 {

    operation::status drag_vertex::trigger(context& c, double x, double y) {
        if (!c.current_object) {
            return status::finished;
        }

        auto matrix =
            c.current_view->projection_matrix *
            c.current_view->view_matrix * c.current_object->model_matrix;
        inverse_matrix = inverse(matrix);

        vec2 mouse_ndc = {x / c.width * 2 - 1, 1 - y / c.height * 2};

        /*if (
            c.current_object->m->pick_vertex(matrix, mouse_ndc, vertex)
        ) {
            // TODO: calculation is redundant in pick_vertex
            vec4 vertex_ndc = matrix * vec4(
                vec3(c.current_object->m->vertex_positions[vertex]), 1
            );
            ndc_z = vertex_ndc.z / vertex_ndc.w;
            return status::running;

        } else {
            return status::finished;
        }*/
        return status::finished;
    }

    operation::status drag_vertex::mouse_move_event(
        context& c, double x, double y
    ) {
        /*auto& m = c.current_object->m;

        vec2 mouse_ndc = {x / c.width * 2 - 1, 1 - y / c.height * 2};

        vec4 t = inverse_matrix * vec4(mouse_ndc, ndc_z, 1);
        t /= t.w;

        m->set_vertex_position(vertex, t);*/

        return status::running;
    }

    operation::status drag_vertex::mouse_button_event(
        context&, int button, int action, int
    ) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            return status::finished;
        } else {
            return status::running;
        }

    }
}
