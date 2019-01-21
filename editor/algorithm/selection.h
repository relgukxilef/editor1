#pragma once

namespace ge1 {

    template<class Set, class Vector>
    void selection_set(
        Set& set, Vector& vector, unsigned int index, bool value
    ) {
        vector[index] = value;
        if (value) {
            set.insert(index);
        } else {
            set.erase(index);
        }
    }

    // TODO: instead of vector could take iterators to index and last element
    template<class Set, class Vector>
    void selection_erase(Set& set, Vector& vector, unsigned int index) {
        unsigned int last = vector.size() - 1;
        if (last != index) {
            selection_set(set, vector, index, vector[last]);
        }
        vector.pop_back();
        set.erase(last);
    }
}
