#include "vertex_array.h"

namespace ge1 {

    GLuint create_vertex_array(
        span<const attribute_pointer_parameter> attributes,
        GLuint element_array_buffer, GLuint draw_indirect_buffer
    ) {
        GLuint name;
        glCreateVertexArrays(1, &name);
        glBindVertexArray(name);

        for (auto attribute : attributes) {
            glBindBuffer(GL_ARRAY_BUFFER, attribute.vertex_buffer);
            glVertexAttribPointer(
                attribute.index, attribute.size, attribute.type,
                attribute.normalized, attribute.stride,
                reinterpret_cast<GLvoid*>(attribute.pointer)
            );
        }

        if (element_array_buffer != 0) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);
        }
        if (draw_indirect_buffer != 0) {
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_indirect_buffer);
        }

        return name;
    }

    GLuint create_vertex_array(
        unsigned int vertex_capacity,
        span<const attribute_pack_parameter> attribute_packs,
        unsigned int draw_indirect_capacity, GLuint *draw_indirect_buffer,
        GLenum draw_indirect_usage
    ) {
        GLuint name;
        glCreateVertexArrays(1, &name);
        glBindVertexArray(name);

        for (auto attribute_pack : attribute_packs) {
            glCreateBuffers(1, attribute_pack.vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, *attribute_pack.vertex_buffer);
            if (vertex_capacity > 0) {
                glBufferData(
                    GL_ARRAY_BUFFER, vertex_capacity * attribute_pack.stride,
                    nullptr, attribute_pack.usage
                );
            }

            for (auto attribute : attribute_pack.attributes) {
                glEnableVertexAttribArray(attribute.index);
                glVertexAttribPointer(
                    attribute.index, attribute.size, attribute.type,
                    attribute.normalized, attribute_pack.stride,
                    reinterpret_cast<GLvoid*>(attribute.offset)
                );
            }
        }

        if (draw_indirect_buffer != nullptr) {
            glCreateBuffers(1, draw_indirect_buffer);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, *draw_indirect_buffer);
            glBufferData(
                GL_ARRAY_BUFFER, draw_indirect_capacity * 4 * 4,
                nullptr, draw_indirect_usage
            );
        }

        return name;
    }

    GLuint create_vertex_array(
        span<const attribute_pack_parameter> vertex_attribute_packs,
        span<const attribute_pack_parameter> instance_attribute_packs,
        span<const command_parameter> commands, GLuint* draw_indirect_buffer,
        GLenum draw_indirect_usage
    ) {
        GLuint name;
        glCreateVertexArrays(1, &name);
        glBindVertexArray(name);

        GLuint vertex_capacity = 0, instance_capacity = 0;

        for (auto command : commands) {
            if (command.mesh) {
                *command.mesh = vertex_capacity;
            }
            if (command.instances) {
                *command.instances = instance_capacity;
            }
            vertex_capacity += command.vertex_capacity;
            instance_capacity += command.instance_capacity;
        }

        for (auto attribute_pack : vertex_attribute_packs) {
            glCreateBuffers(1, attribute_pack.vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, *attribute_pack.vertex_buffer);
            glBufferData(
                GL_ARRAY_BUFFER, vertex_capacity * attribute_pack.stride,
                nullptr, attribute_pack.usage
            );

            for (auto attribute : attribute_pack.attributes) {
                glEnableVertexAttribArray(attribute.index);
                glVertexAttribPointer(
                    attribute.index, attribute.size, attribute.type,
                    attribute.normalized, attribute_pack.stride,
                    reinterpret_cast<GLvoid*>(attribute.offset)
                );
            }
        }

        for (auto attribute_pack : instance_attribute_packs) {
            glCreateBuffers(1, attribute_pack.vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, *attribute_pack.vertex_buffer);
            glBufferData(
                GL_ARRAY_BUFFER, instance_capacity * attribute_pack.stride,
                nullptr, attribute_pack.usage
            );

            for (auto attribute : attribute_pack.attributes) {
                glEnableVertexAttribArray(attribute.index);
                glVertexAttribPointer(
                    attribute.index, attribute.size, attribute.type,
                    attribute.normalized, attribute_pack.stride,
                    reinterpret_cast<GLvoid*>(attribute.offset)
                );
                glVertexAttribDivisor(attribute.index, 1);
            }
        }

        if (draw_indirect_buffer != nullptr) {
            glCreateBuffers(1, draw_indirect_buffer);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, *draw_indirect_buffer);
            glBufferData(
                GL_DRAW_INDIRECT_BUFFER, commands.size() * 4 * sizeof(GLuint),
                nullptr, draw_indirect_usage
            );
        }

        return name;
    }
}
