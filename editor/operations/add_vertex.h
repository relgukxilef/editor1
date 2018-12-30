#pragma once

#include "editor/operation.h"

namespace ge1 {

    struct add_vertex : public operation {
        add_vertex();

        status trigger(context& c, double x, double y) override;
    };

    inline add_vertex::add_vertex() {}
}
