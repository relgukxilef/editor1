#include "add_face.h"

using namespace glm;

namespace ge1 {

    operation::status add_face::trigger(context& c, double, double) {
        auto& m = c.current_object->m;

        if (m->selected_vertices.size() == 3) {
            // TODO: update all objects using this mesh
            c.current_object->face_call.count += 3;

            auto vertex = m->selected_vertices.begin();
            unsigned int vertices[3];
            for (int i = 0; i < 3; i++) {
                vertices[i] = *vertex;
                vertex++;
            }

            m->add_face(vertices);
        }

        return status::finished;
    }
}
