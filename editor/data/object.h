#pragma once

#include <glm/glm.hpp>

#include "mesh.h"
#include "draw_call.h"

namespace ge1 {

    struct object {
        object(
            GLuint face_program, GLuint edge_program,
            GLuint vertex_program
        );

        glm::mat4 model_matrix;
        //mesh* m;

        // no multi draw for now
        //draw_arrays_call face_call;
        //draw_arrays_call edge_call;
        //draw_arrays_call vertex_call;
    };
}
