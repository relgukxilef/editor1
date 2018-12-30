#pragma once

#include <initializer_list>
#include <utility>

#include <GL/glew.h>

#include "resources.h"
#include "span.h"

namespace ge1 {

    inline void delete_program(GLuint program) {
        glDeleteProgram(program);
    }

    inline void delete_shader(GLuint shader) {
        glDeleteShader(shader);
    }

    typedef unique_object<delete_program> unique_program;
    typedef unique_object<delete_shader> unique_shader;

    GLuint compile_shader(GLenum type, const char* path);
    GLuint compile_shader_from_source(GLenum type, const char* source_code);

    struct program_attribute_parameter {
        const char* name;
        GLuint location;
    };

    struct program_uniform_parameter {
        const char* name;
        GLint* location;
    };

    struct program_uniform_block_parameter {
        const char* name;
        GLuint binding;
    };

    GLuint compile_program(
        const char* vertex_shader,
        const char* tesselation_control_shader,
        const char* tesselation_evaluation_shader,
        const char* geometry_shader,
        const char* fragment_shader,
        span<GLuint> libraries,
        span<program_attribute_parameter> attributes,
        span<program_uniform_parameter> uniforms = {},
        span<program_uniform_block_parameter> uniform_blocks = {}
    );
}
