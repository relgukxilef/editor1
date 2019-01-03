#include "delete_face.h"

namespace ge1 {

    operation::status delete_face::trigger(context& c, double x, double y) {
        auto& m = c.current_object->m;

        // delete all faces adjacent to this vertex
        for (auto vertex : m->selected_vertices) {
            auto face_vertex = m->vertex_face_vertices.lower_bound({vertex, 0});
            while (
                face_vertex !=
                m->vertex_face_vertices.lower_bound({vertex + 1, 0})
            ) {
                c.current_object->face_call.count -= 3;
                m->delete_face(face_vertex->second / 3);
                face_vertex = m->vertex_face_vertices.lower_bound({vertex, 0});
            }
        }

        return status::finished;
    }
}
