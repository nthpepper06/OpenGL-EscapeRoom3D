#pragma once

#include "core/Common.hpp"
#include "renderer/Shader.hpp"

class PostProcessor {
public:
    PostProcessor();
    ~PostProcessor();

    bool init(int width, int height, const std::string& vertPath, const std::string& fragPath);
    void resize(int width, int height);

    void beginRender();
    void endRender();
    void render(float time, float fearFactor);

    // Horror parameters
    bool enabled = true;
    float exposure = 1.0f;
    bool toneMappingEnabled = true;
    bool gammaCorrectionEnabled = true;
    bool bypassTonemap = false;
    float vignetteStrength = 0.50f;
    float vignetteRadius = 0.80f;
    float chromaticAberrationStrength = 0.0035f;

    unsigned int getFBO() const { return fbo; }
    unsigned int getTexture() const { return colorTexture; }

private:
    int screenWidth = 1280;
    int screenHeight = 720;

    unsigned int fbo = 0;
    unsigned int colorTexture = 0;
    unsigned int rbo = 0;

    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;

    Shader shader;

    void setupFramebuffer();
    void setupQuad();
    void cleanup();
};
