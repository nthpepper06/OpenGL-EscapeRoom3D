#pragma once

#include "core/Common.hpp"
#include "renderer/Shader.hpp"
#include "scene/SceneObject.hpp"
#include <vector>

class SsaoPipeline {
public:
    SsaoPipeline();
    ~SsaoPipeline();

    bool init(int width, int height);
    void resize(int width, int height);
    void cleanup();

    void renderGeometryPrepass(const glm::mat4& view, const glm::mat4& proj, const std::shared_ptr<SceneObject>& root);
    void renderSSAO(const glm::mat4& proj);

    unsigned int getSsaoTexture() const { return ssaoBlurBuffer; }
    unsigned int getRawSsaoTexture() const { return ssaoColorBuffer; }

    bool enabled = true;
    int kernelSize = 16;
    float radius = 0.45f;
    float bias = 0.025f;
    float power = 1.4f;

private:
    int halfWidth = 640;
    int halfHeight = 360;

    // G-Buffer (View-space Position & Normal)
    unsigned int gBuffer = 0;
    unsigned int gPosition = 0;
    unsigned int gNormal = 0;
    unsigned int gDepthRbo = 0;

    // SSAO calculation FBO
    unsigned int ssaoFBO = 0;
    unsigned int ssaoColorBuffer = 0;

    // Bilateral blur FBO
    unsigned int ssaoBlurFBO = 0;
    unsigned int ssaoBlurBuffer = 0;

    // Noise texture & Kernel samples
    unsigned int noiseTexture = 0;
    std::vector<glm::vec3> ssaoKernel;

    Shader geomShader;
    Shader ssaoShader;
    Shader blurShader;

    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;

    void setupGBuffer();
    void setupSsaoFBOs();
    void setupKernelAndNoise();
    void setupQuad();
    void renderQuad();
};
