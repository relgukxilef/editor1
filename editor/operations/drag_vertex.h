#pragma once

#include "editor/operation.h"
#include "editor/data/context.h"

namespace ge1 {

    struct drag_vertex : public operation {
        drag_vertex(context& context, double x, double y);

        status mouse_move_event(double x, double y) override;
        status mouse_button_event(
            int button, int action, int modifiers
        ) override;

        unsigned int selected_vertex;
        context& c;
        glm::mat4 inverse_matrix;
        float ndc_z;
    };
}
