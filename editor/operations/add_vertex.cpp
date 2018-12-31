#include "add_vertex.h"

using namespace glm;

namespace ge1 {

    operation::status add_vertex::trigger(
        context& c, double x, double y
    ) {
        auto& object = *c.current_object;

        auto matrix =
            c.current_view->projection_matrix *
            c.current_view->view_matrix * object.model_matrix;
        auto inverse_matrix = inverse(matrix);

        vec2 mouse_ndc = {x / c.width * 2 - 1, 1 - y / c.height * 2};

        vec4 center = (matrix * vec4(0, 0, 0, 1));
        float ndc_z = center.z / center.w;

        vec4 v = inverse_matrix * vec4(mouse_ndc, ndc_z, 1);
        v /= v.w;

        object.m->add_vertex(vec3(v));

        // TODO: update call for all objects using this mesh
        object.call.count++;

        return status::finished;
    }
}
