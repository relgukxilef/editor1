#include <iostream>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "composition.h"
#include "resources.h"
#include "draw_call.h"
#include "program.h"
#include "editor/operation.h"
#include "editor/operations/drag_vertex.h"

using namespace std;
using namespace glm;

using namespace ge1;

static GLuint view_properties_buffer;

static std::unique_ptr<operation> current_operation;
static context current_context;

void cursor_position_callback(GLFWwindow*, double x, double y) {
    if (current_operation) {
        if (
            current_operation->mouse_move_event(x, y) ==
            operation::status::finished
        ) {
            current_operation.reset();
        }
    }
}

void mouse_button_callback(
    GLFWwindow* window, int button, int action, int modifiers
) {
    if (current_operation) {
        if (
            current_operation->mouse_button_event(button, action, modifiers) ==
            operation::status::finished
        ) {
            current_operation.reset();
        }
    } else {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        current_operation.reset(new drag_vertex(current_context, x, y));
    }
}

void window_size_callback(GLFWwindow*, int width, int height) {
    current_context.width = width;
    current_context.height = height;
    glViewport(0, 0, width, height);

    current_context.current_view->projection_matrix = perspective(
        radians(45.0f), static_cast<float>(width) / height, 0.1f, 100.0f
    );

    glBindBuffer(GL_COPY_WRITE_BUFFER, view_properties_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, 16 * sizeof(float),
        value_ptr(
            current_context.current_view->projection_matrix *
            current_context.current_view->view_matrix
        )
    );
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

    enum uniform_block_bindings : GLuint {
        view_properties
    };

    glGenBuffers(1, &view_properties_buffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, view_properties_buffer);
    glBufferData(
        GL_COPY_WRITE_BUFFER, 16 * sizeof(float), nullptr, GL_DYNAMIC_DRAW
    );

    GLuint shape_buffer, color_buffer;

    enum attributes : GLuint {
        position, color
    };

    mesh my_mesh;
    my_mesh.vertex_array = create_vertex_array(4, {
        {{
            {position, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &shape_buffer},
        {{
            {color, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &color_buffer},
    });
    my_mesh.vertex_position_buffer = shape_buffer;

    my_mesh.vertex_positions = {
        {-1, -1, 0},
        {-1, 1, 0},
        {1, -1, 0},
        {1, 1, 0}
    };
    float colors[] = {
        1, 0, 1,
        0, 1, 1,
        1, 1, 0,
        1, 1, 1,
    };

    glBindBuffer(GL_COPY_WRITE_BUFFER, shape_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, 4 * 3 * sizeof(float),
        my_mesh.vertex_positions.data()
    );
    glBindBuffer(GL_COPY_WRITE_BUFFER, color_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, 4 * 3 * sizeof(float), colors
    );

    unique_shader fragment_utils = compile_shader(
        GL_FRAGMENT_SHADER, "shaders/utils.fs"
    );

    GLint solid_model_matrix, point_handle_model_matrix;

    unique_program solid_program = compile_program(
        "shaders/solid.vs", nullptr, nullptr, nullptr, "shaders/solid.fs",
        {fragment_utils.get_name()},
        {{"position", position}, {"color", color}},
        {
            {"model", &solid_model_matrix}
        }, {
            {"view_projection", view_properties},
        }
    );

    unique_program point_handle_program = compile_program(
        "shaders/point_handle.vs", nullptr, nullptr, nullptr,
        "shaders/point_handle.fs",
        {fragment_utils.get_name()},
        {{"position", position}},
        {
            {"model", &point_handle_model_matrix}
        }, {
            {"view_projection", view_properties},
        }
    );

    glUseProgram(solid_program.get_name());
    glUniformMatrix4fv(solid_model_matrix, 1, GL_FALSE, value_ptr(mat4(1)));

    glUseProgram(point_handle_program.get_name());
    glUniformMatrix4fv(
        point_handle_model_matrix, 1, GL_FALSE, value_ptr(mat4(1))
    );

    glEnable(GL_POINT_SPRITE);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glBindBufferRange(
        GL_UNIFORM_BUFFER, view_properties,
        view_properties_buffer, 0, 16 * sizeof(float)
    );

    current_context.current_view = new view();
    current_context.current_object = new object();
    current_context.current_object->mesh = &my_mesh;
    current_context.current_view->view_matrix =
        lookAt(vec3(2, 0, 2), {0, 0, 0}, {0, 0, 1});

    draw_call mesh_draw_call{
        my_mesh.vertex_array.get_name(), 0, 4,
        solid_program.get_name(), GL_TRIANGLE_STRIP
    };

    draw_call point_handle_draw_call{
        my_mesh.vertex_array.get_name(), 0, 4,
        point_handle_program.get_name(), GL_POINTS
    };


    composition composition;

    pass background_pass;
    pass foreground_pass;

    composition.passes.push_back(background_pass);
    composition.passes.push_back(foreground_pass);

    foreground_pass.renderables.push_back(mesh_draw_call);
    foreground_pass.renderables.push_back(point_handle_draw_call);


    int width, height;
    glfwGetWindowSize(window, &width, &height);
    window_size_callback(window, width, height);
    glfwSetCursorPosCallback(window, &cursor_position_callback);
    glfwSetMouseButtonCallback(window, &mouse_button_callback);

    glfwSetWindowSizeCallback(window, &window_size_callback);


    while (!glfwWindowShouldClose(window)) {
        glClearColor(255, 255, 255, 255);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        composition.render();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    return 0;
}
