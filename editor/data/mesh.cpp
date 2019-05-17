#include "mesh.h"

#include <algorithm>

#include "editor/algorithm/array.h"
#include "span.h"

namespace ge1 {

    void mesh_format::set_reference_values(
        unsigned mesh, unsigned attribute,
        unsigned first, const unsigned *values, unsigned count
    ) {
        assert(meshes);
        auto m = meshes[mesh];
        assert(m.references);
        auto reference = m.references[attribute];
        auto array = reference_attributes.array.value[attribute];
        assert(
            m.arrays_size[array] * vertex_arrays.patch_size[array] >=
            first + count
        );

        auto vertex = first;
        for (auto value : span<const unsigned>(values, values + count)) {
            reference.set(vertex, value);
            vertex++;
        }

        // for each copy attribute
        for (
            auto float_copy_attribute :
            float_copy_attributes.reference.keys(attribute)
        ) {
            auto float_attribute =
                float_copy_attributes.attribute.value[float_copy_attribute];
            auto vertex = first;
            for (auto value : span<const unsigned>(values, values + count)) {
                m.float_copies[float_copy_attribute][vertex] =
                    m.floats[float_attribute][value];
                vertex++;
            }
        }
    }

    void mesh_format::set_float_values(
        unsigned mesh, unsigned attribute,
        unsigned first, const float *values, unsigned vertex_count
    ) {
        // first is a vertex index
        // values must be array of vertex_count * size

        assert(meshes);
        auto m = meshes[mesh];
        assert(m.floats);
        const auto floats = m.floats[attribute];
        assert(floats);
        auto size = float_attributes.size[attribute];
        auto array = float_attributes.array.value[attribute];
        assert(
            m.arrays_size[array] * vertex_arrays.patch_size[array] >=
            first + vertex_count
        );

        auto values_end = values + vertex_count * size;
        auto floats_begin = floats + first * size;
        std::move(values, values_end, floats_begin);

        // for each dependent copy attribute
        for (
            auto float_copy_attribute :
            float_copy_attributes.attribute.keys(attribute)
        ) {
            const auto reference_attribute =
                float_copy_attributes.reference.value[float_copy_attribute];
            const auto copies = m.float_copies[float_copy_attribute];
            for (
                auto vertex = first;
                vertex < first + vertex_count;
                vertex++
            ) {
                // determine dependent vertices
                // TODO: use scatter
                for (
                    auto copy_vertex :
                    meshes[mesh].references[reference_attribute].keys(vertex)
                ) {
                    for (auto i = 0u; i < size; i++) {
                        copies[copy_vertex * size + i] =
                            values[vertex * size + i];
                    }
                }
            }
        }
    }

    unsigned mesh_format::add_patches(
        unsigned mesh, unsigned array, unsigned count
    ) {
        auto m = meshes[mesh];

        auto size = m.arrays_size[array];
        m.arrays_size[array] = size + count;
        auto capacity = m.arrays_capacity[array];
        auto new_capacity = push_back_capacity(size, capacity, count);
        m.arrays_capacity[array] = new_capacity;

        auto patch_size = vertex_arrays.patch_size[array];

        for (auto attribute : float_attributes.array.keys(array)) {
            auto attribute_size = float_attributes.size[attribute];
            push_back(
                m.floats[attribute],
                size * patch_size * attribute_size,
                capacity * patch_size * attribute_size,
                new_capacity * patch_size * attribute_size,
                count * patch_size * attribute_size,
                0
            );
        }

        for (auto attribute : float_copy_attributes.array.keys(array)) {
            auto float_attribute =
                float_copy_attributes.attribute.value[attribute];
            auto attribute_size = float_attributes.size[float_attribute];
            push_back(
                m.float_copies[attribute],
                size * patch_size * attribute_size,
                capacity * patch_size * attribute_size,
                new_capacity * patch_size * attribute_size,
                count * patch_size * attribute_size,
                0
            );
        }

        for (auto attribute : reference_attributes.array.keys(array)) {
            m.references[attribute].push_back(
                size * patch_size,
                capacity * patch_size,
                new_capacity * patch_size,
                count * patch_size
            );
        }

        for (
            auto attribute : reference_attributes.target_array.keys(array)
        ) {
            m.references[attribute].value_push_back(
                size * patch_size,
                capacity * patch_size,
                new_capacity * patch_size,
                count * patch_size
            );
        }

        return size;
    }

    unsigned mesh_format::remove_patch(
        unsigned mesh, unsigned array, unsigned patch
    ) {
        auto m = meshes[mesh];

        m.arrays_size[array]--;
        auto last = m.arrays_size[array];
        // TODO: remove dependant patches

        // scalars
        for (auto attribute : float_attributes.array.keys(array)) {
            m.floats[attribute][patch] = m.floats[attribute][last];
        }

        // references
        for (
            auto reference_attribute : reference_attributes.array.keys(array)
        ) {
            // move last to patch
            auto &map = m.references[reference_attribute];
            map.set(
                patch, map.value[last]
            );
            map.pop_back(last);
        }

        // update references to last
        for (
            auto reference_attribute :
            reference_attributes.target_array.keys(array)
        ) {
            for (
                auto reference_patch :
                m.references[reference_attribute].keys(last)
            ) {
                m.references[reference_attribute].set(reference_patch, patch);
            }
        }

        return last;
    }

