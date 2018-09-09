#ifndef PROGRAM_H
#define PROGRAM_H

#include <initializer_list>
#include <utility>

#include <GL/glew.h>

namespace ge1 {

    GLuint compile_program(
        const char* vertex_shader,
        const char* tesselation_control_shader,
        const char* tesselation_evaluation_shader,
        const char* geometry_shader,
        const char* fragment_shader,
        std::initializer_list<std::pair<const char*, GLuint>> attributes,
        std::initializer_list<std::pair<const char*, GLuint>> uniforms = {}
    );

    struct program {
        program() = default;
        program(
            const char* vertex_shader,
            const char* tesselation_control_shader,
            const char* tesselation_evaluation_shader,
            const char* geometry_shader,
            const char* fragment_shader,
            std::initializer_list<std::pair<const char*, GLuint>> attributes,
            std::initializer_list<std::pair<const char*, GLuint>> uniforms = {}
        );
        program(const char* vertex_shader,
            const char* fragment_shader,
            std::initializer_list<std::pair<const char*, GLuint>> attributes,
            std::initializer_list<std::pair<const char*, GLuint>> uniforms = {}
        );

        ~program();

        GLuint get_name() const;

        operator GLuint() const;

    private:
        GLuint name;
    };


    inline program::program(
        const char* vertex_shader,
        const char* tesselation_control_shader,
        const char* tesselation_evaluation_shader,
        const char* geometry_shader,
        const char* fragment_shader,
        std::initializer_list<std::pair<const char*, GLuint>> attributes,
        std::initializer_list<std::pair<const char*, GLuint>> uniforms
    ) :
        name(compile_program(
            vertex_shader,
            tesselation_control_shader,
            tesselation_evaluation_shader,
            geometry_shader,
            fragment_shader,
            attributes,
            uniforms
        ))
    {}

    inline program::program(
        const char* vertex_shader, const char* fragment_shader,
        std::initializer_list<std::pair<const char*, GLuint>> attributes,
        std::initializer_list<std::pair<const char*, GLuint>> uniforms
    ) :
        program(
            vertex_shader,
            nullptr,
            nullptr,
            nullptr,
            fragment_shader,
            attributes,
            uniforms
        )
    {}

    inline program::~program() {
        if (name != 0) {
            glDeleteProgram(name);
        }
    }

    inline GLuint program::get_name() const {
        return name;
    }

    inline ge1::program::operator GLuint() const {
        return get_name();
    }

}

#endif // PROGRAM_H
