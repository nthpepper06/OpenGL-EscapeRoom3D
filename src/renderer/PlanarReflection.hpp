#pragma once

#include "core/Common.hpp"
#include "camera/Camera.hpp"
#include "scene/SceneObject.hpp"
#include "renderer/Shader.hpp"
#include <memory>

class GLStateGuard;

class PlanarReflection {
public:
    PlanarReflection();
    ~PlanarReflection();

    bool init(int resolution = 512);
    void resize(int resolution);
    void cleanup();

    void computeReflectionMatrices(const Camera& mainCamera,
                                   const glm::vec3& planePos,
                                   const glm::vec3& planeNormal,
                                   float aspect,
                                   glm::mat4& outView,
                                   glm::mat4& outProj);

    void beginRender();
    void endRender();

    unsigned int getReflectionTexture() const { return reflectionTexture; }
    int getResolution() const { return resolution; }

    bool enabled = true;

private:
    int resolution = 512;
    unsigned int fbo = 0;
    unsigned int reflectionTexture = 0;
    unsigned int depthRbo = 0;
    std::unique_ptr<GLStateGuard> stateGuard;

    void setupFBO();
};
