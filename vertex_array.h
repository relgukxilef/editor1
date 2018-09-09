#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include <initializer_list>
#include <iterator>

#include <boost/intrusive/slist.hpp>

#include <GL/glew.h>

namespace ge1 {
    struct vertex_attribute;
    struct vertex_attribute_pack;
    struct vertex_array_data;
    struct vertex_buffer;


    struct vertex_attribute : public boost::intrusive::slist_base_hook<> {
        friend vertex_attribute_pack;

        vertex_attribute(
            GLuint index, GLint size, GLenum type, GLboolean normalize,
            GLsizei offset
        );
        vertex_attribute(
            GLint size, GLenum type, GLboolean normalize, GLsizei offset
        );
        vertex_attribute(
            GLint size, GLenum type, GLboolean normalize
        );

        GLuint get_index() const;
        GLint get_size() const;
        GLenum get_type() const;
        GLboolean get_normalize() const;
        GLsizei get_offset() const;

        bool get_auto_index() const;
        bool get_auto_offset() const;
        vertex_attribute_pack* get_pack() const;

        operator GLuint() const;

    protected:
        GLsizei offset;
        GLuint index;
        GLint size;
        GLenum type;
        GLboolean normalize;

        bool auto_index;
        bool auto_offset;

    private:
        vertex_attribute_pack* pack;
    };

    struct vertex_attribute_pack : public boost::intrusive::slist_base_hook<> {
        friend vertex_array_data;

        vertex_attribute_pack(
            GLuint vbo, GLint stride,
            std::initializer_list<vertex_attribute*> attributes
        );
        vertex_attribute_pack(
            GLuint vbo,
            std::initializer_list<vertex_attribute*> attributes
        );

        const boost::intrusive::slist<
            vertex_attribute, boost::intrusive::constant_time_size<false>
        >& get_attributes() const;
        GLuint get_vbo() const;
        GLint get_stride() const;
        unsigned int get_index() const;

    protected:
        boost::intrusive::slist<
            vertex_attribute, boost::intrusive::constant_time_size<false>
        > attributes;
        GLuint vbo;
        GLint stride;

    private:
        unsigned int index;
    };

    struct vertex_array_data {
        vertex_array_data(
            boost::intrusive::slist<vertex_attribute_pack>&& attribute_packs
        );
        vertex_array_data(
            std::initializer_list<vertex_attribute_pack*> attribute_packs
        );

        const boost::intrusive::slist<vertex_attribute_pack>&
        get_attribute_packs() const;

        void copy_from(int first, const vertex_buffer& buffer);
        void copy_to(int first, vertex_buffer& buffer) const;

    protected:
        boost::intrusive::slist<
            vertex_attribute_pack
        > attribute_packs;
    };

    struct vertex_array : public vertex_array_data {
        vertex_array(GLuint vao, GLint first, GLsizei count,
            boost::intrusive::slist<vertex_attribute_pack>&& attribute_packs
        );
        vertex_array(
            GLuint vao, GLint first, GLsizei count,
            std::initializer_list<vertex_attribute_pack*> attribute_packs
        );

        GLuint get_vao() const;
        GLint get_first() const;
        GLsizei get_count() const;

    protected:
        GLuint vao;
        GLint first;
        GLsizei count;
    };


    inline vertex_attribute::vertex_attribute(
        GLuint index, GLint size, GLenum type, GLboolean normalize,
        GLsizei offset
    ) :
        offset(offset), index(index),
        size(size), type(type), normalize(normalize),
        auto_index(false), auto_offset(false)
    {}

    inline vertex_attribute::vertex_attribute(
        GLint size, GLenum type, GLboolean normalize, GLsizei offset
    ) :
        offset(offset), index(0),
        size(size), type(type), normalize(normalize),
        auto_index(true), auto_offset(false)
    {}

    inline vertex_attribute::vertex_attribute(
        GLint size, GLenum type, GLboolean normalize
    ) :
        offset(0), index(0),
        size(size), type(type), normalize(normalize),
        auto_index(true), auto_offset(true)
    {}

    inline GLuint vertex_attribute::get_index() const {
        return index;
    }

    inline GLint vertex_attribute::get_size() const {
        return size;
    }

    inline GLenum vertex_attribute::get_type() const {
        return type;
    }

    inline GLboolean vertex_attribute::get_normalize() const {
        return normalize;
    }

    inline GLsizei vertex_attribute::get_offset() const {
        return offset;
    }

    inline bool vertex_attribute::get_auto_index() const {
        return auto_index;
    }

    inline bool vertex_attribute::get_auto_offset() const {
        return auto_offset;
    }

    inline vertex_attribute_pack* vertex_attribute::get_pack() const {
        return pack;
    }

    inline ge1::vertex_attribute::operator GLuint() const {
        return get_index();
    }

    inline const boost::intrusive::slist<
        vertex_attribute, boost::intrusive::constant_time_size<false>
    >& vertex_attribute_pack::get_attributes() const {
        return attributes;
    }

    inline GLuint vertex_attribute_pack::get_vbo() const {
        return vbo;
    }

    inline GLint vertex_attribute_pack::get_stride() const {
        return stride;
    }

    inline unsigned int vertex_attribute_pack::get_index() const {
        return index;
    }

    inline const boost::intrusive::slist<vertex_attribute_pack>&
    vertex_array_data::get_attribute_packs() const {
        return attribute_packs;
    }

    inline GLuint vertex_array::get_vao() const {
        return vao;
    }

    inline GLint vertex_array::get_first() const {
        return first;
    }

    inline GLsizei vertex_array::get_count() const {
        return count;
    }

}

#endif // VERTEX_ARRAY_H
