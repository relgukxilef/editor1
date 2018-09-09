#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "composition.h"
#include "typed/vertex_array.h"
#include "vertex_vector.h"
#include "resources.h"
#include "draw_call.h"
#include "program.h"
#include "vertex_buffer.h"
#include "typed/glm_types.h"

using namespace std;
using namespace glm;

using namespace ge1;

void window_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_SAMPLES, 16);
    glfwSwapInterval(1);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    int screen_width = mode->width, screen_height = mode->height;

    window = glfwCreateWindow(
        screen_width, screen_height, "demo", nullptr, nullptr
    );
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        return -1;
    }

    typed::vertex_attribute<glm::vec3, 0> position{GL_FALSE};

    vertex_attribute_pack_vector mesh_data({&position});

    vertex_vector mesh({&mesh_data});

    vertex_buffer buffer(mesh, 4);

    position(buffer[0]) = {-1, -1, 0};
    position(buffer[1]) = {1, -1, 0};
    position(buffer[2]) = {-1, 1, 0};
    position(buffer[3]) = {1, 1, 0};

    mesh.push_back(buffer);




    program solid(
        "shaders/paint.vs",
        "shaders/paint.tcs",
        "shaders/paint.tes",
        "shaders/paint.gs",
        "shaders/paint.fs",
        {{"position", position}}
    );

    draw_call mesh_draw_call{&mesh, solid, GL_PATCHES};

    // TODO: should be called by draw_call
    glPatchParameteri(GL_PATCH_VERTICES, 4);


    composition composition;

    pass foreground_pass;

    foreground_pass.clear_mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

    composition.passes.push_back(foreground_pass);

    foreground_pass.renderables.push_back(mesh_draw_call);


    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetWindowSizeCallback(window, &window_size_callback);

    glClearColor(255, 255, 255, 0);

    glEnable(GL_DEPTH_TEST);


    while (!glfwWindowShouldClose(window)) {
        composition.render();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    cout << "Hello World!" << endl;
    return 0;
}
