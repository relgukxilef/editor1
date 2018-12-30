#pragma once

#include <initializer_list>

namespace ge1 {

    template<class T>
    struct span {
        span(const T* begin, const T* end);
        span(std::initializer_list<T> values);

        const T* begin() const;
        const T* end() const;
        bool empty() const;

        const T* begin_pointer, * end_pointer;
    };

    template<class T>
    span<T>::span(const T* begin, const T* end) :
        begin_pointer(begin), end_pointer(end)
    {}

    template<class T>
    span<T>::span(std::initializer_list<T> values) :
        begin_pointer(values.begin()), end_pointer(values.end())
    {}

    template<class T>
    const T* span<T>::begin() const {
        return begin_pointer;
    }

    template<class T>
    const T* span<T>::end() const {
        return end_pointer;
    }

    template<class T>
    bool span<T>::empty() const {
        return begin() == end();
    }

}
