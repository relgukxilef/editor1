#include "mesh.h"

#include <cassert>
#include <cstdlib>

namespace ge1 {

    template<class Type>
    void resize(Type *&array, unsigned new_size) {
        static_assert(
            std::is_trivially_copyable<Type>::value,
            "'array' must be trivially copyable!"
        );

        void *new_array = std::realloc(array, new_size * sizeof(Type));
        if (new_array) {
            array = reinterpret_cast<Type*>(new_array);
        } else {
            throw std::bad_alloc();
        }
    }

    template<class Type>
    void resize(Type *&array, unsigned size, unsigned new_size) {
        // TODO: use std::aligned_storage
        Type *new_array = new Type[new_size];
        for (auto i = 0u; i < size; i++) {
            new_array[i] = std::move(array[i]);
        }
        delete[] array;
        array = new_array;
    }

    void mesh_format::set_reference_values(
        unsigned mesh, unsigned attribute,
        unsigned first, fast::span<const unsigned> values
    ) {
        auto reference = meshes[mesh].references[attribute];
        for (auto value : values) {
            reference.set(first, value);
            // TODO: update copies
            first++;
        }
    }

    void mesh_format::set_float_values(
        unsigned mesh, unsigned attribute,
        unsigned first, fast::span<const float> values
    ) {
        auto m = meshes[mesh];
        const auto floats = m.floats[attribute];

        auto vertex = first;
        for (auto value : values) {
            floats[vertex] = value;
            vertex++;
        }

        // for each dependent copy attribute
        for (
            auto copy_attribute :
            float_copy_dependencies.reference.keys(attribute)
        ) {
            const auto reference_attribute =
                float_copy_dependencies.reference.value[copy_attribute];
            const auto copies = m.float_copies[copy_attribute];
            vertex = first;
            for (auto value : values) {
                // determine dependent vertices
                for (
                    auto copy_vertex :
                    meshes[mesh].references[reference_attribute].keys(vertex)
                ) {
                    copies[copy_vertex] = value;
                }
                vertex++;
            }
        }
    }

    unsigned mesh_format::add_patches(
        unsigned mesh, unsigned array,
        unsigned** references, unsigned count
    ) {
        auto m = meshes[mesh];

        auto size = m.arrays_size[array];
        auto capacity = m.arrays_capacity[array];
        auto patch_size = vertex_arrays.patch_size[array];
        if (size + count * patch_size > capacity) {
            capacity = std::max(capacity * 2, capacity + count * patch_size);

            for (auto attribute : float_attributes.array.keys(array)) {
                resize(m.floats[attribute], capacity);
            }

            for (auto attribute : reference_attributes.array.keys(array)) {
                resize(m.references[attribute].value, capacity);
                resize(m.references[attribute].next, capacity);
                resize(m.references[attribute].previous, capacity);
            }

            for (
                auto attribute : reference_attributes.target_array.keys(array)
            ) {
                resize(m.references[attribute].size, capacity);
                resize(m.references[attribute].first, capacity);
            }

            m.arrays_capacity[array] = capacity;
        }

        for (auto attribute : float_attributes.array.keys(array)) {
            auto f = m.floats[attribute];
            for (auto i = 0u; i < count * patch_size; i++) {
                f[size + i] = 0;
            }
        }

        for (
            auto reference_attribute : reference_attributes.array.keys(array)
        ) {
            auto reference = m.references[reference_attribute];
            for (auto i = 0u; i < count * patch_size; i++) {
                reference.push_back(size + i, (*references)[i]);
                // TODO: update copies
            }
            references++;
        }

        for (auto attribute : reference_attributes.target_array.keys(array)) {
            for (auto i = 0u; i < count * patch_size; i++) {
                m.references[attribute].value_push_back(size + i);
            }
        }

        m.arrays_size[array] = size + count * patch_size;
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
        if (array_size == array_capacity) {
            array_capacity = std::max(array_capacity * 2, 1u);
            resize(vertex_arrays.name, array_size, array_capacity);
            resize(vertex_arrays.patch_size, array_capacity);

            resize(float_attributes.array.size, array_capacity);
            resize(float_attributes.array.first, array_capacity);
            resize(reference_attributes.array.size, array_capacity);
            resize(reference_attributes.array.first, array_capacity);
            resize(reference_attributes.target_array.size, array_capacity);
            resize(reference_attributes.target_array.first, array_capacity);

            for (auto i = 0u; i < mesh_size; i++) {
                resize(meshes[i].arrays_size, array_capacity);
                resize(meshes[i].arrays_capacity, array_capacity);
            }
        }

        vertex_arrays.name[array_size] = "new array";
        vertex_arrays.patch_size[array_size] = patch_size;

        float_attributes.array.value_push_back(array_size);
        reference_attributes.array.value_push_back(array_size);
        reference_attributes.target_array.value_push_back(array_size);

        // TODO: initialization at resizing might be cheaper
        for (auto i = 0u; i < mesh_size; i++) {
            meshes[i].arrays_size[array_size] = 0;
            meshes[i].arrays_capacity[array_size] = 0;
        }

        return array_size++;
    }

