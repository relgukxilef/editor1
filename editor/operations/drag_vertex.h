#pragma once

#include "editor/operation.h"
#include "editor/data/context.h"

namespace ge1 {

    struct drag_vertex : public operation {
        drag_vertex();

        status trigger(context& c, double x, double y) override;
        status mouse_move_event(context &c, double x, double y) override;
        status mouse_button_event(
            context &c, int button, int action, int modifiers
        ) override;

        unsigned int selected_vertex;
        glm::mat4 inverse_matrix;
        float ndc_z;
    };

    inline drag_vertex::drag_vertex() {}
}
