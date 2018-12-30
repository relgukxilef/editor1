#include "drag_vertex.h"

#include <GLFW/glfw3.h>

using namespace glm;

namespace ge1 {

    drag_vertex::drag_vertex(
        context& c, double x, double y
    ) : c(c) {
        auto matrix =
            c.current_view->projection_matrix *
            c.current_view->view_matrix * c.current_object->model_matrix;
        inverse_matrix = inverse(matrix);

        vec2 mouse_ndc = {x / c.width * 2 - 1, 1 - y / c.height * 2};

        selected_vertex = static_cast<unsigned int>(-1);
        unsigned int i = 0;
        for (auto& vertex : c.current_object->mesh->vertex_positions) {
            vec4 ndc = matrix * vec4(vertex, 1);
            ndc /= ndc.w;
            // TODO: respect aspect ratio
            if (length(vec2(ndc.x, ndc.y) - mouse_ndc) < 0.1f) {
                selected_vertex = i;
                ndc_z = ndc.z;
                break;
            }
            i++;
        }
    }

    operation::status drag_vertex::mouse_move_event(double x, double y) {
        if (selected_vertex == static_cast<unsigned int>(-1)) {
            // TODO: would be nice if the constructor could already
            // set the status to finished
            return status::finished;
        }

        vec2 mouse_ndc = {x / c.width * 2 - 1, 1 - y / c.height * 2};

        vec4 t = inverse_matrix * vec4(mouse_ndc, ndc_z, 1);

        c.current_object->mesh->vertex_positions[selected_vertex] =
            vec3(t.x, t.y, t.z) / t.w;

        glBindBuffer(
            GL_COPY_WRITE_BUFFER,
            c.current_object->mesh->vertex_position_buffer.get_name()
        );
        glBufferSubData(
            GL_COPY_WRITE_BUFFER, 0, 4 * 3 * sizeof(float),
            c.current_object->mesh->vertex_positions.data()
        );

        return status::running;
    }

    operation::status drag_vertex::mouse_button_event(
        int button, int action, int modifiers
    ) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            return status::finished;
        } else {
            return status::running;
        }

    }
}
