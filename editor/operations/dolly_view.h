#pragma once

#include <glm/glm.hpp>

#include "editor/operation.h"
#include "editor/data/context.h"

namespace ge1 {

    struct dolly_view : public operation {
        status trigger(context& c, double x, double y) override;
        status mouse_move_event(context &c, double x, double y) override;
        status mouse_button_event(context &, int, int action, int) override;

        float offset;
    };
}