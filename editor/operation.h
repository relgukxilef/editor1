#pragma once

namespace ge1 {

    struct operation {
        enum struct status {
            running, finished
        };

        operation() = default;
        virtual ~operation() = default;

        virtual status mouse_move_event(double x, double y);
        virtual status mouse_button_event(
            int button, int action, int modifiers
        );
        virtual status key_event(int key, int scancode, int modifiers);
    };
}