    unsigned mesh_format::add_array(unsigned patch_size) {
        auto size = array_size;
        array_size++;
        auto capacity = array_capacity;
        auto new_capacity = push_back_capacity(size, capacity, 1);
        array_capacity = new_capacity;

        push_back(
            vertex_arrays.name, size, capacity, new_capacity, 1, "new array"
        );
        push_back(
            vertex_arrays.patch_size, size, capacity, new_capacity,
            1, patch_size
        );

        float_attributes.array.value_push_back(size, capacity, new_capacity, 1);
        reference_attributes.array.value_push_back(
            size, capacity, new_capacity, 1
        );
        reference_attributes.target_array.value_push_back(
            size, capacity, new_capacity, 1
        );
        float_copy_attributes.array.value_push_back(
            size, capacity, new_capacity, 1
        );

        for (auto i = 0u; i < mesh_size; i++) {
            push_back(
                meshes[i].arrays_size, size, capacity, new_capacity, 1, 0
            );
            push_back(
                meshes[i].arrays_capacity, size, capacity, new_capacity, 1, 0
            );
        }

        return size;
    }

    unsigned mesh_format::add_float_attribute(
        unsigned array, unsigned element_size
    ) {
        assert(array < array_size);

        auto size = float_attribute_size;
        float_attribute_size++;
        auto capacity = float_attribute_capacity;
        auto new_capacity = push_back_capacity(size, capacity, 1);
        float_attribute_capacity = new_capacity;

        push_back(
            float_attributes.name, size, capacity, new_capacity,
            1, "new float attribute"
        );
        push_back(
            float_attributes.size, size, capacity, new_capacity,
            1, element_size
        );

        float_attributes.array.push_back(
            size, capacity, new_capacity, 1, array
        );

        float_copy_attributes.attribute.value_push_back(
            size, capacity, new_capacity, 1
        );

        for (auto i = 0u; i < mesh_size; i++) {
            auto &m = meshes[i];

            float *new_floats;
            push_back(
                new_floats, 0, 0, m.arrays_capacity[array],
                m.arrays_size[array], 0
            );
            push_back(m.floats, size, capacity, new_capacity, 1, new_floats);
        }

        return size;
    }

    unsigned mesh_format::add_reference_attribute(
        unsigned array, unsigned target
    ) {
        assert(array < array_size);
        assert(target < array_size);

        auto size = reference_attribute_size;
        reference_attribute_size++;
        auto capacity = reference_attribute_capacity;
        auto new_capacity = push_back_capacity(size, capacity, 1);
        reference_attribute_capacity = new_capacity;

        push_back(
            reference_attributes.name, size, capacity, new_capacity,
            1, "new reference attribute"
        );

        reference_attributes.array.push_back(
            size, capacity, new_capacity, 1, array
        );

        reference_attributes.target_array.push_back(
            size, capacity, new_capacity, 1, target
        );

        float_copy_attributes.reference.value_push_back(
            size, capacity, new_capacity, 1
        );

        for (auto i = 0u; i < mesh_size; i++) {
            map new_references;
            new_references.push_back(
                0, 0, meshes[i].arrays_capacity[array],
                meshes[i].arrays_size[array]
            );
            new_references.value_push_back(
                0, 0, meshes[i].arrays_capacity[target],
                meshes[i].arrays_size[target]
            );
            push_back(
                meshes[i].references, size, capacity, new_capacity,
                1, new_references
            );
        }

        return size;
    }

    unsigned mesh_format::add_float_copy_attribute(
        unsigned array, unsigned reference_attribute, unsigned float_attribute
    ) {
        assert(array < array_size);
        assert(reference_attribute < reference_attribute_size);
        assert(float_attribute < float_attribute_size);
        assert(
            float_attributes.array.value[float_attribute] ==
            reference_attributes.target_array.value[reference_attribute]
        );
        assert(
            array ==
            reference_attributes.array.value[reference_attribute]
        );

        auto size = float_copy_attribute_size;
        float_copy_attribute_size++;
        auto capacity = float_copy_attribute_capacity;
        auto new_capacity = push_back_capacity(size, capacity, 1);
        float_copy_attribute_capacity = new_capacity;

        push_back(
            float_copy_attributes.name, size, capacity, new_capacity,
            1, "new float copy attirbute"
        );

        float_copy_attributes.array.push_back(
            size, capacity, new_capacity, 1, array
        );
        float_copy_attributes.reference.push_back(
            size, capacity, new_capacity, 1, reference_attribute
        );
        float_copy_attributes.attribute.push_back(
            size, capacity, new_capacity, 1, float_attribute
        );

        for (auto i = 0u; i < mesh_size; i++) {
            float* new_copies;
            push_back(
                new_copies, 0, 0, meshes[i].arrays_capacity[array],
                meshes[i].arrays_size[array], 0
            );
            push_back(
                meshes[i].float_copies, size, capacity, new_capacity,
                1, new_copies
            );
        }

        return size;
    }

    unsigned mesh_format::add_mesh() {
        auto size = mesh_size;
        mesh_size++;
        auto capacity = mesh_capacity;
        auto new_capacity = push_back_capacity(size, capacity, 1);
        mesh_capacity = new_capacity;

        auto new_mesh = mesh();

        new_mesh.name = "new mesh";
        push_back(new_mesh.arrays_size, 0, 0, array_capacity, array_size, 0);
        push_back(
            new_mesh.arrays_capacity, 0, 0, array_capacity, array_size, 0
        );
        push_back(
            new_mesh.floats, 0, 0, float_attribute_capacity,
            float_attribute_size, nullptr
        );
        push_back(
            new_mesh.references, 0, 0, reference_attribute_capacity,
            reference_attribute_size, map()
        );
        push_back(
            new_mesh.float_copies, 0, 0, float_copy_attribute_capacity,
            float_copy_attribute_size, nullptr
        );

        push_back(meshes, size, capacity, new_capacity, 1, new_mesh);

        return size;
    }
}
