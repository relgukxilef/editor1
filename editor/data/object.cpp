#include "object.h"

namespace ge1 {

    object::object(
        mesh* m, GLuint face_program, GLuint edge_program, GLuint vertex_program
    ) :
        m(m),
        face_call(
            m->face_vertex_array.get_name(), 0, 0, face_program, GL_TRIANGLES
        ),
        edge_call(
            m->edge_vertex_array.get_name(), 0, 0, edge_program, GL_LINES
        ),
        vertex_call(
            m->vertex_array.get_name(), 0, 0, vertex_program, GL_POINTS
        )
    {

    }
}
