#pragma once

#include "editor/operation.h"

namespace ge1 {

    struct delete_vertices : public operation {
        status trigger(context& c, double x, double y) override;
    };
}
