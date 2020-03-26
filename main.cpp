#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <openvr/openvr_mingw.hpp>

#include "ge1/composition.h"
#include "ge1/resources.h"
#include "ge1/draw_call.h"
#include "ge1/program.h"
#include "typed/glm_types.h"

using namespace std;
using namespace glm;

using namespace ge1;

void window_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
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

mat4x3 pose_to_mat(vr::HmdMatrix34_t pose) {
    auto matrix = transpose(make_mat3x4(reinterpret_cast<float*>(pose.m)));
    return matrix;
}

mat4x4 pose_to_mat(vr::HmdMatrix44_t pose) {
    auto matrix = transpose(make_mat4(reinterpret_cast<float*>(pose.m)));
    return matrix;
}

int main() {
    unique_glfw glfw;

    GLFWwindow* window;

    glfwWindowHint(GLFW_SAMPLES, 16);
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


    vr::EVRInitError error = vr::VRInitError_None;
    vr::IVRSystem *vr_system = vr::VR_Init(&error, vr::VRApplication_Scene);

    if (error != vr::VRInitError_None) {
        auto message = vr::VR_GetVRInitErrorAsEnglishDescription(error);
        throw new runtime_error(message);
    }

    vr::TrackedDevicePose_t tracked_device_poses[vr::k_unMaxTrackedDeviceCount];


    GLuint shape_buffer, color_buffer;

    enum attributes : GLuint {
        position, color
    };

    unique_vertex_array mesh = create_vertex_array(4, {
        {{
            {position, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &shape_buffer},
        {{
            {color, 3, GL_FLOAT, GL_FALSE, 0},
        }, 3 * sizeof(float), GL_STATIC_DRAW, &color_buffer},
    });

    float positions[] = {
        -1, 0, -1,
        -1, 0, 1,
        1, 0, -1,
        1, 0, 1
    };
    float colors[] = {
        1, 0, 1,
        0, 1, 1,
        1, 1, 0,
        1, 1, 1,
    };

    glBindBuffer(GL_COPY_WRITE_BUFFER, shape_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, 4 * 3 * sizeof(float), positions
    );
    glBindBuffer(GL_COPY_WRITE_BUFFER, color_buffer);
    glBufferSubData(
        GL_COPY_WRITE_BUFFER, 0, 4 * 3 * sizeof(float), colors
    );

    unique_shader fragment_utils = compile_shader(
        GL_FRAGMENT_SHADER, "shaders/utils.fs"
    );

    unique_program solid = compile_program(
        "shaders/solid.vs", nullptr, nullptr, nullptr, "shaders/solid.fs",
        {fragment_utils.get_name()},
        {{"position", position}, {"color", color}}
    );

    GLuint model_view_projection_matrix_uniform;

    get_uniform_locations(
        solid.get_name(), {
            {
                "model_view_projection_matrix",
                &model_view_projection_matrix_uniform
            }
        }
    );


    draw_arrays_call mesh_draw_call{
        mesh.get_name(), 0, 4, solid.get_name(), GL_TRIANGLE_STRIP
    };


    composition composition;

    pass background_pass;
    pass foreground_pass;

    composition.passes.push_back(background_pass);
    composition.passes.push_back(foreground_pass);

    foreground_pass.renderables.push_back(mesh_draw_call);


    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetWindowSizeCallback(window, &window_size_callback);


    unsigned vr_width, vr_height;
    vr_system->GetRecommendedRenderTargetSize(&vr_width, &vr_height);

    GLuint framebuffers[4], renderbuffers[2], frame_textures[4];
    glGenFramebuffers(4, framebuffers);
    glGenRenderbuffers(2, renderbuffers);
    glGenTextures(4, frame_textures);

    for (auto i = 0u; i < 2; i++) {
        // render buffers
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers[i]);
        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER, 8, GL_DEPTH_COMPONENT, vr_width, vr_height
        );
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, renderbuffers[i]
        );
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, frame_textures[i]);
        glTexImage2DMultisample(
            GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB8, vr_width, vr_height, true
        );
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D_MULTISAMPLE, frame_textures[i], 0
        );

        if (
            glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
            GL_FRAMEBUFFER_COMPLETE
        ) {
            throw runtime_error("incomplete framebuffer");
        }

        // multisample resolving buffers
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i + 2]);
        glBindTexture(GL_TEXTURE_2D, frame_textures[i + 2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB8,
            static_cast<GLsizei>(vr_width),
            static_cast<GLsizei>(vr_height),
            0, GL_RGB, GL_UNSIGNED_BYTE, nullptr
        );
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, frame_textures[i + 2], 0
        );

        if (
            glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
            GL_FRAMEBUFFER_COMPLETE
        ) {
            throw runtime_error("incomplete framebuffer");
        }
    }

    vr::EVREye eyes[2] = {vr::Eye_Left, vr::Eye_Right};
    vr::Texture_t vr_frame_textures[2];
    mat4 projection_matrices[2];

    for (auto i = 0u; i < 2; i++) {
        // TODO: matrices might update
        projection_matrices[i] =
            pose_to_mat(vr_system->GetProjectionMatrix(eyes[i], 0.01f, 10.f)) *
            inverse(mat4(pose_to_mat(vr_system->GetEyeToHeadTransform(eyes[i]))));
        vr_frame_textures[i] = {
            reinterpret_cast<void*>(frame_textures[i + 2]),
            vr::TextureType_OpenGL, vr::ColorSpace_Gamma
        };
    }


    glEnable(GL_MULTISAMPLE);

    while (!glfwWindowShouldClose(window)) {
        vr::VRCompositor()->WaitGetPoses(
            tracked_device_poses, vr::k_unMaxTrackedDeviceCount,
            nullptr, 0
        );

        mat4x3 headset_pose;
        for (auto i = 0u; i < vr::k_unMaxTrackedDeviceCount; i++) {
            if (
                vr_system->GetTrackedDeviceClass(i) ==
                vr::TrackedDeviceClass::TrackedDeviceClass_HMD
            ) {
                auto pose = tracked_device_poses[i].mDeviceToAbsoluteTracking;
                headset_pose = pose_to_mat(pose);
                break;
            }
        }


        glViewport(
            0, 0,
            static_cast<GLsizei>(vr_width), static_cast<GLsizei>(vr_height)
        );

        for (auto i = 0u; i < 2; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            mat4 projection_matrix =
                projection_matrices[i] * inverse(mat4(headset_pose));

            glUniformMatrix4fv(
                model_view_projection_matrix_uniform, 1, GL_FALSE,
                value_ptr(projection_matrix)
            );

            composition.render();

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[i + 2]);
            glBlitFramebuffer(
                0, 0, vr_width, vr_height,
                0, 0, vr_width, vr_height,
                GL_COLOR_BUFFER_BIT, GL_LINEAR
            );

            vr::EVRCompositorError error =
                vr::VRCompositor()->Submit(eyes[i], &vr_frame_textures[i]);
            if (error != vr::VRCompositorError_None) {
                throw runtime_error("compositor error");
            }
        }

        glfwPollEvents();

        vr::VREvent_t event;
        while (vr_system->PollNextEvent(&event, sizeof(event))) {

        }
    }

    vr::VR_Shutdown();

    return 0;
}
