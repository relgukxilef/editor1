#pragma once

#include <type_traits>
#include <cstdlib>
#include <new>
#include <utility>
#include <cassert>
#include <algorithm>

namespace ge1 {

    template<class Type>
    void resize(Type *&array, unsigned size, unsigned new_size) {
        // TODO: use std::aligned_storage
        assert(size < new_size);
        Type *new_array = new Type[new_size];
        if (size > 0) {
            for (auto i = 0u; i < size; i++) {
                new_array[i] = std::move(array[i]);
            }
            assert(array);
            delete[] array;
        } else {
            assert(!array);
        }
        array = new_array;
    }

    inline void resize(unsigned, unsigned) {}

    template<class Type, class... Types>
    void resize(
        unsigned size, unsigned new_size, Type *&array, Types *&...arrays
    ) {
        resize(array, size, new_size);
        resize(size, new_size, arrays...);
    }

    inline unsigned push_back_capacity(
        unsigned size, unsigned capacity, unsigned count
    ) {
        assert(size <= capacity);
        if (size + count > capacity) {
            return std::max(capacity * 2, capacity + count);
        } else {
            return capacity;
        }
    }

    template<class Type, class Value>
    void push_back(
        Type *&array,
        unsigned size, unsigned old_capacity, unsigned new_capacity,
        unsigned count, Value value
    ) {
        if (old_capacity != new_capacity) {
            resize(array, old_capacity, new_capacity);
        }

        for (auto i = 0u; i < count; i++) {
            array[size + i] = value;
        }
    }
}
