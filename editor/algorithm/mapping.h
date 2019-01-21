#pragma once

namespace ge1 {

    template<class Set, class Vector>
    void mapping_set_value(
        Set& set, Vector& vector, unsigned int index, unsigned int value
    ) {
        if (value != vector[index]) {
            set.erase({vector[index], index});
            set.insert({value, index});
            vector[index] = value;
        }
    }

    template<class Set, class Vector>
    void mapping_erase_key(Set& set, Vector& vector, unsigned int index) {
        unsigned int last = vector.size() - 1;

        mapping_set_value(set, vector, index, vector[last]);
        set.erase({vector[last], last});
        vector.pop_back();
    }

    template<class Set, class Vector>
    void mapping_erase_value(
        Set& set, Vector& vector, unsigned int last, unsigned int value
    ) {
        // handling cascaded deletes is trivial and can be done by the caller
        if (last != value) {
            auto i = set.lower_bound({last, 0});
            while (i != set.end()) {
                mapping_set_value(set, vector, i->second, value);
                i = set.lower_bound({last, 0});
            }
        }
    }
}
