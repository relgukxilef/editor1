#include "add_face.h"

using namespace glm;

namespace ge1 {

    operation::status add_face::trigger(context& c, double, double) {
        auto& m = c.current_object->m;

        if (m->selected_vertices.size() == 3) {
            // TODO: update all objects using this mesh
            c.current_object->face_call.count += 3;

            auto vertex = m->selected_vertices.begin();
            std::array<unsigned int, 3> vertices;
            for (unsigned int i = 0; i < 3; i++) {
                vertices[i] = *vertex;
                vertex++;
            }

            m->add_face(vertices);
        }

        c.current_object->edge_call.count = m->edge_vertices.size();

        return status::finished;
    }
}
