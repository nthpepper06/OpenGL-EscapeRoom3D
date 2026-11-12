#pragma once

#include <glad/glad.h>

/**
 * @brief RAII Guard for OpenGL rendering states to prevent state leakage across passes.
 * Automatically saves and restores Viewport, Framebuffer, Cull Face, Depth Mask, and Blending.
 */
class GLStateGuard {
public:
    GLStateGuard() {
        glGetIntegerv(GL_VIEWPORT, prevViewport);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
        glGetIntegerv(GL_CULL_FACE_MODE, &prevCullMode);
        glGetIntegerv(GL_FRONT_FACE, &prevFrontFace);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &prevDepthMask);
        prevCullFaceEnabled = glIsEnabled(GL_CULL_FACE);
        prevDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
        prevBlendEnabled = glIsEnabled(GL_BLEND);
    }

    ~GLStateGuard() {
        restore();
    }

    void restore() {
        if (restored) return;

        glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
        glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
        glCullFace(prevCullMode);
        glFrontFace(prevFrontFace);
        glDepthMask(prevDepthMask);

        if (prevCullFaceEnabled) glEnable(GL_CULL_FACE);
        else glDisable(GL_CULL_FACE);

        if (prevDepthTestEnabled) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);

        if (prevBlendEnabled) glEnable(GL_BLEND);
        else glDisable(GL_BLEND);

        restored = true;
    }

private:
    GLint prevViewport[4]{0, 0, 0, 0};
    GLint prevFBO = 0;
    GLint prevCullMode = GL_BACK;
    GLint prevFrontFace = GL_CCW;
    GLboolean prevDepthMask = GL_TRUE;
    GLboolean prevCullFaceEnabled = GL_TRUE;
    GLboolean prevDepthTestEnabled = GL_TRUE;
    GLboolean prevBlendEnabled = GL_FALSE;
    bool restored = false;
};
