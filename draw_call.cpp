#include "draw_call.h"

namespace ge1 {

    draw_arrays_call::draw_arrays_call(
        GLuint vertex_array, GLint first, GLint count,
        GLuint program, GLenum mode
    ) :
        vertex_array(vertex_array), first(first), count(count),
        program(program), mode(mode)
    {
    }

    void draw_arrays_call::render() {
        glUseProgram(program);
        glBindVertexArray(vertex_array);
        glDrawArrays(mode, first, count);
    }

    draw_elements_call::draw_elements_call(
        GLuint vertex_array, GLuint program,
        GLenum mode, GLint count, GLenum type, unsigned indices
    ) :
        vertex_array(vertex_array), program(program),
        mode(mode), count(count), type(type),
        indices(reinterpret_cast<void*>(indices))
    {
    }

    void draw_elements_call::render()
    {
        glUseProgram(program);
        glBindVertexArray(vertex_array);
        glDrawElements(mode, count, type, indices);
    }

    multi_draw_arrays_indirect_call::multi_draw_arrays_indirect_call(
        GLuint vertex_array, const void* offset, GLint draw_count,
        GLuint program, GLenum mode
    ) :
        vertex_array(vertex_array), offset(offset), draw_count(draw_count),
        program(program), mode(mode)
    {
    }

    void multi_draw_arrays_indirect_call::render() {
        glUseProgram(program);
        glBindVertexArray(vertex_array);
        glMultiDrawArraysIndirect(mode, offset, draw_count, 0);
    }

}
