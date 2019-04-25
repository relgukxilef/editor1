#pragma once

#include <array>

template<class Type, unsigned Element_size>
void scatter(
    std::array<Type, Element_size> value, unsigned head, unsigned size,
    Type* values, unsigned* next
) {
    auto current = head;
    for (unsigned i = 0; i < size; i++) {
        auto element = values + current * Element_size;
        for (unsigned j = 0; j < Element_size; j++) {
            element[j] = value[j];
        }
        current = next[current];
    }
}

template<class Type>
void scatter(
    std::array<Type, 4> value, unsigned element_size,
    unsigned size, unsigned head,
    Type* values, unsigned* next
) {
    if (element_size == 1) {
        scatter<Type, 1>(value, head, size, values, next);
    } else if (element_size == 2) {
        scatter<Type, 2>(value, head, size, values, next);
    } else if (element_size == 3) {
        scatter<Type, 3>(value, head, size, values, next);
    } else if (element_size == 4) {
        scatter<Type, 4>(value, head, size, values, next);
    }
}

inline void set_reference(
    unsigned a, unsigned b,
    unsigned* as_b, unsigned* bs_as_head,
    unsigned* as_next, unsigned* as_previous
) {
    unsigned old_b = as_b[a];
    if (old_b != b) {
        unsigned head = bs_as_head[b];

        // remove existing reference
        unsigned next = as_next[a];
        unsigned previous = as_previous[a];
        if (previous != static_cast<unsigned>(-1)) {
            // a.previous -> a.next
            as_next[previous] = next;
        } else {
            // b -> a.next
            bs_as_head[old_b] = next;
        }
        if (next != static_cast<unsigned>(-1)) {
            // a.previous <- a.next
            as_previous[next] = previous;
        }

        // add new reference
        as_b[a] = b;
        // b -> a <-> head
        as_previous[head] = a;
        as_next[a] = head;
        bs_as_head[b] = a;
    }
}
