#include "renderer/PostProcessor.hpp"
#include "renderer/GLStateGuard.hpp"
#include <iostream>

PostProcessor::PostProcessor() {}

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

    if (!bloomDownsampleShader.loadFromFiles("assets/shaders/ssao.vert", "assets/shaders/bloom_downsample.frag")) {
        std::cerr << "Failed to load bloom_downsample shader!\n";
        return false;
    }

    if (!bloomUpsampleShader.loadFromFiles("assets/shaders/ssao.vert", "assets/shaders/bloom_upsample.frag")) {
        std::cerr << "Failed to load bloom_upsample shader!\n";
        return false;
    }

    setupQuad();
    setupFramebuffer();
    setupBloom(width, height);
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

    for (auto& mip : bloomMips) {
        if (mip.fbo) glDeleteFramebuffers(1, &mip.fbo);
        if (mip.texture) glDeleteTextures(1, &mip.texture);
    }
    bloomMips.clear();
}

void PostProcessor::setupQuad() {
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

void PostProcessor::renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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

    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: PostProcessor Framebuffer is not complete!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::setupBloom(int width, int height) {
    for (auto& mip : bloomMips) {
        if (mip.fbo) glDeleteFramebuffers(1, &mip.fbo);
        if (mip.texture) glDeleteTextures(1, &mip.texture);
    }
    bloomMips.clear();

    glm::ivec2 mipSize(width / 2, height / 2);
    int levels = std::max(2, std::min(6, bloomMipLevels));

    for (int i = 0; i < levels; ++i) {
        BloomMip mip;
        mip.size = mipSize;

        glGenFramebuffers(1, &mip.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, mip.fbo);

        glGenTextures(1, &mip.texture);
        glBindTexture(GL_TEXTURE_2D, mip.texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mip.size.x, mip.size.y, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "[PostProcessor] Bloom Mip FBO " << i << " incomplete!\n";
        }

        bloomMips.push_back(mip);
        mipSize.x = std::max(1, mipSize.x / 2);
        mipSize.y = std::max(1, mipSize.y / 2);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::renderBloom() {
    if (!bloomEnabled || bloomMips.empty()) return;
    GLStateGuard guard;

    glDisable(GL_DEPTH_TEST);

    // 1. Downsample pass
    bloomDownsampleShader.use();
    bloomDownsampleShader.setFloat("threshold", bloomThreshold);
    bloomDownsampleShader.setFloat("softKnee", bloomSoftKnee);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    bloomDownsampleShader.setInt("srcTexture", 0);

    for (size_t i = 0; i < bloomMips.size(); ++i) {
        const auto& mip = bloomMips[i];
        glViewport(0, 0, mip.size.x, mip.size.y);
        glBindFramebuffer(GL_FRAMEBUFFER, mip.fbo);

        bloomDownsampleShader.setInt("mipLevel", (int)i);
        if (i == 0) {
            bloomDownsampleShader.setVec2("srcResolution", glm::vec2(screenWidth, screenHeight));
            glBindTexture(GL_TEXTURE_2D, colorTexture);
        } else {
            bloomDownsampleShader.setVec2("srcResolution", glm::vec2(bloomMips[i - 1].size));
            glBindTexture(GL_TEXTURE_2D, bloomMips[i - 1].texture);
        }

        renderQuad();
    }

    // 2. Progressive Upsample pass with Additive Blending
    bloomUpsampleShader.use();
    bloomUpsampleShader.setFloat("filterRadius", bloomFilterRadius);
    bloomUpsampleShader.setInt("srcTexture", 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    for (int i = (int)bloomMips.size() - 1; i > 0; --i) {
        const auto& mip = bloomMips[i];
        const auto& nextMip = bloomMips[i - 1];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        glViewport(0, 0, nextMip.size.x, nextMip.size.y);
        glBindFramebuffer(GL_FRAMEBUFFER, nextMip.fbo);

        renderQuad();
    }

    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::resize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    if (width == screenWidth && height == screenHeight) return;

    screenWidth = width;
    screenHeight = height;
    setupFramebuffer();
    setupBloom(width, height);
}

void PostProcessor::beginRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.015f, 0.015f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessor::endRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::render(float time, float fearFactor) {
    if (bloomEnabled) {
        renderBloom();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
    glDisable(GL_DEPTH_TEST);

    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    shader.setInt("sceneTexture", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, getBloomTexture());
    shader.setInt("bloomTexture", 1);

    shader.setBool("bloomEnabled", bloomEnabled);
    shader.setFloat("bloomIntensity", bloomIntensity);

    shader.setFloat("time", time);
    shader.setFloat("fearFactor", fearFactor);
    shader.setFloat("exposure", exposure);
    shader.setBool("toneMappingEnabled", toneMappingEnabled);
    shader.setBool("gammaCorrectionEnabled", gammaCorrectionEnabled);
    shader.setBool("bypassTonemap", bypassTonemap);
    shader.setFloat("vignetteStrength", vignetteStrength);
    shader.setFloat("vignetteRadius", vignetteRadius);
    shader.setFloat("chromaticAberrationStrength", chromaticAberrationStrength);
    shader.setFloat("filmGrainStrength", filmGrainStrength);

    renderQuad();

    glEnable(GL_DEPTH_TEST);
}