    unsigned mesh_format::add_float_attribute(unsigned array, unsigned size) {
        if (float_attribute_size == float_attribute_capacity) {
            float_attribute_capacity =
                std::max(float_attribute_capacity * 2, 1u);
            resize(
                float_attributes.name,
                float_attribute_size, float_attribute_capacity
            );
            resize(float_attributes.size, float_attribute_capacity);

            resize(float_attributes.array.value, float_attribute_capacity);
            resize(float_attributes.array.next, float_attribute_capacity);
            resize(float_attributes.array.previous, float_attribute_capacity);

            resize(
                float_copy_dependencies.reference.size, float_attribute_capacity
            );
            resize(
                float_copy_dependencies.reference.first,
                float_attribute_capacity
            );

            for (auto i = 0u; i < mesh_size; i++) {
                resize(meshes[i].floats, float_attribute_capacity);
            }
        }

        float_attributes.name[float_attribute_size] = "new float attribute";
        float_attributes.size[float_attribute_size] = size;

        float_attributes.array.push_back(float_attribute_size, array);

        float_copy_dependencies.reference.value_push_back(float_attribute_size);

        for (auto i = 0u; i < mesh_size; i++) {
            meshes[i].floats[float_attribute_size] = nullptr;
        }

        return float_attribute_size++;
    }

    unsigned mesh_format::add_reference_attribute(
        unsigned array, unsigned target
    ) {
        if (reference_attribute_size == reference_attribute_capacity) {
            reference_attribute_capacity =
                std::max(reference_attribute_capacity * 2, 1u);
            resize(
                reference_attributes.name,
                reference_attribute_size, reference_attribute_capacity
            );

            resize(
                reference_attributes.array.value, reference_attribute_capacity
            );
            resize(
                reference_attributes.array.next, reference_attribute_capacity
            );
            resize(
                reference_attributes.array.previous,
                reference_attribute_capacity
            );

            resize(
                reference_attributes.target_array.value,
                reference_attribute_capacity
            );
            resize(
                reference_attributes.target_array.next,
                reference_attribute_capacity
            );
            resize(
                reference_attributes.target_array.previous,
                reference_attribute_capacity
            );

            for (auto i = 0u; i < mesh_size; i++) {
                resize(meshes[i].references, reference_attribute_capacity);
            }
        }

        reference_attributes.name[reference_attribute_size] =
            "new reference attribute";
        reference_attributes.array.push_back(reference_attribute_size, array);
        reference_attributes.target_array.push_back(
            reference_attribute_size, target
        );

        for (auto i = 0u; i < mesh_size; i++) {
            meshes[i].references[reference_attribute_size] = map();
        }

        return reference_attribute_size++;
    }

    unsigned mesh_format::add_float_copy_attribute(
        unsigned array, unsigned reference_attribute, unsigned float_attribute
    ) {
        // TODO
    }

    unsigned mesh_format::add_mesh() {
        if (mesh_size == mesh_capacity) {
            mesh_capacity = std::max(mesh_capacity * 2, 1u);
            resize(meshes, mesh_capacity);
        }

        auto &m = meshes[mesh_size];
        m = mesh();

        resize(m.arrays_size, array_capacity);
        resize(m.arrays_capacity, array_capacity);
        for (auto i = 0u; i < array_size; i++) {
            m.arrays_size[i] = 0;
            m.arrays_capacity[i] = 0;
        }

        resize(m.floats, float_attribute_capacity);
        for (auto i = 0u; i < float_attribute_size; i++) {
            m.floats[i] = nullptr;
        }

        resize(m.references, reference_attribute_capacity);
        for (auto i = 0u; i < reference_attribute_size; i++) {
            m.references[i] = map();
        }

        return mesh_size++;
    }
}
