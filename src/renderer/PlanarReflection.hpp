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

    bool init(int width = 1280, int height = 720);
    void resize(int width, int height);
    void cleanup();

    void computeReflectionMatrices(const Camera& mainCamera,
                                   const glm::vec3& planePos,
                                   const glm::vec3& planeNormal,
                                   float aspect,
                                   glm::mat4& outView,
                                   glm::mat4& outProj);

    glm::vec3 computeRefCamPos(const Camera& mainCamera,
                               const glm::vec3& planePos,
                               const glm::vec3& planeNormal);

    void beginRender();
    void endRender();

    unsigned int getReflectionTexture() const { return reflectionTexture; }
    int getWidth() const { return texWidth; }
    int getHeight() const { return texHeight; }

    bool enabled = true;

private:
    int texWidth = 1280;
    int texHeight = 720;
    unsigned int fbo = 0;
    unsigned int reflectionTexture = 0;
    unsigned int depthRbo = 0;
    std::unique_ptr<GLStateGuard> stateGuard;

    void setupFBO();
};
