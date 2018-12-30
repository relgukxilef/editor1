#pragma once

#include "editor/operation.h"

namespace ge1 {

    struct select_vertex : public operation {
        select_vertex();

        status trigger(context& c, double x, double y) override;
    };
}
