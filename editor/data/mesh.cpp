#include "mesh.h"

using namespace glm;

namespace ge1 {

    bool mesh::pick_vertex(mat4 matrix, vec2 ndc, unsigned int& vertex_index) {
        unsigned int i = 0;
        for (auto& vertex : vertex_positions) {
            vec4 vertex_ndc = matrix * vec4(vertex, 1);
            vertex_ndc /= vertex_ndc.w;
            // TODO: respect aspect ratio
            if (length(vec2(vertex_ndc) - ndc) < 0.1f) {
                vertex_index = i;
                return true;
            }
            i++;
        }

        return false;
    }

    void mesh::resize_vertex_buffer(unsigned int size) {
        if (size <= vertex_capacity) {
            return;
        }

        vertex_capacity = size;

        glBindBuffer(
            GL_COPY_WRITE_BUFFER,
            vertex_position_buffer.get_name()
        );
        glBufferData(
            GL_COPY_WRITE_BUFFER,
            vertex_capacity * 3 * sizeof(float),
            vertex_positions.data(), GL_DYNAMIC_DRAW
        );

        glBindBuffer(
            GL_COPY_WRITE_BUFFER,
            vertex_selection_buffer.get_name()
        );
        glBufferData(
            GL_COPY_WRITE_BUFFER,
            vertex_capacity,
            vertex_selection.data(), GL_DYNAMIC_DRAW
        );
    }

    void mesh::add_vertex(vec3 position) {
        vertex_positions.push_back(position);
        vertex_selection.push_back(false);

        vertex_count++;

        if (vertex_count > vertex_capacity) {
            resize_vertex_buffer(max(vertex_capacity * 2, vertex_count));

        } else {
            glBindBuffer(
                GL_COPY_WRITE_BUFFER,
                vertex_position_buffer.get_name()
            );
            glBufferSubData(
                GL_COPY_WRITE_BUFFER,
                (vertex_count - 1) * 3 * sizeof(float),
                3 * sizeof(float), &position
            );

            bool selected = false;
            glBindBuffer(
                GL_COPY_WRITE_BUFFER,
                vertex_selection_buffer.get_name()
            );
            glBufferSubData(
                GL_COPY_WRITE_BUFFER,
                vertex_count - 1, 1, &selected
            );
        }
    }
}
