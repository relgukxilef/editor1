#pragma once

#include <type_traits>
#include <cstdlib>
#include <new>

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
}
