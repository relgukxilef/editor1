#include "add_face.h"

using namespace glm;

namespace ge1 {

    operation::status add_face::trigger(context& c, double, double) {
        auto& m = c.current_object->m;

        if (m->selected_vertices.size() == 3) {
            // TODO: update all objects using this mesh
            c.current_object->face_call.count += 3;

            auto vertex = m->selected_vertices.begin();
            auto face_vertex_count = m->face_vertex_positions.size();
            m->face_vertex_positions.push_back(m->vertex_positions[*vertex]);
            m->vertex_face_vertices.insert({*vertex, face_vertex_count});
            vertex++;
            m->face_vertex_positions.push_back(m->vertex_positions[*vertex]);
            m->vertex_face_vertices.insert({*vertex, face_vertex_count + 1});
            vertex++;
            m->face_vertex_positions.push_back(m->vertex_positions[*vertex]);
            m->vertex_face_vertices.insert({*vertex, face_vertex_count + 2});
        }

        return status::finished;
    }
}
