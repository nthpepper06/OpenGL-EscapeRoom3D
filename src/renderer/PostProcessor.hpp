#pragma once

#include "core/Common.hpp"
#include "renderer/Shader.hpp"
#include <vector>

struct BloomMip {
    unsigned int fbo = 0;
    unsigned int texture = 0;
    glm::ivec2 size{0};
};

class PostProcessor {
public:
    PostProcessor();
    ~PostProcessor();

    bool init(int width, int height, const std::string& vertPath, const std::string& fragPath);
    void resize(int width, int height);

    void beginRender();
    void endRender();
    void render(float time, float fearFactor);

    // Tone Mapping & Horror controls
    bool enabled = true;
    float exposure = 1.0f;
    bool toneMappingEnabled = true;
    bool gammaCorrectionEnabled = true;
    bool bypassTonemap = false;
    float vignetteStrength = 0.50f;
    float vignetteRadius = 0.80f;
    float chromaticAberrationStrength = 0.0035f;
    float filmGrainStrength = 0.035f;

    // HDR Bloom controls
    bool bloomEnabled = true;
    float bloomThreshold = 1.15f;
    float bloomSoftKnee = 0.5f;
    float bloomIntensity = 0.65f;
    float bloomFilterRadius = 0.005f;
    int bloomMipLevels = 5; // 2..5

    unsigned int getFBO() const { return fbo; }
    unsigned int getTexture() const { return colorTexture; }
    unsigned int getBloomTexture() const { return bloomMips.empty() ? 0 : bloomMips[0].texture; }

private:
    int screenWidth = 1280;
    int screenHeight = 720;

    unsigned int fbo = 0;
    unsigned int colorTexture = 0;
    unsigned int rbo = 0;

    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;

    Shader shader;
    Shader bloomDownsampleShader;
    Shader bloomUpsampleShader;

    std::vector<BloomMip> bloomMips;

    void setupFramebuffer();
    void setupBloom(int width, int height);
    void renderBloom();
    void setupQuad();
    void renderQuad();
    void cleanup();
};
