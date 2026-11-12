#include "renderer/ShadowPipeline.hpp"
#include <iostream>

ShadowPipeline::ShadowPipeline() {}

ShadowPipeline::~ShadowPipeline() {
    cleanup();
}

bool ShadowPipeline::init(int spotRes, int pointRes) {
    spotResolution = spotRes;
    pointResolution = pointRes;

    if (!spotDepthShader.loadFromFiles("assets/shaders/shadow_depth.vert", "assets/shaders/shadow_depth.frag")) {
        std::cerr << "[ShadowPipeline] Failed to load shadow_depth shader\n";
        return false;
    }

    if (!pointCubeShader.loadFromFiles("assets/shaders/shadow_cube.vert", "assets/shaders/shadow_cube.frag")) {
        std::cerr << "[ShadowPipeline] Failed to load shadow_cube shader\n";
        return false;
    }

    setupSpotFBO();
    setupPointFBO();

    pointShadowDirty = true;
    return true;
}

void ShadowPipeline::cleanup() {
    if (spotFBO) {
        glDeleteFramebuffers(1, &spotFBO);
        spotFBO = 0;
    }
    if (spotDepthMap) {
        glDeleteTextures(1, &spotDepthMap);
        spotDepthMap = 0;
    }
    if (pointFBO) {
        glDeleteFramebuffers(1, &pointFBO);
        pointFBO = 0;
    }
    if (pointDepthCubemap) {
        glDeleteTextures(1, &pointDepthCubemap);
        pointDepthCubemap = 0;
    }
}

void ShadowPipeline::resize(int spotRes, int pointRes) {
    cleanup();
    spotResolution = spotRes;
    pointResolution = pointRes;
    setupSpotFBO();
    setupPointFBO();
    pointShadowDirty = true;
}

void ShadowPipeline::setupSpotFBO() {
    glGenFramebuffers(1, &spotFBO);
    glGenTextures(1, &spotDepthMap);

    glBindTexture(GL_TEXTURE_2D, spotDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, spotResolution, spotResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, spotFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, spotDepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[ShadowPipeline] Spot Shadow FBO is incomplete!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowPipeline::setupPointFBO() {
    glGenFramebuffers(1, &pointFBO);
    glGenTextures(1, &pointDepthCubemap);

    glBindTexture(GL_TEXTURE_CUBE_MAP, pointDepthCubemap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24,
                     pointResolution, pointResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, pointFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, pointDepthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[ShadowPipeline] Point Shadow Cubemap FBO is incomplete!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowPipeline::renderSpotShadow(const SpotLight& spot, const std::shared_ptr<SceneObject>& root) {
    if (!spot.enabled || spot.intensity < 0.001f || !root) return;

    // Outer cutoff in degrees
    float outerAngleDeg = glm::degrees(std::acos(spot.outerCutOff)) * 2.0f;
    float fov = std::max(45.0f, std::min(130.0f, outerAngleDeg));

    glm::mat4 lightProj = glm::perspective(glm::radians(fov), 1.0f, 0.1f, 20.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (std::abs(glm::dot(normalize(spot.direction), up)) > 0.99f) {
        up = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    glm::mat4 lightView = glm::lookAt(spot.position, spot.position + spot.direction, up);
    spotLightMatrix = lightProj * lightView;

    glViewport(0, 0, spotResolution, spotResolution);
    glBindFramebuffer(GL_FRAMEBUFFER, spotFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT); // Front-face culling to eliminate acne on thick solid geometry

    spotDepthShader.use();
    spotDepthShader.setMat4("lightSpaceMatrix", spotLightMatrix);

    std::vector<std::shared_ptr<SceneObject>> objects;
    root->collectAll(objects);

    for (const auto& obj : objects) {
        if (!obj || !obj->isVisible || !obj->mesh) continue;
        spotDepthShader.setMat4("model", obj->getWorldMatrix());
        obj->mesh->draw();
    }

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowPipeline::renderPointShadow(const PointLight& point, const std::shared_ptr<SceneObject>& root, bool forceBake) {
    if (!pointShadowEnabled || !point.enabled || point.intensity < 0.001f || !root) return;
    if (!pointShadowDirty && !forceBake) return;

    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, pointFarPlane);
    glm::vec3 pos = point.position;

    glm::mat4 shadowTransforms[6] = {
        shadowProj * glm::lookAt(pos, pos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        shadowProj * glm::lookAt(pos, pos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        shadowProj * glm::lookAt(pos, pos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        shadowProj * glm::lookAt(pos, pos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        shadowProj * glm::lookAt(pos, pos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        shadowProj * glm::lookAt(pos, pos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    glViewport(0, 0, pointResolution, pointResolution);
    glBindFramebuffer(GL_FRAMEBUFFER, pointFBO);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE); // Point light inside room can cast from both sides cleanly

    pointCubeShader.use();
    pointCubeShader.setVec3("lightPos", pos);
    pointCubeShader.setFloat("farPlane", pointFarPlane);

    std::vector<std::shared_ptr<SceneObject>> objects;
    root->collectAll(objects);

    for (int face = 0; face < 6; ++face) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, pointDepthCubemap, 0);
        glClear(GL_DEPTH_BUFFER_BIT);

        pointCubeShader.setMat4("lightSpaceMatrix", shadowTransforms[face]);

        for (const auto& obj : objects) {
            if (!obj || !obj->isVisible || !obj->mesh) continue;
            pointCubeShader.setMat4("model", obj->getWorldMatrix());
            obj->mesh->draw();
        }
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    pointShadowDirty = false;
}
