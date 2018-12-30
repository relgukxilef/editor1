#pragma once

#include "editor/data/context.h"

namespace ge1 {

    struct operation {
        enum struct status {
            running, finished
        };

        operation() = default;
        virtual ~operation() = default;

        virtual status trigger(context &c, double x, double y) = 0;
        virtual status mouse_move_event(context &c, double x, double y);
        virtual status mouse_button_event(
            context &c, int button, int action, int modifiers
        );
        virtual status key_event(
            context &c, int key, int scancode, int modifiers
        );
    };
}
