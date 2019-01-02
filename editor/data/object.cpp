#include "object.h"

namespace ge1 {

    object::object(mesh* m, GLuint face_program, GLuint vertex_program) :
        m(m),
        face_call(
            m->face_vertex_array.get_name(), 0, 0, face_program, GL_TRIANGLES
        ),
        vertex_call(
            m->vertex_array.get_name(), 0, 0, vertex_program, GL_POINTS
        )
    {

    }
}
