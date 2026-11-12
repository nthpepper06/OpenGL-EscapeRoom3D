#pragma once

#include <glad/glad.h>

/**
 * @brief RAII Guard for OpenGL rendering states to prevent state leakage across passes.
 * Automatically saves and restores Viewport, Framebuffer, Cull Face, Depth Mask, Blending,
 * Depth Func, and Clip Distances.
 */
class GLStateGuard {
public:
    GLStateGuard() {
        glGetIntegerv(GL_VIEWPORT, prevViewport);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
        glGetIntegerv(GL_CULL_FACE_MODE, &prevCullMode);
        glGetIntegerv(GL_FRONT_FACE, &prevFrontFace);
        glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &prevDepthMask);
        glGetIntegerv(GL_BLEND_SRC_RGB, &prevBlendSrcRGB);
        glGetIntegerv(GL_BLEND_DST_RGB, &prevBlendDstRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &prevBlendSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &prevBlendDstAlpha);

        prevCullFaceEnabled = glIsEnabled(GL_CULL_FACE);
        prevDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
        prevBlendEnabled = glIsEnabled(GL_BLEND);
        prevClipDist0Enabled = glIsEnabled(GL_CLIP_DISTANCE0);
        prevPolygonOffsetFill = glIsEnabled(GL_POLYGON_OFFSET_FILL);
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
        glDepthFunc(prevDepthFunc);
        glDepthMask(prevDepthMask);
        glBlendFuncSeparate(prevBlendSrcRGB, prevBlendDstRGB, prevBlendSrcAlpha, prevBlendDstAlpha);

        if (prevCullFaceEnabled) glEnable(GL_CULL_FACE);
        else glDisable(GL_CULL_FACE);

        if (prevDepthTestEnabled) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);

        if (prevBlendEnabled) glEnable(GL_BLEND);
        else glDisable(GL_BLEND);

        if (prevClipDist0Enabled) glEnable(GL_CLIP_DISTANCE0);
        else glDisable(GL_CLIP_DISTANCE0);

        if (prevPolygonOffsetFill) glEnable(GL_POLYGON_OFFSET_FILL);
        else glDisable(GL_POLYGON_OFFSET_FILL);

        restored = true;
    }

private:
    GLint prevViewport[4]{0, 0, 0, 0};
    GLint prevFBO = 0;
    GLint prevCullMode = GL_BACK;
    GLint prevFrontFace = GL_CCW;
    GLint prevDepthFunc = GL_LESS;
    GLboolean prevDepthMask = GL_TRUE;
    GLint prevBlendSrcRGB = GL_ONE;
    GLint prevBlendDstRGB = GL_ZERO;
    GLint prevBlendSrcAlpha = GL_ONE;
    GLint prevBlendDstAlpha = GL_ZERO;

    GLboolean prevCullFaceEnabled = GL_TRUE;
    GLboolean prevDepthTestEnabled = GL_TRUE;
    GLboolean prevBlendEnabled = GL_FALSE;
    GLboolean prevClipDist0Enabled = GL_FALSE;
    GLboolean prevPolygonOffsetFill = GL_FALSE;
    bool restored = false;
};
