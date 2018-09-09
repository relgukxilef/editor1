#ifndef PASS_H
#define PASS_H

#include <boost/intrusive/list.hpp>

#include <GL/glew.h>

#include "renderable.h"

namespace ge1 {

    using namespace boost::intrusive;

    struct pass : public list_base_hook<link_mode<auto_unlink>> {
        pass();

        list<renderable, constant_time_size<false>> renderables;
        GLuint uniform_block; // TODO: maybe allow a list?
        GLbitfield clear_mask;
    };

}

#endif // PASS_H
