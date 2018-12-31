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
#include "editor/operations/add_vertex.h"
#include "editor/operations/select_vertex.h"

using namespace std;
using namespace glm;

using namespace ge1;

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

static unique_glfw glfw;

static GLuint view_properties_buffer;

static operation* current_operation = nullptr;
static context current_context;

static drag_vertex drag_vertex_operation;
static add_vertex add_vertex_operation;
static select_vertex select_vertex_operation;

void cursor_position_callback(GLFWwindow*, double x, double y) {
    if (current_operation) {
        if (
            current_operation->mouse_move_event(
                current_context, x, y
            ) == operation::status::finished
        ) {
            current_operation = nullptr;
        }
    }
}

void mouse_button_callback(
    GLFWwindow* window, int button, int action, int modifiers
) {
    if (current_operation) {
        if (
            current_operation->mouse_button_event(
                current_context, button, action, modifiers
            ) == operation::status::finished
        ) {
            current_operation = nullptr;
        }

    } else if (action == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);

        if (modifiers & GLFW_MOD_SHIFT) {
            current_operation = &add_vertex_operation;
        } else if (modifiers & GLFW_MOD_CONTROL) {
            current_operation = &select_vertex_operation;
        } else {
            current_operation = &drag_vertex_operation;
        }

        if (
            current_operation != nullptr &&
            current_operation->trigger(current_context, x, y) ==
            operation::status::finished
        ) {
            current_operation = nullptr;
        }
    }
}

void window_size_callback(GLFWwindow*, int width, int height) {
    current_context.width = static_cast<unsigned int>(width);
    current_context.height = static_cast<unsigned int>(height);
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

int main() {
    GLFWwindow* window;

    glfwWindowHint(GLFW_SAMPLES, 8);
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

    GLuint position_buffer, face_position_buffer, selection_buffer;

    enum attributes : GLuint {
        position, color, selection
    };

    mesh my_mesh;
    my_mesh.vertex_capacity = 4;
    my_mesh.vertex_count = 4;
    my_mesh.face_capacity = 2;
    my_mesh.face_count = 2;

    my_mesh.vertex_array = create_vertex_array(4, {
        {{
            {position, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &position_buffer},
        {{
            {selection, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0},
        }, 1, GL_STATIC_DRAW, &selection_buffer},
    });

    my_mesh.face_vertex_array = create_vertex_array(6, {
        {{
            {position, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &face_position_buffer}
    });

    my_mesh.vertex_position_buffer = position_buffer;
    my_mesh.face_vertex_position_buffer = face_position_buffer;
    my_mesh.vertex_selection_buffer = selection_buffer;

    my_mesh.vertex_selection.resize(4);

    my_mesh.vertex_positions = {
        {-1, -1, 0},
        {-1, 1, 0},
        {1, -1, 0},
        {1, 1, 0},
    };
    my_mesh.face_vertex_positions = {
        {-1, -1, 0}, {-1, 1, 0}, {1, -1, 0},
        {1, -1, 0}, {-1, 1, 0}, {1, 1, 0},
    };
    my_mesh.vertex_face_vertices = {
        {0, 0}, {1, 1}, {2, 2}, {2, 3}, {1, 4}, {3, 5},
    };

    glBindBuffer(GL_COPY_WRITE_BUFFER, position_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, 4 * 3 * sizeof(float),
        my_mesh.vertex_positions.data()
    );
    glBindBuffer(GL_COPY_WRITE_BUFFER, face_position_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, 6 * 3 * sizeof(float),
        my_mesh.face_vertex_positions.data()
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
        {{"position", position}, {"selection", selection}},
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
    current_context.current_object = new object(
        &my_mesh, point_handle_program.get_name()
    );
    current_context.current_view->view_matrix =
        lookAt(vec3(2, 0, 2), {0, 0, 0}, {0, 0, 1});

    draw_call mesh_draw_call{
        my_mesh.face_vertex_array.get_name(), 0, 6,
        solid_program.get_name(), GL_TRIANGLES
    };


    composition composition;

    pass background_pass;
    pass foreground_pass;

    composition.passes.push_back(background_pass);
    composition.passes.push_back(foreground_pass);

    foreground_pass.renderables.push_back(mesh_draw_call);
    foreground_pass.renderables.push_back(
        current_context.current_object->call
    );


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
