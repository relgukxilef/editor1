#pragma once

#include "editor/operation.h"

namespace ge1 {

    struct add_face : public operation {
        status trigger(context& c, double x, double y) override;
    };
}
