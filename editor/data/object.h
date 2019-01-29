#pragma once

#include <glm/glm.hpp>

#include "mesh.h"
#include "draw_call.h"

namespace ge1 {

    struct object {
        object(
            mesh* m, GLuint face_program, GLuint edge_program,
            GLuint vertex_program
        );

        glm::mat4 model_matrix;
        mesh* m;

        // no multi draw for now
        draw_call face_call;
        draw_call edge_call;
        draw_call vertex_call;
    };
}
