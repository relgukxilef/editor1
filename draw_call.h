#ifndef DRAW_CALL_H
#define DRAW_CALL_H

#include <GL/glew.h>

#include "renderable.h"
#include "vertex_array.h"

namespace ge1 {

    struct draw_arrays_call : public renderable {
        draw_arrays_call(
            GLuint vertex_array, GLint first, GLint count,
            GLuint program, GLenum mode
        );

        void render() override;

        GLuint vertex_array;
        GLint first;
        GLint count;
        GLuint program;
        GLenum mode;
    };

    struct draw_elements_call : public renderable {
        draw_elements_call(
            GLuint vertex_array, GLuint program,
            GLenum mode, GLint count, GLenum type, unsigned indices
        );

        void render() override;

        GLuint vertex_array;
        GLuint program;
        GLenum mode;
        GLsizei count;
        GLenum type;
        const void *indices;
    };

    struct draw_arrays_indirect_command {
        GLuint count, instance_count, first, base_instance;
    };

    struct multi_draw_arrays_indirect_call : public renderable {
        multi_draw_arrays_indirect_call(
            GLuint vertex_array, const void *offset, GLint draw_count,
            GLuint program, GLenum mode
        );

        void render() override;

        GLuint vertex_array;
        const void *offset;
        GLint draw_count;
        GLuint program;
        GLenum mode;
    };

}

#endif // DRAW_CALL_H
