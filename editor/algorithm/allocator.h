#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <memory>

namespace ge1 {

    struct allocator {
        std::unique_ptr<unsigned[]> next, previous, begin, end;
        unsigned node_size, node_capacity, capacity, first;
    };

    unsigned allocate(allocator& a, unsigned size);

    void deallocate(allocator& a, unsigned address, unsigned size);
}

#endif // ALLOCATOR_H
