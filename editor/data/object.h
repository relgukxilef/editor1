#pragma once

#include <glm/glm.hpp>

#include "mesh.h"
#include "draw_call.h"

namespace ge1 {

    struct object {
        object(mesh* m, GLuint program);

        glm::mat4 model_matrix;
        mesh* m;

        // no multi draw for now
        draw_call call;
    };
}
