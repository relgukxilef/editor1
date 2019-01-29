#include "delete_vertices.h"

namespace ge1 {

    operation::status delete_vertices::trigger(context& c, double x, double y) {
        auto& m = c.current_object->m;

        // delete all faces adjacent to this vertex
        auto vertex = m->selected_vertices.begin();
        while (vertex != m->selected_vertices.end()) {
            m->delete_vertex(*vertex);
            c.current_object->face_call.count =
                static_cast<int>(m->face_vertices.size());
            c.current_object->vertex_call.count--;

            vertex = m->selected_vertices.begin();
        }

        c.current_object->edge_call.count = m->edge_vertices.size();

        return status::finished;
    }
}
