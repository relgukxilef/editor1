#pragma once

#include <string>
#include <vector>
#include <array>

#include "editor/algorithm/map.h"

namespace ge1 {

    struct mesh_format {
        struct attributes {
            std::string *name = nullptr;

            map array;
        };

        // TODO: support structs
        // one of the arrays is used for instance attributes
        struct scalar_attributes : attributes {
            // TODO: rename to element_size
            unsigned *size = nullptr;
        } float_attributes;

        struct reference_attributes : attributes {
            map target_array;
        } reference_attributes;

        struct copy_dependencies : attributes {
            map reference;
            map attribute;
        } float_copy_attributes;

        // TODO: maybe rename this to arrays
        struct vertex_arrays {
            std::string *name = nullptr;
            unsigned *patch_size = nullptr;
        } vertex_arrays;

        struct mesh {
            std::string name;
            unsigned *arrays_size = nullptr; // in patches
            unsigned *arrays_capacity = nullptr;

            // floats[float_attribute_size][arrays_size[array]]
            float **floats = nullptr;
            float **float_copies = nullptr;

            map *references = nullptr;
        } *meshes = nullptr;

        std::string name;

        unsigned mesh_size = 0, mesh_capacity = 0;
        unsigned array_size = 0, array_capacity = 0;
        unsigned float_attribute_size = 0, float_attribute_capacity = 0;
        unsigned reference_attribute_size = 0, reference_attribute_capacity = 0;
        unsigned float_copy_attribute_size = 0;
        unsigned float_copy_attribute_capacity = 0;

        void set_reference_values(
            unsigned mesh, unsigned attribute,
            unsigned first, const unsigned* values, unsigned count
        );

        void set_float_values(
            unsigned mesh, unsigned attribute,
            unsigned first, const float* values, unsigned vertex_count
        );

        unsigned add_patches(unsigned mesh, unsigned array, unsigned count);

        unsigned remove_patch(unsigned mesh, unsigned array, unsigned patch);

        unsigned add_array(unsigned patch_size);

        unsigned add_float_attribute(unsigned array, unsigned element_size);

        unsigned add_reference_attribute(unsigned array, unsigned target);

        unsigned add_float_copy_attribute(
            unsigned array, unsigned reference_attribute,
            unsigned float_attribute
        );

        unsigned add_mesh();
    };
}
