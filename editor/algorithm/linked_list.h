#pragma once

#include <cassert>

namespace ge1 {

    struct linked_list {
        unsigned first, size;
        unsigned* next, * previous;

        struct iterator {
            unsigned index, size;
            unsigned current;
            const unsigned* next;

            iterator operator++() {
                assert(index < size);
                index++;
                current = next[current];
                return *this;
            }
            bool operator!=(const iterator& other) {
                return index != other.index;
            }
            unsigned operator*() {
                assert(current != static_cast<unsigned>(-1));
                return current;
            }
        };

        iterator begin() const {
            return {0, size, first, next};
        }
        iterator end() const {
            return {size, size, first, next};
        }
    };
}
