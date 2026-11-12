#include "renderer/SsaoPipeline.hpp"
#include "renderer/GLStateGuard.hpp"
#include <random>
#include <iostream>

static float lerpFloat(float a, float b, float f) {
    return a + f * (b - a);
}

SsaoPipeline::SsaoPipeline() {}

SsaoPipeline::~SsaoPipeline() {
    cleanup();
}

bool SsaoPipeline::init(int width, int height) {
    halfWidth = std::max(1, width / 2);
    halfHeight = std::max(1, height / 2);

    if (!geomShader.loadFromFiles("assets/shaders/ssao_geometry.vert", "assets/shaders/ssao_geometry.frag")) {
        std::cerr << "[SsaoPipeline] Failed to load ssao_geometry shaders\n";
        return false;
    }

    if (!ssaoShader.loadFromFiles("assets/shaders/ssao.vert", "assets/shaders/ssao.frag")) {
        std::cerr << "[SsaoPipeline] Failed to load ssao shaders\n";
        return false;
    }

    if (!blurShader.loadFromFiles("assets/shaders/ssao.vert", "assets/shaders/ssao_blur.frag")) {
        std::cerr << "[SsaoPipeline] Failed to load ssao_blur shaders\n";
        return false;
    }

    setupGBuffer();
    setupSsaoFBOs();
    setupKernelAndNoise();
    setupQuad();
    return true;
}

void SsaoPipeline::cleanup() {
    if (gBuffer) {
        glDeleteFramebuffers(1, &gBuffer);
        gBuffer = 0;
    }
    if (gPosition) {
        glDeleteTextures(1, &gPosition);
        gPosition = 0;
    }
    if (gNormal) {
        glDeleteTextures(1, &gNormal);
        gNormal = 0;
    }
    if (gDepthRbo) {
        glDeleteRenderbuffers(1, &gDepthRbo);
        gDepthRbo = 0;
    }
    if (ssaoFBO) {
        glDeleteFramebuffers(1, &ssaoFBO);
        ssaoFBO = 0;
    }
    if (ssaoColorBuffer) {
        glDeleteTextures(1, &ssaoColorBuffer);
        ssaoColorBuffer = 0;
    }
    if (ssaoBlurFBO) {
        glDeleteFramebuffers(1, &ssaoBlurFBO);
        ssaoBlurFBO = 0;
    }
    if (ssaoBlurBuffer) {
        glDeleteTextures(1, &ssaoBlurBuffer);
        ssaoBlurBuffer = 0;
    }
    if (noiseTexture) {
        glDeleteTextures(1, &noiseTexture);
        noiseTexture = 0;
    }
    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
}

void SsaoPipeline::resize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    cleanup();
    init(width, height);
}

void SsaoPipeline::setupGBuffer() {
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Position color buffer (View-space)
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, halfWidth, halfHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // Normal color buffer (View-space)
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, halfWidth, halfHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    // Depth renderbuffer
    glGenRenderbuffers(1, &gDepthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, halfWidth, halfHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepthRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[SsaoPipeline] G-Buffer is incomplete!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SsaoPipeline::setupSsaoFBOs() {
    // 1. Raw SSAO FBO
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, halfWidth, halfHeight, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[SsaoPipeline] Raw SSAO FBO is incomplete!\n";
    }

    // 2. Blurred SSAO FBO
    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

    glGenTextures(1, &ssaoBlurBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, halfWidth, halfHeight, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[SsaoPipeline] Blurred SSAO FBO is incomplete!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SsaoPipeline::setupKernelAndNoise() {
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator(42);

    // 64-sample hemisphere kernel
    ssaoKernel.clear();
    for (int i = 0; i < 64; ++i) {
        glm::vec3 sample(
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) // Hemisphere oriented along +Z
        );
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);

        float scale = float(i) / 64.0f;
        scale = lerpFloat(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // 4x4 tileable rotation noise
    std::vector<glm::vec3> ssaoNoise;
    for (int i = 0; i < 16; ++i) {
        glm::vec3 noise(
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            0.0f // rotate around tangent plane
        );
        ssaoNoise.push_back(noise);
    }

    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SsaoPipeline::setupQuad() {
    float quadVertices[] = {
        // positions   // texCoords
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void SsaoPipeline::renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SsaoPipeline::renderGeometryPrepass(const glm::mat4& view, const glm::mat4& proj, const std::shared_ptr<SceneObject>& root) {
    if (!enabled || !root) return;
    GLStateGuard guard;

    glViewport(0, 0, halfWidth, halfHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    geomShader.use();
    geomShader.setMat4("view", view);
    geomShader.setMat4("projection", proj);

    std::vector<std::shared_ptr<SceneObject>> objects;
    root->collectAll(objects);

    for (const auto& obj : objects) {
        if (!obj || !obj->isVisible || !obj->mesh) continue;
        geomShader.setMat4("model", obj->getWorldMatrix());
        geomShader.setMat3("normalMatrix", obj->getNormalMatrix());
        obj->mesh->draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SsaoPipeline::renderSSAO(const glm::mat4& proj) {
    if (!enabled) return;
    GLStateGuard guard;

    glViewport(0, 0, halfWidth, halfHeight);

    // 1. Generate SSAO texture
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    ssaoShader.use();
    ssaoShader.setMat4("projection", proj);
    ssaoShader.setFloat("radius", radius);
    ssaoShader.setFloat("bias", bias);
    ssaoShader.setFloat("power", power);
    ssaoShader.setInt("kernelSize", std::min(kernelSize, 64));
    ssaoShader.setVec2("noiseScale", glm::vec2(float(halfWidth) / 4.0f, float(halfHeight) / 4.0f));

    for (int i = 0; i < std::min(kernelSize, 64); ++i) {
        ssaoShader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    ssaoShader.setInt("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    ssaoShader.setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    ssaoShader.setInt("texNoise", 2);

    glDisable(GL_DEPTH_TEST);
    renderQuad();

    // 2. Bilateral Blur pass
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    blurShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    blurShader.setInt("ssaoInput", 0);
    blurShader.setVec2("texelSize", glm::vec2(1.0f / float(halfWidth), 1.0f / float(halfHeight)));

    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
}
