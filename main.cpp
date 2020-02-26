#include <iostream>
#include <memory>
#include <unordered_map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ge1/composition.h"
#include "ge1/resources.h"
#include "ge1/draw_call.h"
#include "ge1/program.h"
#include "editor/operation.h"
#include "editor/operations/drag_vertex.h"
#include "editor/operations/add_vertex.h"
#include "editor/operations/select_vertex.h"
#include "editor/operations/add_face.h"
#include "editor/operations/delete_vertices.h"
#include "editor/operations/rotate_view.h"
#include "editor/operations/pan_view.h"
#include "editor/operations/dolly_view.h"

#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;
using namespace glm;

using namespace ge1;

template<>
struct std::hash<array<int, 4>> {
    auto operator() (const array<int, 4>& key) const {
        std::hash<int> hasher;
        size_t result = 0;
        for(size_t i = 0; i < 4; ++i) {
            result = result * 31 + hasher(key[i]);
        }
        return result;
    }
};

template<>
struct std::hash<pair<unsigned, unsigned>> {
    auto operator() (const pair<int, int>& key) const {
        std::hash<int> hasher;
        return hasher(key.first) * 31 + hasher(key.second);
    }
};

template<class T>
void buffer_sub_data(GLuint buffer, vector<T> data) {
    glBindBuffer(GL_COPY_WRITE_BUFFER, buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0,
        data.size() * sizeof(T), data.data()
    );
}

template<class T>
void buffer_sub_data(GLuint buffer, span<T> data) {
    glBindBuffer(GL_COPY_WRITE_BUFFER, buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0,
        data.size() * sizeof(T), data.begin()
    );
}

template<class T>
GLuint create_buffer(GLenum target, GLenum usage, span<T> data) {
    GLuint name;
    glGenBuffers(1, &name);
    glBindBuffer(target, name);
    glBufferData(target, data.size() * sizeof(T), data.begin(), usage);
    return name;
}

