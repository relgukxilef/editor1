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
#include "editor/operations/add_face.h"
#include "editor/operations/delete_vertices.h"

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
static add_face add_face_operation;
static delete_vertices delete_face_operation;

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

void key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods
) {
    if (current_operation) {
        // TODO

    } else {
        double x, y;
        glfwGetCursorPos(window, &x, &y);

        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_F) {
                current_operation = &add_face_operation;
            } else if (key == GLFW_KEY_DELETE) {
                current_operation = &delete_face_operation;
            }
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


    enum attributes : GLuint {
        position, color, selection
    };

    GLuint grid_positions_buffer, grid_colors_buffer;
    vec3 grid_positions[44], grid_colors[44];
    for (int i = 0; i <= 10; i++) {
        grid_positions[i * 4 + 0] = {i - 5, -5, 0};
        grid_positions[i * 4 + 1] = {i - 5, 5, 0};
        grid_positions[i * 4 + 2] = {-5, i - 5, 0};
        grid_positions[i * 4 + 3] = {5, i - 5, 0};
    }
    for (auto& i : grid_colors) {
        i = {0.5, 0.5, 0.5};
    }
    auto grid_vertex_array = create_vertex_array(44, {
        {{
            {position, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &grid_positions_buffer},
        {{
            {color, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &grid_colors_buffer},
    });

    glBindBuffer(GL_COPY_WRITE_BUFFER, grid_positions_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, sizeof(grid_positions), &grid_positions
    );
    glBindBuffer(GL_COPY_WRITE_BUFFER, grid_colors_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, sizeof(grid_positions), &grid_colors
    );

    GLuint position_buffer, face_position_buffer, selection_buffer;

    mesh my_mesh;

    my_mesh.vertex_array = create_vertex_array(0, {
        {{
            {position, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &position_buffer},
        {{
            {selection, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0},
        }, 1, GL_STATIC_DRAW, &selection_buffer},
    });

    my_mesh.face_vertex_array = create_vertex_array(0, {
        {{
            {position, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &face_position_buffer}
    });

    my_mesh.face_vertex_positions.buffer = face_position_buffer;
    my_mesh.vertex_positions.buffer = position_buffer;
    my_mesh.vertex_selections.buffer = selection_buffer;

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
        &my_mesh, solid_program.get_name(), point_handle_program.get_name()
    );
    current_context.current_view->view_matrix =
        lookAt(vec3(2, 0, 2), {0, 0, 0}, {0, 0, 1});

    draw_call grid_draw_call(
        grid_vertex_array, 0, 44, solid_program.get_name(), GL_LINES
    );


    composition composition;

    pass background_pass;
    pass foreground_pass;

    composition.passes.push_back(background_pass);
    composition.passes.push_back(foreground_pass);

    foreground_pass.renderables.push_back(
        current_context.current_object->face_call
    );
    foreground_pass.renderables.push_back(
        current_context.current_object->vertex_call
    );
    foreground_pass.renderables.push_back(grid_draw_call);


    int width, height;
    glfwGetWindowSize(window, &width, &height);
    window_size_callback(window, width, height);
    glfwSetCursorPosCallback(window, &cursor_position_callback);
    glfwSetMouseButtonCallback(window, &mouse_button_callback);
    glfwSetKeyCallback(window, &key_callback);

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
