#include "select_vertex.h"

#include "editor/algorithm/selection.h"

using namespace glm;

namespace ge1 {

    select_vertex::select_vertex() {}

    operation::status select_vertex::trigger(context& c, double x, double y) {
        auto matrix =
            c.current_view->projection_matrix *
            c.current_view->view_matrix * c.current_object->model_matrix;

        vec2 mouse_ndc = {x / c.width * 2 - 1, 1 - y / c.height * 2};
        unsigned int selected_vertex;

        auto& m = c.current_object->m;

        if (m->pick_vertex(matrix, mouse_ndc, selected_vertex)) {
            auto vertex_selection = m->get_vertex_selections();
            bool selection = !vertex_selection[selected_vertex];
            vertex_selection[selected_vertex] = selection;
        }

        return status::finished;
    }

}
