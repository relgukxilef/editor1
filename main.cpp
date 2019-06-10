#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "ge1/composition.h"
#include "ge1/resources.h"
#include "ge1/draw_call.h"
#include "ge1/program.h"
#include "typed/glm_types.h"

using namespace std;
using namespace glm;

using namespace ge1;

void window_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

struct unique_glfw {
    unique_glfw() {
        if (!glfwInit()) {
            throw runtime_error("Couldn't initialize GLFW!");
        }
    }

    ~unique_glfw() {
        glfwTerminate();
    }
};

int main() {
    unique_glfw glfw;

    GLFWwindow* window;

    glfwWindowHint(GLFW_SAMPLES, 16);
    glfwSwapInterval(1);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    int screen_width = mode->width, screen_height = mode->height;

    window = glfwCreateWindow(
        screen_width, screen_height, "demo", nullptr, nullptr
    );
    if (!window) {
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        return -1;
    }

    GLuint shape_buffer, color_buffer;

    enum attributes : GLuint {
        position, color
    };

    unique_vertex_array mesh = create_vertex_array(4, {
        {{
            {position, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &shape_buffer},
        {{
            {color, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &color_buffer},
    });

    float positions[] = {
        -1, -1, 0,
        -1, 1, 0,
        1, -1, 0,
        1, 1, 0
    };
    float colors[] = {
        1, 0, 1,
        0, 1, 1,
        1, 1, 0,
        1, 1, 1,
    };

    glBindBuffer(GL_COPY_WRITE_BUFFER, shape_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, 4 * 3 * sizeof(float), positions
    );
    glBindBuffer(GL_COPY_WRITE_BUFFER, color_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, 4 * 3 * sizeof(float), colors
    );

    unique_shader fragment_utils = compile_shader(
        GL_FRAGMENT_SHADER, "shaders/utils.fs"
    );

    unique_program solid = compile_program(
        "shaders/solid.vs", nullptr, nullptr, nullptr, "shaders/solid.fs",
        {fragment_utils.get_name()},
        {{"position", position}, {"color", color}}, {}
    );

    draw_call mesh_draw_call{
        mesh.get_name(), 0, 4, solid.get_name(), GL_TRIANGLE_STRIP
    };


    composition composition;

    pass background_pass;
    pass foreground_pass;

    composition.passes.push_back(background_pass);
    composition.passes.push_back(foreground_pass);

    foreground_pass.renderables.push_back(mesh_draw_call);


    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetWindowSizeCallback(window, &window_size_callback);


    while (!glfwWindowShouldClose(window)) {
        composition.render();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    return 0;
}
