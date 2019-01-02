#pragma once

#include <initializer_list>

namespace ge1 {

    template<class T>
    struct span {
        span();
        span(const T* begin, const T* end);
        span(std::initializer_list<T> values);

        const T* begin() const;
        const T* end() const;
        bool empty() const;
        unsigned int size() const;

        const T* begin_pointer, * end_pointer;
    };

    template<class T>
    span<T>::span() : span(nullptr, nullptr) {}

    template<class T>
    span<T>::span(const T* begin, const T* end) :
        begin_pointer(begin), end_pointer(end)
    {}

    template<class T>
    span<T>::span(std::initializer_list<T> values) :
        span(values.begin(), values.end())
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

    template<class T>
    unsigned int span<T>::size() const {
        return end_pointer - begin_pointer;
    }

}
