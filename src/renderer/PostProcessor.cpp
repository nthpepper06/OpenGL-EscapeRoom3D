#include "renderer/PostProcessor.hpp"

PostProcessor::PostProcessor() {
}

PostProcessor::~PostProcessor() {
    cleanup();
}

bool PostProcessor::init(int width, int height, const std::string& vertPath, const std::string& fragPath) {
    screenWidth = width;
    screenHeight = height;

    if (!shader.loadFromFiles(vertPath, fragPath)) {
        std::cerr << "Failed to load PostProcessor shaders!\n";
        return false;
    }

    setupQuad();
    setupFramebuffer();
    return true;
}

void PostProcessor::cleanup() {
    if (quadVAO != 0) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (quadVBO != 0) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
    if (colorTexture != 0) {
        glDeleteTextures(1, &colorTexture);
        colorTexture = 0;
    }
    if (rbo != 0) {
        glDeleteRenderbuffers(1, &rbo);
        rbo = 0;
    }
    if (fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
}

void PostProcessor::setupQuad() {
    // 2D Fullscreen Quad in Normalized Device Coordinates [-1, 1]
    float quadVertices[] = {
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void PostProcessor::setupFramebuffer() {
    if (fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &colorTexture);
        glDeleteRenderbuffers(1, &rbo);
    }

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create HDR color texture attachment (GL_RGBA16F floating-point buffer)
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    // Create depth and stencil renderbuffer attachment
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::resize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    if (width == screenWidth && height == screenHeight) return;

    screenWidth = width;
    screenHeight = height;
    setupFramebuffer();
}

void PostProcessor::beginRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessor::endRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::render(float time, float fearFactor) {
    if (!enabled) {
        // If disabled, just blit or draw normal
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glDisable(GL_DEPTH_TEST);
    shader.use();
    shader.setInt("screenTexture", 0);
    shader.setFloat("time", time);
    shader.setFloat("vignetteStrength", vignetteStrength);
    shader.setFloat("vignetteRadius", vignetteRadius);
    shader.setFloat("chromaticAberrationStrength", chromaticAberrationStrength);
    shader.setFloat("fearFactor", fearFactor);
    shader.setFloat("exposure", exposure);
    shader.setBool("toneMappingEnabled", toneMappingEnabled);
    shader.setBool("gammaCorrectionEnabled", gammaCorrectionEnabled);
    shader.setBool("bypassTonemap", bypassTonemap);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}
