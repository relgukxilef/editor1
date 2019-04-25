#pragma once

#include "view.h"
#include "object.h"

#include <memory>

namespace ge1 {

    struct context {
        context() = default;

        view* current_view = nullptr;
        object* current_object = nullptr;
        unsigned int width, height;


    };
}
