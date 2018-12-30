#pragma once

#include <glm/glm.hpp>

#include "mesh.h"

namespace ge1 {

    struct object {
        object() = default;

        glm::mat4 model_matrix;
        mesh* mesh;
    };
}
