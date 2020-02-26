#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "ge1/span.h"
#include "ge1/draw_call.h"

namespace ge1 {

    struct obj_file {
        obj_file(const char* filename);

        // TODO: don't store draw_arrays_indirect_command here
        std::vector<draw_arrays_indirect_command> material_faces;
        std::vector<glm::vec3> material_diffuse_color;

        std::vector<glm::vec3> face_positions, face_normals;
        std::vector<glm::vec2> face_texture_coordinates;

        // shapes are ignored for now
    };

}
