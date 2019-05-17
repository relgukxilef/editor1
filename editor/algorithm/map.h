#pragma once

#include <cassert>

#include "linked_list.h"
#include "editor/algorithm/array.h"

namespace ge1 {
    struct map {
        unsigned *value = nullptr, *next = nullptr, *previous = nullptr;
        unsigned *size = nullptr, *first = nullptr;

        inline linked_list keys(unsigned value) const {
            return {first[value], size[value], next, previous};
        }

        inline void set(unsigned key, unsigned value) {
            // set this->value[key] to value
            unsigned old_value = this->value[key];
            if (old_value != value) {
                // remove existing reference
                if (old_value != static_cast<unsigned>(-1)) {
                    unsigned next = this->next[key];
                    unsigned previous = this->previous[key];
                    if (previous != static_cast<unsigned>(-1)) {
                        // index.previous -> index.next
                        this->next[previous] = next;
                    } else {
                        // list -> index.next
                        this->first[old_value] = next;
                    }
                    if (next != static_cast<unsigned>(-1)) {
                        // index.previous <- index.next
                        this->previous[next] = previous;
                    }
                    assert(this->size[old_value] > 0);
                    this->size[old_value]--;
                }

                // add new reference
                if (value != static_cast<unsigned>(-1)) {
                    unsigned first = this->first[value];

                    this->value[key] = value;
                    // list -> key <-> first
                    if (first != static_cast<unsigned>(-1)) {
                        this->previous[first] = key;
                    }
                    this->next[key] = first;
                    this->first[value] = key;
                    this->size[value]++;
                }
            }
        }

        void set_all(unsigned value, unsigned new_value) {
            // for key where this->value[key] == value
            //     set this->value[key] = new_value
            assert(value != new_value);
            assert(value != static_cast<unsigned>(-1));
            assert(new_value != static_cast<unsigned>(-1));
            auto size = this->size[value];
            auto key = this->first[value];
            auto new_value_size = this->size[new_value];
            auto new_value_first = this->first[new_value];

            this->size[value] = 0;
            this->size[new_value] = size + new_value_size;
            this->first[value] = static_cast<unsigned>(-1);
            this->first[new_value] = key;

            auto last = key;
            for (auto i = 0u; i < size; i++) {
                this->value[key] = new_value;
                last = key;
                key = this->next[key];
            }

            // concat the two linked list
            this->next[last] = new_value_first;
            this->previous[new_value_first] = last;
        }

        inline void pop_back(unsigned key) {
            unsigned value = this->value[key];
            if (value != static_cast<unsigned>(-1)) {
                assert(this->size[value] > 0);
                this->size[value]--;
                unsigned next = this->next[key];
                unsigned previous = this->previous[key];
                if (previous != static_cast<unsigned>(-1)) {
                    // index.previous -> index.next
                    this->next[previous] = next;
                } else {
                    // list -> index.next
                    this->first[value] = next;
                }
                if (next != static_cast<unsigned>(-1)) {
                    // index.previous <- index.next
                    this->previous[next] = previous;
                }
            }
        }

        inline void push_back(
            unsigned size, unsigned old_capacity, unsigned new_capacity,
            unsigned count, unsigned value = static_cast<unsigned>(-1)
        ) {
            ge1::push_back(
                this->value, size, old_capacity, new_capacity,
                count, static_cast<unsigned>(-1)
            );
            ge1::push_back(
                this->previous, size, old_capacity, new_capacity,
                count, static_cast<unsigned>(-1)
            );
            ge1::push_back(
                this->next, size, old_capacity, new_capacity,
                count, static_cast<unsigned>(-1)
            );
            for (auto i = 0u; i < count; i++) {
                set(size + i, value);
            }
        }

        inline void value_push_back(
            unsigned size, unsigned old_capacity, unsigned new_capacity,
            unsigned count
        ) {
            ge1::push_back(
                this->size, size, old_capacity, new_capacity, count, 0
            );
            ge1::push_back(
                this->first, size, old_capacity, new_capacity, count,
                static_cast<unsigned>(-1)
            );
        }
    };
}
