
#include "program.h"

#include <iostream>
#include <fstream>
#include <exception>
#include <string>

namespace ge1 {

    using namespace std::literals::string_literals;

    GLuint compile_shader_from_source(GLenum type, const char* source_code) {
        GLuint name = glCreateShader(type);
        glShaderSource(name, 1, &source_code, nullptr);
        glCompileShader(name);

        GLint success;
        GLchar info_log[1024];
        glGetShaderiv(name, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(name, 1024, nullptr, info_log);
            glDeleteShader(name);
            throw std::runtime_error(std::string(info_log));
        }

        return name;
    }

    GLuint compile_shader(GLenum type, const char* path) {
        std::ifstream file(path);

        if (!file.is_open()) {
            throw std::runtime_error("Couldn't open "s + path);
        }

        std::string source(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        file.close();

        char const* source_c_str = source.c_str();

        try {
            return compile_shader_from_source(type, source_c_str);
        } catch (std::runtime_error e) {
            throw std::runtime_error(
                "Could not compile "s + path + "\n"s + e.what()
            );
        }
    }

    GLuint compile_program(const char* vertex_shader,
        const char* tesselation_control_shader,
        const char* tesselation_evaluation_shader,
        const char* geometry_shader,
        const char* fragment_shader,
        span<const GLuint> libraries,
        span<const program_attribute_parameter> attributes,
        span<const program_uniform_parameter> uniforms,
        span<const program_uniform_block_parameter> uniform_blocks
    ) {
        unique_shader shaders[5] = {
            vertex_shader == nullptr ?
                0 : compile_shader(GL_VERTEX_SHADER, vertex_shader),
            tesselation_control_shader == nullptr ?
                0 : compile_shader(
                    GL_TESS_CONTROL_SHADER, tesselation_control_shader
                ),
            tesselation_evaluation_shader == nullptr ?
                0 : compile_shader(
                    GL_TESS_EVALUATION_SHADER, tesselation_evaluation_shader
                ),
            geometry_shader == nullptr ?
                0 : compile_shader(GL_GEOMETRY_SHADER, geometry_shader),
            fragment_shader == nullptr ?
                0 : compile_shader(GL_FRAGMENT_SHADER, fragment_shader)
        };

        GLuint name = glCreateProgram();

        for (auto& s : shaders) {
            if (s.get_name() != 0) {
                glAttachShader(name, s.get_name());
            }
        }

        for (GLuint s : libraries) {
            if (s != 0) {
                glAttachShader(name, s);
            }
        }

        for (auto& attribute : attributes) {
            glBindAttribLocation(name, attribute.location, attribute.name);
        }

        glLinkProgram(name);

        GLint success;
        GLchar info_log[1024];
        glGetProgramiv(name, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(name, 1024, nullptr, info_log);
            glDeleteProgram(name);
            throw std::runtime_error("Linking failed:\n"s + info_log);
        }

        for (auto& s : shaders) {
            if (s.get_name() != 0) {
                glDetachShader(name, s.get_name());
            }
        }

        for (auto& uniform : uniforms) {
            *uniform.location = glGetUniformLocation(name, uniform.name);
        }

        for (auto& uniform_block : uniform_blocks) {
            GLuint index = glGetUniformBlockIndex(name, uniform_block.name);
            glUniformBlockBinding(name, index, uniform_block.binding);
        }

        return name;
    }

}
