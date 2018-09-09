#ifndef DRAW_CALL_H
#define DRAW_CALL_H

#include <GL/glew.h>

#include <boost/intrusive/slist.hpp>

#include "renderable.h"
#include "vertex_array.h"

namespace ge1 {

    struct draw_call : public renderable {
        draw_call(const vertex_array* data, GLuint program, GLenum mode);

        void render() override;

        const vertex_array* data;
        GLuint program;
        GLenum mode;
    };

}

#endif // DRAW_CALL_H
