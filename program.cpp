
#include "program.h"

#include <iostream>
#include <fstream>
#include <exception>
#include <string>

namespace ge1 {

    using namespace std::literals::string_literals;

    struct shader {
        shader(GLenum type, const char *path) : name(0) {
            if (path != nullptr) {
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
                GLint length = static_cast<GLint>(source.length());

                name = glCreateShader(type);
                glShaderSource(name, 1, &source_c_str, &length);
                glCompileShader(name);

                GLint success;
                GLchar info_log[1024];
                glGetShaderiv(name, GL_COMPILE_STATUS, &success);
                if (!success) {
                    glGetShaderInfoLog(name, 1024, nullptr, info_log);
                    glDeleteShader(name);
                    throw std::runtime_error(
                        "Compilation of "s + path + " failed:\n" + info_log
                    );
                }
            }
        }

        shader(shader&& other) {
            if (name != 0) {
                glDeleteShader(name);
            }
            name = other.name;
            other.name = 0;
        }

        ~shader() {
            if (name != 0) {
                glDeleteShader(name);
            }
        }

        GLuint name;
    };

    GLuint compile_program(
        const char* vertex_shader,
        const char* tesselation_control_shader,
        const char* tesselation_evaluation_shader,
        const char* geometry_shader,
        const char* fragment_shader,
        std::initializer_list<std::pair<const char*, GLuint>> attributes,
        std::initializer_list<std::pair<const char*, GLuint>>
    ) {
        shader shaders[5] = {
            {GL_VERTEX_SHADER, vertex_shader},
            {GL_TESS_CONTROL_SHADER, tesselation_control_shader},
            {GL_TESS_EVALUATION_SHADER, tesselation_evaluation_shader},
            {GL_GEOMETRY_SHADER, geometry_shader},
            {GL_FRAGMENT_SHADER, fragment_shader}
        };

        GLuint name = glCreateProgram();

        for (auto& s : shaders) {
            if (s.name != 0) {
                glAttachShader(name, s.name);
            }
        }

        for (auto& argument : attributes) {
            glBindAttribLocation(
                name, std::get<1>(argument), std::get<0>(argument));
        }

        glLinkProgram(name);

        GLint success;
        GLchar info_log[1024];
        glGetProgramiv(name, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(name, 1024, nullptr, info_log);
            glDeleteProgram(name);
            throw std::runtime_error("Shader linking failed:\n"s + info_log);
        }

        for (auto& s : shaders) {
            if (s.name != 0) {
                glDetachShader(name, s.name);
            }
        }

        return name;
    }

}
