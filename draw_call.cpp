#include "draw_call.h"

namespace ge1 {

    draw_call::draw_call(
        const vertex_array* data, GLuint program, GLenum mode
    ) :
        data(data), program(program), mode(mode)
    {
    }

    void draw_call::render() {
        // NOTE: VAO does not store non-array attribute values
        glUseProgram(program);
        glBindVertexArray(data->get_vao());
        glDrawArrays(mode, data->get_first(), data->get_count());
    }

}
