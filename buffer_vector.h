#pragma once

#include <memory>

#include <GL/glew.h>

#include "vertex_array.h"

namespace ge1 {

    template<class T>
    struct buffer_vector {
        typedef T* iterator;
        struct reference;

        void push_back(T value);
        void pop_back();
        reference operator[](unsigned int);
        iterator begin();
        iterator end();

        unsigned int size();

        struct reference {
            reference& operator=(T value);
            reference& operator=(const reference& value);
            operator const T&() const;

            unsigned int i;
            buffer_vector& owner;
        };

        std::unique_ptr<T[]> elements;
        unique_buffer buffer;
        unsigned int count = 0;
        unsigned int capacity = 0;
    };

    template<class T>
    void buffer_vector<T>::push_back(T value) {
        glBindBuffer(GL_COPY_WRITE_BUFFER, buffer.get_name());

        if (count == capacity) {
            capacity = std::max(count + 1, capacity * 2);
            std::unique_ptr<T[]> new_elements(new T[capacity]);
            std::copy(
                elements.get(), elements.get() + count, new_elements.get()
            );

            elements = std::move(new_elements);

            elements[count] = value;

            glBufferData(
                GL_COPY_WRITE_BUFFER,
                capacity * sizeof(T),
                elements.get(), GL_DYNAMIC_DRAW
            );

        } else {
            elements[count] = value;

            glBufferSubData(
                GL_COPY_WRITE_BUFFER, count * sizeof(T), sizeof(T), &value
            );
        }

        count++;
    }

    template<class T>
    void buffer_vector<T>::pop_back() {
        count--;
    }

    template<class T>
    typename buffer_vector<T>::reference
    buffer_vector<T>::operator[](unsigned int i) {
        return {i, *this};
    }

    template<class T>
    typename buffer_vector<T>::iterator buffer_vector<T>::begin() {
        return elements.get();
    }

    template<class T>
    typename buffer_vector<T>::iterator buffer_vector<T>::end() {
        return elements.get() + count;
    }

    template<class T>
    unsigned int buffer_vector<T>::size() {
        return count;
    }

    template<class T>
    typename buffer_vector<T>::reference&
    buffer_vector<T>::reference::operator=(T value) {
        *(owner.begin() + i) = value;

        glBindBuffer(GL_COPY_WRITE_BUFFER, owner.buffer.get_name());
        glBufferSubData(
            GL_COPY_WRITE_BUFFER, i * sizeof(T), sizeof(T), &value
        );

        return *this;
    }

    template<class T>
    typename buffer_vector<T>::reference&
    buffer_vector<T>::reference::operator=(
        const buffer_vector::reference& value
    ) {
        return operator=(*(owner.begin() + value.i));
    }

    template<class T>
    buffer_vector<T>::reference::operator const T&() const {
        return *(owner.begin() + i);
    }

}
