#pragma once

#include <type_traits>
#include <cstdlib>
#include <new>
#include <utility>
#include <cassert>

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

    // TODO: always pass the current size
    template<class Type>
    void resize(Type *&array, unsigned new_size) {
        static_assert(
            std::is_trivially_copyable<Type>::value,
            "'array' must be trivially copyable!"
        );

        void *new_array;
        if (array) {
            new_array = std::realloc(array, new_size * sizeof(Type));
        } else {
            new_array = std::malloc(new_size * sizeof(Type));
        }

        if (new_array) {
            array = reinterpret_cast<Type*>(new_array);
        } else {
            throw std::bad_alloc();
        }
    }

    inline void resize(unsigned, unsigned) {}

    template<class Type, class... Types>
    void resize(
        unsigned size, unsigned new_size, Type *&array, Types *&...arrays
    ) {
        // TODO: use std::aligned_storage
        Type *new_array = new Type[new_size];
        // TODO: null check
        for (auto i = 0u; i < size; i++) {
            new_array[i] = std::move(array[i]);
        }
        delete[] array;
        array = new_array;
        resize(size, new_size, arrays...);
    }
}
