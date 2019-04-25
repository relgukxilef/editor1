#pragma once

#include "linked_list.h"

namespace ge1 {
    struct map {
        unsigned *value = nullptr, *next = nullptr, *previous = nullptr;
        unsigned *size = nullptr, *first = nullptr;

        linked_list keys(unsigned value) const {
            return {first[value], size[value], next, previous};
        }

        void set(unsigned key, unsigned value) {
            // set this->value[key] to value
            unsigned old_value = this->value[key];
            if (old_value != value) {
                unsigned first = this->first[value];

                // remove existing reference
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

                // add new reference
                this->value[key] = value;
                // list -> key <-> first
                if (first != static_cast<unsigned>(-1)) {
                    this->previous[first] = key;
                }
                this->next[key] = first;
                this->first[value] = key;
            }
        }

        void set_all(unsigned value, unsigned new_value) {
            // for key where this->value[key] == value
            //     set this->value[key] = new_value
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

        void pop_back(unsigned key) {
            unsigned value = this->value[key];
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

        void push_back(unsigned key, unsigned value) {
            unsigned first = this->first[value];
            this->size[value]++;
            this->value[key] = value;
            // list -> key <-> first
            this->previous[key] = static_cast<unsigned>(-1);
            if (first != static_cast<unsigned>(-1)) {
                this->previous[first] = key;
            }
            this->next[key] = first;
            this->first[value] = key;
        }

        void value_push_back(unsigned value) {
            size[value] = 0;
            first[value] = static_cast<unsigned>(-1);
        }
    };
}
