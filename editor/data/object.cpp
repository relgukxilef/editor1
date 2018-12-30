#include "object.h"

namespace ge1 {

    object::object(mesh* m, GLuint program) :
        m(m),
        call(
            m->vertex_array.get_name(), 0,
            static_cast<int>(m->vertex_position_capacity), program, GL_POINTS
        )
    {

    }
}
