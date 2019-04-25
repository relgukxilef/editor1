#include "allocator.h"

#include <cstdlib>
#include <algorithm>
#include <stdexcept>

namespace ge1 {

    template<class T>
    void reallocate(std::unique_ptr<T[]>& memory, size_t new_size) {
        auto old = memory.release();
        auto reallocated = std::realloc(old, new_size * sizeof(T));

        if (reallocated) {
            memory.reset(reinterpret_cast<T*>(reallocated));
        } else {
            delete old;
            throw std::runtime_error("failed to allocate memory");
        }
    }

    unsigned allocate(allocator& a, unsigned size) {
        auto i = 0u;
        while (a.end[i] - a.begin[i] < size) {
            i = a.next[i];
            if (i == 0) {
                return a.capacity; // past the end index
            }
        }
        if (a.end[i] - a.begin[i] != size) {
            // split up block
            a.begin[i] = a.begin[i] + size;

        } else {
            // remove block
            // i.previous <-> i.next
            a.next[a.previous[i]] = a.next[i];
            a.previous[a.next[i]] = a.previous[i];

            a.node_size--;

            if (i != a.node_size) {
                // last.previous <-> i <-> last.next
                a.next[i] = a.next[a.node_size];
                a.previous[i] = a.previous[a.node_size];
                a.next[a.previous[a.node_size]] = i;
                a.previous[a.next[a.node_size]] = i;

                a.begin[i] = a.begin[a.node_size];
                a.end[i] = a.end[a.node_size];
            }
        }

        return a.begin[i];
    }

    void grow(allocator& a) {
        if (a.node_size == a.node_capacity) {
            a.node_capacity = std::max(1u, a.node_capacity * 2);
            reallocate(a.next, a.node_capacity);
            reallocate(a.previous, a.node_capacity);
            reallocate(a.begin, a.node_capacity);
            reallocate(a.end, a.node_capacity);
        }
    }

    void deallocate(allocator& a, unsigned address, unsigned size) {
        // find blocks before and after the freed block
        auto node = a.first;
        if (a.begin[a.first] > address) {
            if (address + size == a.begin[a.first]) {
                // merge with first block
                a.begin[a.first] = address;

            } else {
                // insert node before first one
                grow(a);

                // first.previous <-> i <-> first
                a.begin[a.node_size] = address;
                a.end[a.node_size] = address + size;
                a.next[a.node_size] = a.first;
                a.previous[a.node_size] = a.previous[a.first];

                a.next[a.previous[a.first]] = a.node_size;
                a.previous[a.first] = a.node_size;

                a.first = a.node_size;
                a.node_size++;
            }

        } else {
            while (a.begin[node] < address) {
                node = a.next[node];
                if (node == a.first) {
                    break;
                }
            }

            // node is first after deallocated range
            auto previous = a.previous[node];

            if (address + size == a.begin[node]) {
                // merge
                if (a.end[previous] == address) {
                    // merge node with previous
                    // previous <-> node.next
                    a.end[previous] = a.end[node];
                    a.next[previous] = a.next[node];
                    a.previous[a.next[node]] = previous;

                    a.node_size--;

                    a.begin[node] = a.begin[a.node_size];
                    a.end[node] = a.end[a.node_size];
                    a.next[a.previous[a.node_size]] = node;
                    a.previous[a.next[a.node_size]] = node;

                } else {
                    // extend node
                    a.begin[node] = address;
                }

            } else if (a.end[previous] == address) {
                // extend previous
                a.end[previous] += size;

            } else {
                // insert node before node
                grow(a);

                // previous <-> i <-> node
                a.begin[a.node_size] = address;
                a.end[a.node_size] = address + size;
                a.next[a.node_size] = node;
                a.previous[a.node_size] = previous;

                a.previous[node] = a.node_size;
                a.next[previous] = a.node_size;

                a.node_size++;
            }
        }

    }
}