template<class T>
GLuint create_buffer_size(GLenum target, GLenum usage, unsigned size) {
    GLuint name;
    glGenBuffers(1, &name);
    glBindBuffer(target, name);
    glBufferData(target, size * sizeof(T), nullptr, usage);
    return name;
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

static unique_glfw glfw;

static operation* current_operation = nullptr;
static context current_context;

static drag_vertex drag_vertex_operation;
static add_vertex add_vertex_operation;
static select_vertex select_vertex_operation;
static add_face add_face_operation;
static delete_vertices delete_face_operation;
static rotate_view rotate_view_operation;
static pan_view pan_view_operation;
static dolly_view dolly_view_operation;

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
        } else if (modifiers & GLFW_MOD_ALT) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                current_operation = &rotate_view_operation;
            } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
                current_operation = &pan_view_operation;
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                current_operation = &dolly_view_operation;
            }
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
        radians(60.0f), static_cast<float>(width) / height, 0.1f, 100.0f
    );

    glBindBuffer(
        GL_COPY_WRITE_BUFFER,
        current_context.current_view->view_properties_buffer
    );
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
    glfwWindowHint(GLFW_ALPHA_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwSwapInterval(1);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    int screen_width = 1270, screen_height = 710;

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

    current_context.current_view = new view();

    enum uniform_block_bindings : GLuint {
        view_properties
    };

    glGenBuffers(
        1, &current_context.current_view->view_properties_buffer
    );
    glBindBuffer(
        GL_COPY_WRITE_BUFFER,
        current_context.current_view->view_properties_buffer
    );
    glBufferData(
        GL_COPY_WRITE_BUFFER, 16 * sizeof(float), nullptr, GL_DYNAMIC_DRAW
    );


    enum attributes : GLuint {
        position, color, selection, normal, texture_coordinates,
        texture_unit, texture_slice
    };

    GLuint widget_positions_buffer, widget_colors_buffer;
    GLuint widget_draw_indirect_buffer;

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

    GLuint grid_mesh;

    auto widget_vertex_array = create_vertex_array(
        {
            {{
                {position, 3, GL_FLOAT, GL_FALSE, 0},
            }, 3 * sizeof(float), GL_STATIC_DRAW, &widget_positions_buffer},
            {{
                {color, 3, GL_FLOAT, GL_FALSE, 0},
            }, 3 * sizeof(float), GL_STATIC_DRAW, &widget_colors_buffer},
        }, {
        }, {
            {44, 1, &grid_mesh, nullptr}
        },
        &widget_draw_indirect_buffer
    );

    draw_arrays_indirect_command widget_commands[] {
        {44, 1, grid_mesh, 0}
    };

    // TODO: create helper function for uploading static geometry
    glBindBuffer(GL_COPY_WRITE_BUFFER, widget_positions_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER,
        grid_mesh * sizeof(grid_positions),
        sizeof(grid_positions), &grid_positions
    );
    glBindBuffer(GL_COPY_WRITE_BUFFER, widget_colors_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER,
        grid_mesh * sizeof(grid_colors),
        sizeof(grid_colors), &grid_colors
    );
    glBindBuffer(GL_COPY_WRITE_BUFFER, widget_draw_indirect_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, sizeof(widget_commands), &widget_commands
    );

    GLuint position_buffer, face_position_buffer, selection_buffer;
    GLuint edge_position_buffer;

    mesh my_mesh;

    my_mesh.vertex_array = create_vertex_array(0, {
        {{
            {position, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_DYNAMIC_DRAW, &position_buffer},
        {{
            {selection, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0},
        }, 1, GL_DYNAMIC_DRAW, &selection_buffer},
    });

    my_mesh.edge_vertex_array = create_vertex_array(0, {
        {{
            {position, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_DYNAMIC_DRAW, &edge_position_buffer}
    });

    my_mesh.face_vertex_array = create_vertex_array(0, {
        {{
            {position, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_DYNAMIC_DRAW, &face_position_buffer}
    });

    my_mesh.face_vertex_positions.buffer = face_position_buffer;
    my_mesh.edge_vertex_positions.buffer = edge_position_buffer;
    my_mesh.vertex_positions.buffer = position_buffer;
    my_mesh.vertex_selections.buffer = selection_buffer;

    unique_shader fragment_utils = compile_shader(
        GL_FRAGMENT_SHADER, "shaders/utils.fs"
    );

    GLuint solid_model_matrix, point_handle_model_matrix;
    GLuint edge_handle_model_matrix;

    unique_program solid_program = compile_program(
        "shaders/solid.vs", nullptr, nullptr, nullptr, "shaders/solid.fs",
        {fragment_utils.get_name()},
        {{"position", position}, {"color", color}},
        {
            {"model", &solid_model_matrix}
        }, {
            {"view_properties", view_properties},
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
            {"view_properties", view_properties},
        }
    );

    unique_program edge_handle_program = compile_program(
        "shaders/edge_handle.vs", nullptr, nullptr, nullptr,
        "shaders/edge_handle.fs",
        {fragment_utils.get_name()},
        {{"position", position},},
        {
            {"model", &edge_handle_model_matrix}
        }, {
            {"view_properties", view_properties},
        }
    );

    glUseProgram(solid_program.get_name());
    glUniformMatrix4fv(solid_model_matrix, 1, GL_FALSE, value_ptr(mat4(1)));

    glUseProgram(point_handle_program.get_name());
    glUniformMatrix4fv(
        point_handle_model_matrix, 1, GL_FALSE, value_ptr(mat4(1))
    );

    glUseProgram(edge_handle_program.get_name());
    glUniformMatrix4fv(
        edge_handle_model_matrix, 1, GL_FALSE, value_ptr(mat4(1))
    );

    glEnable(GL_POINT_SPRITE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glLineWidth(4);

    glBindBufferRange(
        GL_UNIFORM_BUFFER, view_properties,
        current_context.current_view->view_properties_buffer,
        0, 16 * sizeof(float)
    );

    current_context.current_object = new object(
        &my_mesh, solid_program.get_name(), edge_handle_program.get_name(),
        point_handle_program.get_name()
    );
    current_context.current_view->view_matrix =
        lookAt(vec3(-10, 0, 2), {0, 0, 2}, {0, 0, 1});

    multi_draw_arrays_indirect_call grid_draw_call(
        widget_vertex_array, nullptr, 1, solid_program.get_name(), GL_LINES
    );


    tinyobj::attrib_t attributes;
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;

    string error;

    bool success = LoadObj(
        &attributes, &shapes, &materials, &error, "models/sponza.obj", "models"
    );

    if (!success) {
        throw runtime_error(error);
    }

    struct texture_array_index {
        unsigned unit, slice;
    };
    struct texture_array {
        unsigned width, height;
        vector<unique_ptr<unsigned char[]>> data;
    };
    vector<texture_array> texture_data;
    unordered_map<string, texture_array_index> texture_indices;
    map<pair<unsigned, unsigned>, unsigned> resolution_textures;
    vector<texture_array_index> material_textures;
    material_textures.reserve(materials.size());

    stbi_set_flip_vertically_on_load(true);

    for (const auto& material : materials) {
        if (!material.diffuse_texname.empty()) {
            auto i = texture_indices.find(material.diffuse_texname);
            if (i == texture_indices.end()) {
                int width, height, channel;
                auto image = stbi_load(
                    ("models\\" + material.diffuse_texname).c_str(),
                    &width, &height, &channel, 4
                );
                assert(image);

                auto resolution = resolution_textures.find({width, height});
                if (resolution == resolution_textures.end()) {
                    resolution = resolution_textures.insert(
                        {{width, height}, texture_data.size()}
                    ).first;
                    texture_data.push_back({
                        static_cast<unsigned>(width),
                        static_cast<unsigned>(height), {}
                    });
                }

                auto& slices = texture_data[resolution->second].data;
                i = texture_indices.insert({
                    material.diffuse_texname,
                    {
                        resolution->second,
                        static_cast<unsigned>(slices.size())
                    }
                }).first;

                slices.push_back(unique_ptr<unsigned char[]>(image));
            }
            material_textures.push_back(i->second);
        } else {
            material_textures.push_back({0, 0});
        }
    }

    vector<unsigned> textures;
    textures.resize(texture_data.size());
    glGenTextures(static_cast<signed>(textures.size()), textures.data());

    for (auto texture : textures) {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(
            GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
        );
        glTexParameteri(
            GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR
        );
        glTexParameteri(
            GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY, 16
        );
    }

    {
        auto i = 0u;
        for (auto& data : texture_data) {
            glBindTexture(GL_TEXTURE_2D_ARRAY, textures[i]);
            auto width = data.width, height = data.height;
            glTexStorage3D(
                GL_TEXTURE_2D_ARRAY,
                static_cast<int>(std::log2(std::max(width, height))),
                GL_RGB8, width, height, data.data.size()
            );
            /*glTexImage3D(
                GL_TEXTURE_2D_ARRAY, 0, GL_RGB5_A1,
                width, height, data.data.size(),
                0,  GL_RGBA,  GL_UNSIGNED_BYTE, nullptr
            );*/
            auto j = 0u;
            for (auto& texture : data.data) {
                glTexSubImage3D(
                    GL_TEXTURE_2D_ARRAY, 0, 0, 0, j, width, height, 1,
                    GL_RGBA, GL_UNSIGNED_BYTE, texture.get()
                );
                j++;
            }
            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
            i++;
        }
    }

    auto face_vertex_count = 0u;
    for (const auto& shape : shapes) {
        face_vertex_count += shape.mesh.indices.size();
    }

    vector<float> obj_positions, obj_normals, obj_texcoords;
    vector<unsigned> obj_faces;
    unordered_map<array<int, 4>, unsigned> face_vertex_indices;
    vector<unsigned char> obj_texture_indices;
    obj_texture_indices.reserve(face_vertex_count * 2);

    // edge info
    unsigned edge_count = 0;
    unordered_map<pair<unsigned, unsigned>, unsigned> vertex_edge;
    vector<unsigned> edge_vertices;
    vector<unsigned> edge_neighbours;

    for (const auto& shape : shapes) {
        // obj vertex indices are only unique per shape
        map<int, unsigned> vertex_indices; // obj.vertex_index -> gpu index

        auto vertex = shape.mesh.indices.begin();
        auto material = shape.mesh.material_ids.begin();
        while (vertex != shape.mesh.indices.end()) {
            for (auto i = 0u; i < 3; i++) {
                // split up vertices

                // translate vertex index
                auto vertex_index =
                    vertex_indices.find(vertex->vertex_index);
                if (vertex_index == vertex_indices.end()) {
                    vertex_index = vertex_indices.insert(
                        vertex_index,
                        {vertex->vertex_index, face_vertex_indices.size()}
                    );
                }

                // translate face_vertex index
                array<int, 4> indices = {
                    vertex->vertex_index, vertex->normal_index,
                    vertex->texcoord_index, *material
                };
                auto face_vertex_index = face_vertex_indices.find(indices);
                if (face_vertex_index == face_vertex_indices.end()) {
                    face_vertex_index = face_vertex_indices.insert(
                        face_vertex_index, {indices, face_vertex_indices.size()}
                    );

                    for (auto i = 0u; i < 3; i++) {
                        obj_positions.push_back(
                            attributes.vertices[
                                static_cast<unsigned>(
                                    vertex->vertex_index
                                ) * 3 + i
                            ]
                        );
                    }
                    for (auto i = 0u; i < 3; i++) {
                        obj_normals.push_back(
                            attributes.normals[
                                static_cast<unsigned>(
                                    vertex->normal_index
                                ) * 3 + i
                            ]
                        );
                    }
                    for (auto i = 0u; i < 2; i++) {
                        obj_texcoords.push_back(
                            attributes.texcoords[
                                static_cast<unsigned>(
                                    std::max(vertex->texcoord_index, 0)
                                ) * 2 + i
                            ]
                        );
                    }
                    auto texture_index =
                        material_textures[static_cast<unsigned>(*material)];
                    obj_texture_indices.push_back(
                        static_cast<unsigned char>(texture_index.unit)
                    );
                    obj_texture_indices.push_back(
                        static_cast<unsigned char>(texture_index.slice)
                    );
                }

                obj_faces.push_back(face_vertex_index->second);

                vertex++;
            }

            auto face_vertex = vertex - 3;
            unsigned vertex_index = vertex_indices[face_vertex->vertex_index];
            unsigned next_vertex = vertex_indices[face_vertex[1].vertex_index];
            unsigned last_vertex = vertex_indices[face_vertex[2].vertex_index];

            for (auto i = 0u; i < 3; i++) {

                // write edges
                // TODO: don't use GPU vertices as those are split up
                // by normals and texture coordinates
                pair<unsigned, unsigned> edge = {
                    last_vertex, vertex_index
                };
                if (edge.first > edge.second) {
                    swap(edge.first, edge.second);
                }
                auto edge_index = vertex_edge.find(edge);
                if (edge_index == vertex_edge.end()) {
                    edge_index = vertex_edge.insert(
                        edge_index, {edge, edge_count}
                    );
                    edge_count++;

                    edge_neighbours.push_back(
                        static_cast<unsigned>(next_vertex)
                    );
                    edge_neighbours.push_back(-1u);
                    edge_vertices.push_back(
                        static_cast<unsigned>(last_vertex)
                    );
                    edge_vertices.push_back(
                        static_cast<unsigned>(vertex_index)
                    );
                } else {
                    edge_neighbours[edge_index->second * 2 + 1] =
                        static_cast<unsigned>(next_vertex);
                    vertex_edge.erase(edge_index);
                }

                swap(next_vertex, last_vertex);
                swap(last_vertex, vertex_index);
            }

            material++;
        }
    }

    // remove open edges
    for (auto edge : vertex_edge) {
        edge_vertices[edge.second * 2 + 1] = edge_vertices.back();
        edge_vertices.pop_back();
        edge_vertices[edge.second * 2] = edge_vertices.back();
        edge_vertices.pop_back();
        edge_neighbours[edge.second * 2 + 1] = edge_neighbours.back();
        edge_neighbours.pop_back();
        edge_neighbours[edge.second * 2] = edge_neighbours.back();
        edge_neighbours.pop_back();
    }

    // TODO:
    // identify connected edges

    unsigned vertex_count = static_cast<unsigned>(face_vertex_indices.size());

    unsigned obj_position_buffer, obj_normal_buffer, obj_texcoord_buffer;
    unsigned obj_texture_indices_buffer, obj_face_buffer;
    // TODO: simplify vertex array creation
    auto obj_vertex_array = create_vertex_array(
        vertex_count,
        {
            {{
                {position, 3, GL_FLOAT, GL_FALSE, 0},
            }, 3 * sizeof(float), GL_STATIC_DRAW, &obj_position_buffer},
            {{
                {normal, 3, GL_FLOAT, GL_FALSE, 0},
            }, 3 * sizeof(float), GL_STATIC_DRAW, &obj_normal_buffer},
            {{
                {texture_coordinates, 2, GL_FLOAT, GL_FALSE, 0},
            }, 2 * sizeof(float), GL_STATIC_DRAW, &obj_texcoord_buffer},
            {{
                {texture_unit, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0},
                {texture_slice, 1, GL_UNSIGNED_BYTE, GL_FALSE, 1},
            }, 2 * sizeof(char), GL_STATIC_DRAW, &obj_texture_indices_buffer}
        },
        face_vertex_count, &obj_face_buffer
    );

    buffer_sub_data(obj_position_buffer, obj_positions);
    buffer_sub_data(obj_normal_buffer, obj_normals);
    buffer_sub_data(obj_texcoord_buffer, obj_texcoords);
    buffer_sub_data(obj_texture_indices_buffer, obj_texture_indices);
    buffer_sub_data(obj_face_buffer, obj_faces);

    GLuint edge_vertices_buffer = create_buffer(
        GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW,
        span<const unsigned>{&*edge_vertices.begin(), &*edge_vertices.end()}
    );
    GLuint edge_neighbours_buffer = create_buffer(
        GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW,
        span<const unsigned>{&*edge_neighbours.begin(), &*edge_neighbours.end()}
    );
    GLuint outline_faces_buffer = create_buffer_size<unsigned>(
        GL_ARRAY_BUFFER, GL_STREAM_COPY,
        static_cast<unsigned>(edge_vertices.size())
    );
    GLuint counter_buffer = create_buffer<const unsigned>(
        GL_SHADER_STORAGE_BUFFER, GL_STREAM_READ, {0}
    );
    // linked list of vertices in outline
    GLuint vertex_outline_neighbours_buffer = create_buffer_size<unsigned>(
        GL_SHADER_STORAGE_BUFFER, GL_STREAM_COPY, vertex_count * 2
    );

    auto outline_vertex_array = create_vertex_array(
        {
            {
                obj_position_buffer, position, 3, GL_FLOAT, GL_FALSE,
                3 * sizeof(float), 0
            }
        },
        outline_faces_buffer
    );

    GLuint outline_matrix;
    auto outline_program = compile_program("shaders/outline.cs");
    get_uniform_locations(outline_program,  {
        {"model_view_projection_matrix", &outline_matrix}
    });

    bind_buffer_bases(GL_SHADER_STORAGE_BUFFER, {
        {0, edge_vertices_buffer},
        {1, edge_neighbours_buffer},
        {2, obj_position_buffer},
        {3, outline_faces_buffer},
        {4, counter_buffer},
        {5, vertex_outline_neighbours_buffer},
    });

    // TODO: draw outlines using geometry shader

    GLuint obj_model_matrix, obj_textures[4];
    auto obj_program = compile_program(
        "shaders/obj.vs", nullptr, nullptr, nullptr, "shaders/obj.fs",
        {},
        {
            {"position", position}, {"normal", normal},
            {"texture_coordinates", texture_coordinates},
            {"unit", texture_unit},
            {"slice", texture_slice},
        }
    );
    get_uniform_locations(obj_program, {
        {"model", &obj_model_matrix},
        {"textures[0]", &obj_textures[0]},
        {"textures[1]", &obj_textures[1]},
        {"textures[2]", &obj_textures[2]},
        {"textures[3]", &obj_textures[3]},
    });
    bind_uniform_blocks(obj_program, {
        {"view_properties", view_properties}
    });

    mat4 model_matrix = mat4(
        .01, 0, 0, 0,
        0, 0, .01, 0,
        0, .01, 0, 0,
        0, 0, 0, 1
    );

    glUseProgram(obj_program);
    glUniformMatrix4fv(
        obj_model_matrix, 1, GL_FALSE, value_ptr(model_matrix)
    );
    for (auto i = 0u; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textures[i]);
        glUniform1i(obj_textures[i], i);
    }

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_CLAMP);

    draw_elements_call obj_draw_call(
        obj_vertex_array, obj_program, GL_TRIANGLES,
        static_cast<int>(face_vertex_count), GL_UNSIGNED_INT, 0
    );

    draw_elements_call outline_draw_call(
        outline_vertex_array, obj_program, GL_LINES,
        0, GL_UNSIGNED_INT, 0
    );

    composition composition;

    pass background_pass;
    pass foreground_pass;

    composition.passes.push_back(background_pass);
    composition.passes.push_back(foreground_pass);

    foreground_pass.renderables.push_back(grid_draw_call);
    foreground_pass.renderables.push_back(obj_draw_call);
    foreground_pass.renderables.push_back(outline_draw_call);
    foreground_pass.renderables.push_back(
        current_context.current_object->face_call
    );
    foreground_pass.renderables.push_back(
        current_context.current_object->edge_call
    );
    foreground_pass.renderables.push_back(
        current_context.current_object->vertex_call
    );

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    window_size_callback(window, width, height);
    glfwSetCursorPosCallback(window, &cursor_position_callback);
    glfwSetMouseButtonCallback(window, &mouse_button_callback);
    glfwSetKeyCallback(window, &key_callback);

    glfwSetWindowSizeCallback(window, &window_size_callback);

    glClearColor(0, 0, 0, 1);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: use clear instead
        buffer_sub_data(counter_buffer, span<const unsigned>{0});

        obj_draw_call.render();

        glUseProgram(outline_program);
        glUniformMatrix4fv(outline_matrix, 1, GL_FALSE, value_ptr(
            current_context.current_view->projection_matrix *
            current_context.current_view->view_matrix * model_matrix
        ));
        glDispatchCompute((edge_vertices.size() / 2 - 1) / 32 + 1, 1, 1);

        // TODO: don't read data back to GPU, use indirect call instead
        GLuint outline_vertices_count;
        glBindBuffer(GL_COPY_READ_BUFFER, counter_buffer);
        glGetBufferSubData(
            GL_COPY_READ_BUFFER, 0, sizeof(GLuint), &outline_vertices_count
        );

        outline_draw_call.count = static_cast<int>(outline_vertices_count * 2);

        outline_draw_call.render();

        //composition.render();

        glfwSwapBuffers(window);

        GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        // wait for drawing to finish. glfwSwapBuffers doesn't ensure that
        GLenum waitReturn = GL_UNSIGNALED;
        while (
            waitReturn != GL_ALREADY_SIGNALED &&
            waitReturn != GL_CONDITION_SATISFIED
        ) {
            waitReturn = glClientWaitSync(
                sync, GL_SYNC_FLUSH_COMMANDS_BIT, 10
            );
        }
        glDeleteSync(sync);

        // some event handlers write to VRAM
        // can't poll before last frame was rendered
        glfwPollEvents();
    }

    return 0;
}
