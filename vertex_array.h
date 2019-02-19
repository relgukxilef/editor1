#pragma once

#include <initializer_list>
#include <utility>

#include <GL/glew.h>

#include "resources.h"
#include "span.h"

namespace ge1 {

    inline void delete_vertex_array(GLuint vertex_array) {
        glDeleteVertexArrays(1, &vertex_array);
    }

    inline void delete_buffer(GLuint buffer) {
        glDeleteBuffers(1, &buffer);
    }

    typedef unique_object<delete_vertex_array> unique_vertex_array;
    typedef unique_object<delete_buffer> unique_buffer;

    struct attribute_pointer_parameter {
        GLuint vertex_buffer;
        GLuint index;
        GLint size;
        GLenum type;
        GLboolean normalized;
        GLsizei stride;
        unsigned int pointer;
    };

    struct attribute_parameter {
        GLuint index;
        GLint size;
        GLenum type;
        GLboolean normalized;
        unsigned int offset;
    };

    struct attribute_pack_parameter {
        span<attribute_parameter> attributes;
        GLuint stride;
        GLenum usage;
        GLuint *vertex_buffer;
    };

    struct command_parameter {
        GLuint vertex_capacity, instance_capacity;
        GLuint *mesh, *instances;
    };

    GLuint create_vertex_array(
        span<attribute_pointer_parameter> attributes,
        GLuint element_array_buffer = 0,
        GLuint draw_indirect_buffer = 0
    );

    GLuint create_vertex_array(
        unsigned int vertex_capacity,
        span<attribute_pack_parameter> attribute_packs,
        unsigned int draw_indirect_capacity = 0,
        GLuint* draw_indirect_buffer = nullptr,
        GLenum draw_indirect_usage = GL_DYNAMIC_DRAW
    );

    GLuint create_vertex_array(
        span<attribute_pack_parameter> vertex_attribute_packs,
        span<attribute_pack_parameter> instance_attribute_packs,
        span<command_parameter> commands, GLuint* draw_indirect_buffer,
        GLenum draw_indirect_usage = GL_DYNAMIC_DRAW
    );
}
