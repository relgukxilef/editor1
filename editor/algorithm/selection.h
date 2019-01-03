#pragma once

namespace ge1 {

    template<class Selected, class Set>
    struct selection_vector {
        struct reference;

        void push_back(bool);
        void pop_back();
        reference operator[](unsigned int);

        struct reference {
            reference& operator=(bool);
            reference& operator=(const reference&);
            operator bool() const;

            unsigned int i;
            Selected& selected;
            Set& set;
        };

        Selected& selected;
        Set& set;
    };

    template<class Selected, class Set>
    selection_vector<Selected, Set> make_selection_vector(Selected&, Set&);

    template<class Selected, class Set>
    struct selection_set {
        typedef typename Set::iterator iterator;

        iterator begin();
        iterator end();
        iterator erase(iterator i);
        unsigned int erase(unsigned int i);
        iterator insert(unsigned int i);

        Selected& selected;
        Set& set;
    };

    template<class Selected, class Set>
    selection_set<Selected, Set> make_selection_set(Selected&, Set&);


    template<class Selected, class Set>
    void selection_vector<Selected, Set>::push_back(bool x) {
        if (x) {
            set.insert(selected.size());
        }
        selected.push_back(x);
    }

    template<class Selected, class Set>
    void selection_vector<Selected, Set>::pop_back() {
        if (selected[selected.size() - 1]) {
            set.erase(selected.size() - 1);
        }
        selected.pop_back();
    }

    template<class Selected, class Set>
    typename selection_vector<Selected, Set>::reference
    selection_vector<Selected, Set>::operator[](unsigned int pos) {
        return {pos, selected, set};
    }

    template<class Selected, class Set>
    typename selection_vector<Selected, Set>::reference&
    selection_vector<Selected, Set>::reference::operator=(bool x) {
        if (x != selected[i]) {
            if (x) {
                set.insert(i);
            } else {
                set.erase(i);
            }
        }
        selected[i] = x;

        return *this;
    }

    template<class Selected, class Set>
    typename selection_vector<Selected, Set>::reference&
    selection_vector<Selected, Set>::reference::operator=(
        const selection_vector::reference& other
    ) {
        return operator=(bool(other));
    }

    template<class Selected, class Set>
    selection_vector<Selected, Set> make_selection_vector(
        Selected& selected, Set& set
    ) {
        return {selected, set};
    }

    template<class Selected, class Set>
    selection_vector<Selected, Set>::reference::operator bool() const {
        return selected[i];
    }

    template<class Selected, class Set>
    typename selection_set<Selected, Set>::iterator
    selection_set<Selected, Set>::begin() {
        return set.begin();
    }

    template<class Selected, class Set>
    typename selection_set<Selected, Set>::iterator
    selection_set<Selected, Set>::end() {
        return set.end();
    }

    template<class Selected, class Set>
    typename selection_set<Selected, Set>::iterator
    selection_set<Selected, Set>::erase(selection_set::iterator i) {
        selected[*i] = false;
        return set.erase(i);
    }

    template<class Selected, class Set>
    unsigned int selection_set<Selected, Set>::erase(unsigned int i) {
        selected[i] = false;
        return set.erase(i);
    }

    template<class Selected, class Set>
    typename selection_set<Selected, Set>::iterator
    selection_set<Selected, Set>::insert(unsigned int i) {
        selected[i] = true;
        return set.insert(i);
    }

    template<class Selected, class Set>
    selection_set<Selected, Set> make_selection_set(
        Selected& selected, Set& set
    ) {
        return {selected, set};
    }

}
