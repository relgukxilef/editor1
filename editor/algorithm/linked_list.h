#pragma once

struct linked_list {
    unsigned& first, & size;
    unsigned* next, * previous;

    struct iterator {
        unsigned index, size;
        unsigned current;
        const unsigned* next;

        iterator operator++() {
            index++;
            current = next[current];
            return *this;
        }
        bool operator!=(const iterator& other) {
            return index != other.index;
        }
        unsigned operator*() {
            return current;
        }
    };

    iterator begin() const {
        return {0, size, first, next};
    }
    iterator end() const {
        return {size, size, first, next};
    }

    void insert(unsigned index) {
        // list -> index <-> first
        previous[first] = index;
        next[index] = first;
        first = index;
        size++;
    }

    void remove(unsigned index) {
        unsigned next = this->next[index];
        unsigned previous = this->previous[index];
        if (previous != static_cast<unsigned>(-1)) {
            // index.previous -> index.next
            this->next[previous] = next;
        } else {
            // list -> index.next
            this->first = next;
        }
        if (next != static_cast<unsigned>(-1)) {
            // a.previous <- a.next
            this->previous[next] = previous;
        }
        size--;
    }
};
