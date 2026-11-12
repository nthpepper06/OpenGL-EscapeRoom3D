#pragma once

#include "core/Common.hpp"
#include "renderer/Shader.hpp"
#include "renderer/Light.hpp"
#include "scene/SceneObject.hpp"

class ShadowPipeline {
public:
    ShadowPipeline();
    ~ShadowPipeline();

    bool init(int spotRes = 1024, int pointRes = 512);
    void resize(int spotRes, int pointRes);
    void cleanup();

    void renderSpotShadow(const SpotLight& spot, const std::shared_ptr<SceneObject>& root);
    void renderPointShadow(const PointLight& point, const std::shared_ptr<SceneObject>& root, bool forceBake = false);

    unsigned int getSpotShadowMap() const { return spotDepthMap; }
    unsigned int getPointShadowCubemap() const { return pointDepthCubemap; }
    const glm::mat4& getSpotLightMatrix() const { return spotLightMatrix; }
    float getPointFarPlane() const { return pointFarPlane; }

    void markPointShadowDirty() { pointShadowDirty = true; }
    bool isPointShadowDirty() const { return pointShadowDirty; }

    // Tunable bias and filter settings
    float spotBiasMin = 0.0005f;
    float spotBiasMax = 0.0035f;
    float spotNormalBias = 0.035f;
    float spotPcfRadius = 1.6f;
    int spotPcfTaps = 8; // 4, 8, 16

    float pointBiasMin = 0.004f;
    float pointBiasMax = 0.015f;
    float pointNormalBias = 0.045f;
    bool pointShadowEnabled = true;

private:
    int spotResolution = 1024;
    int pointResolution = 512;

    unsigned int spotFBO = 0;
    unsigned int spotDepthMap = 0;

    unsigned int pointFBO = 0;
    unsigned int pointDepthCubemap = 0;

    Shader spotDepthShader;
    Shader pointCubeShader;

    glm::mat4 spotLightMatrix{1.0f};
    float pointFarPlane = 25.0f;
    bool pointShadowDirty = true;

    void setupSpotFBO();
    void setupPointFBO();
};
