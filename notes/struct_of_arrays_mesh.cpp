
#include <string>

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
};

struct map {
    unsigned* value, * size, * head, * next, * previous;

    linked_list keys(unsigned value) const {
        return {head[value], size[value], next, previous};
    }

    void set(unsigned index, unsigned value) {
        unsigned old_value = this->value[index];
        if (old_value != value) {
            unsigned head = this->head[value];

            // remove existing reference
            unsigned next = this->next[index];
            unsigned previous = this->previous[index];
            if (previous != static_cast<unsigned>(-1)) {
                // index.previous -> index.next
                this->next[previous] = next;
            } else {
                // list -> index.next
                this->head[old_value] = next;
            }
            if (next != static_cast<unsigned>(-1)) {
                // index.previous <- index.next
                this->previous[next] = previous;
            }

            // add new reference
            this->value[index] = value;
            // list -> index <-> head
            this->previous[head] = index;
            this->next[index] = head;
            this->head[value] = index;
        }
    }
};

struct mesh_format {
    struct attributes {
        std::string* name;
        unsigned* array;
        unsigned* array_attributes_next, * array_attributes_previous;
    };

    struct scalar_attributes : public attributes {
        unsigned* size;
        unsigned* copy_dependency_first;
        unsigned* copy_dependency_size;
    } integer_attributes, float_attributes;

    struct reference_attributes : public attributes {
        unsigned* target_array;
        unsigned* index;
        unsigned* index_next, index_previous;
    } reference_attributes;

    struct vertex_arrays {
        std::string *name;
        unsigned* patch_size;
        unsigned* float_first, * float_size;
    } vertex_arrays;

    struct copy_dependencies {
        unsigned* reference;
        unsigned* attribute;

        unsigned* dependency_next;
        unsigned* dependency_previous;
    } copy_dependencies;

    struct mesh {
        std::string* names;
        unsigned* array_sizes;

        float** floats;
        float** float_copies;        

        unsigned** references;
        unsigned** references_next;
        unsigned** references_previous;

        unsigned** indices_first;
        unsigned** indices_size;
    }* meshes;

    map float_attributes_array() {
        return {
            float_attributes.array, vertex_arrays.float_size,
            vertex_arrays.float_first, 
            float_attributes.array_attributes_next, 
            float_attributes.array_attributes_previous
        };
    }

    map references(unsigned mesh, unsigned attribute) {
        auto target = reference_attributes.target_array[attribute];
        auto index = reference_attributes.index[attribute];
        return {
            meshes[mesh].references[attribute],
            meshes[mesh].indices_first[index], 
            meshes[mesh].indices_size[index],
            meshes[mesh].references_next[attribute],
            meshes[mesh].references_previous[attribute]
        };
    }

    map float_attribute_copy_dependencies() {
        // which copy attributes depend on the given attribute
        return {
            copy_dependencies.reference,
            float_attributes.copy_dependency_size,
            float_attributes.copy_dependency_first,
            copy_dependencies.dependency_next,
            copy_dependencies.dependency_previous
        };
    }
};

void set_reference_value(
    mesh_format& f, unsigned mesh, unsigned attribute, unsigned vertex, unsigned value
) {
    f.references(mesh, attribute).set(vertex, value);
}

void set_float_value(
    mesh_format& f, unsigned mesh, unsigned attribute, unsigned vertex, float value
) {
    auto& m = f.meshes[mesh];
    m.floats[attribute][vertex] = value;
    // determine dependent copy attributes
    for (auto copy_attribute : f.float_attribute_copy_dependencies().keys(attribute)) {
        // determine dependent vertices
        auto reference_attribute = f.copy_dependencies.reference[copy_attribute];
        for (auto copy_vertex : f.references(mesh, reference_attribute).keys(vertex)) {
            m.float_copies[copy_attribute][copy_vertex] = value;
        }
    }
}
