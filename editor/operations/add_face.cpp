#include "add_face.h"

using namespace glm;

namespace ge1 {

    operation::status add_face::trigger(context& c, double x, double y) {
        auto& m = c.current_object->m;

        if (m->selected_vertices.size() == 3) {
            // TODO: update all objects using this mesh
            c.current_object->face_call.count += 3;

            auto vertex = m->selected_vertices.begin();
            m->face_vertex_positions.push_back(m->vertex_positions[*vertex]);
            m->vertex_face_vertices.insert({*vertex, m->face_count * 3});
            vertex++;
            m->face_vertex_positions.push_back(m->vertex_positions[*vertex]);
            m->vertex_face_vertices.insert({*vertex, m->face_count * 3 + 1});
            vertex++;
            m->face_vertex_positions.push_back(m->vertex_positions[*vertex]);
            m->vertex_face_vertices.insert({*vertex, m->face_count * 3 + 2});

            m->face_count++;

            glBindBuffer(
                GL_COPY_WRITE_BUFFER,
                m->face_vertex_position_buffer.get_name()
            );

            if (m->face_count > m->face_capacity) {
                m->face_capacity *= 2;
                glBufferData(
                    GL_COPY_WRITE_BUFFER,
                    m->face_capacity * 3 * 3 * sizeof(float),
                    m->face_vertex_positions.data(), GL_DYNAMIC_DRAW
                );

            } else {
                glBufferSubData(
                    GL_COPY_WRITE_BUFFER,
                    (m->face_count - 1) * 3 * 3 * sizeof(float),
                    3 * 3 * sizeof(float),
                    m->face_vertex_positions.data() + (m->face_count - 1) * 3
                );

            }
        }

        return status::finished;
    }
}
