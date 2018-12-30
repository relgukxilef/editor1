#include "operation.h"

namespace ge1 {

    operation::status operation::mouse_move_event(double, double) {
        return status::running;
    }

    operation::status operation::mouse_button_event(int, int, int) {
        return status::running;
    }

    operation::status operation::key_event(int, int, int) {
        return status::running;
    }
}
