#pragma once

#include <string>
#include <vector>

namespace ge1 {

    struct copy_dependency {
        unsigned reference; // what reference to follow for value
        unsigned attribute; // what attribute to copy

        unsigned next, previous; // list of dependencies on same attribute
    };

    struct vertex_attribute_format {
        std::string name;

        unsigned copy_dependencies_head; // a dependent copy_dependency
    };

    struct scalar_vertex_attribute_format : public vertex_attribute_format {
        unsigned size;
        unsigned reference_space;

        bool relative;
        bool active;
    };

    struct reference_vertex_attribute_format : public vertex_attribute_format {
        unsigned int reference_array;
        unsigned int map_index;
    };

    struct vertex_array_format {
        std::string name;
        unsigned int patch_size; // TODO: allow mixing patch_size

        std::vector<scalar_vertex_attribute_format> integer_attributes;
        std::vector<scalar_vertex_attribute_format> float_attributes;

        std::vector<copy_dependency> integer_copy_dependencies;
        std::vector<copy_dependency> float_copy_dependencies;

        std::vector<reference_vertex_attribute_format> reference_attributes;
    };

    struct reference_array {
        std::vector<unsigned> reference;

        std::vector<unsigned> head;
        std::vector<unsigned> next;
        std::vector<unsigned> previous;
    };

    struct vertex_array {
        // vertex_array[attribute][vertex]
        std::vector<std::vector<int>> integer_arrays;
        std::vector<std::vector<float>> float_arrays;

        // derived attributes
        std::vector<std::vector<int>> integer_copies;
        std::vector<std::vector<float>> float_copies;

        std::vector<reference_array> reference_arrays;

        std::vector<unsigned> vertex_selections;
        std::vector<unsigned> selection_vertices;
    };

    struct mesh {
        std::string name;

        // mesh[array][attribute][vertex]
        std::vector<vertex_array> arrays;
    };

    struct mesh_format {
        // format[mesh][array][attribute][vertex]
        std::string name;
        mesh_format() = default;

        std::vector<vertex_array_format> vertex_arrays;
        std::vector<scalar_vertex_attribute_format> instance_attributes;
        std::vector<mesh> meshes;
    };


    unsigned add_patch(vertex_array& array);
    void remove_patch(vertex_array& array, unsigned patch);
    void set_vertex_float_value(
        mesh_format& format,
        unsigned mesh, unsigned array, unsigned attribute, unsigned vertex,
        float value
    );
    void set_vertex_integer_value(
        mesh_format& format,
        unsigned mesh, unsigned array, unsigned attribute, unsigned vertex,
        int value
    );
    unsigned add_mesh(mesh_format& format);
    unsigned add_array(mesh_format& format);
    unsigned add_vertex_attribute(mesh_format& format, unsigned array);
    unsigned add_instance_attribute(mesh_format& format);
}
