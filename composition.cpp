#include "composition.h"

namespace ge1 {

    composition::composition() {

    }

    void composition::render() {
        for (auto& pass : passes) {
            glClear(pass.clear_mask);
            for (auto& renderable : pass.renderables) {
                renderable.render();
            }
        }
    }

}
