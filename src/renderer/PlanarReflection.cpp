#include "renderer/PlanarReflection.hpp"
#include "renderer/GLStateGuard.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

PlanarReflection::PlanarReflection() {}

PlanarReflection::~PlanarReflection() {
    cleanup();
}

bool PlanarReflection::init(int res) {
    resolution = res;
    setupFBO();
    return (fbo != 0);
}

void PlanarReflection::resize(int res) {
    if (res <= 0 || res == resolution) return;
    resolution = res;
    cleanup();
    setupFBO();
}

void PlanarReflection::cleanup() {
    if (fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &reflectionTexture);
        glDeleteRenderbuffers(1, &depthRbo);
        fbo = 0;
        reflectionTexture = 0;
        depthRbo = 0;
    }
}

void PlanarReflection::setupFBO() {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Color texture (RGBA16F HDR)
    glGenTextures(1, &reflectionTexture);
    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution, resolution, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionTexture, 0);

    // Depth Renderbuffer
    glGenRenderbuffers(1, &depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[PlanarReflection] Framebuffer is incomplete!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PlanarReflection::computeReflectionMatrices(const Camera& mainCamera,
                                               const glm::vec3& planePos,
                                               const glm::vec3& planeNormal,
                                               float aspect,
                                               glm::mat4& outView,
                                               glm::mat4& outProj) {
    glm::vec3 n = glm::normalize(planeNormal);
    float d = -glm::dot(n, planePos);

    // Reflection matrix for positions
    glm::mat4 R(1.0f);
    R[0][0] = 1.0f - 2.0f * n.x * n.x;
    R[0][1] = -2.0f * n.x * n.y;
    R[0][2] = -2.0f * n.x * n.z;
    R[0][3] = 0.0f;

    R[1][0] = -2.0f * n.x * n.y;
    R[1][1] = 1.0f - 2.0f * n.y * n.y;
    R[1][2] = -2.0f * n.y * n.z;
    R[1][3] = 0.0f;

    R[2][0] = -2.0f * n.x * n.z;
    R[2][1] = -2.0f * n.y * n.z;
    R[2][2] = 1.0f - 2.0f * n.z * n.z;
    R[2][3] = 0.0f;

    R[3][0] = -2.0f * n.x * d;
    R[3][1] = -2.0f * n.y * d;
    R[3][2] = -2.0f * n.z * d;
    R[3][3] = 1.0f;

    glm::vec4 refCamPos4 = R * glm::vec4(mainCamera.position, 1.0f);
    glm::vec3 refCamPos = glm::vec3(refCamPos4) / refCamPos4.w;

    glm::vec3 refCamFront = glm::reflect(mainCamera.front, n);
    glm::vec3 refCamUp = glm::reflect(mainCamera.up, n);

    outView = glm::lookAt(refCamPos, refCamPos + refCamFront, refCamUp);
    outProj = mainCamera.getProjectionMatrix(aspect);

    // Oblique Near-Plane Projection Matrix (Lengyel method)
    // Add small bias (0.05m) to clip plane distance d to eliminate Z-fighting and near-plane precision flickering
    float clipPlaneBias = 0.05f;
    glm::vec4 clipPlane = glm::vec4(n, d + clipPlaneBias);
    glm::vec4 viewSpacePlane = glm::transpose(glm::inverse(outView)) * clipPlane;
    if (viewSpacePlane.w > 0.0f) {
        viewSpacePlane = -viewSpacePlane;
    }

    glm::vec4 q;
    q.x = (glm::sign(viewSpacePlane.x) + outProj[2][0]) / outProj[0][0];
    q.y = (glm::sign(viewSpacePlane.y) + outProj[2][1]) / outProj[1][1];
    q.z = -1.0f;
    q.w = (1.0f + outProj[2][2]) / outProj[3][2];

    glm::vec4 c = viewSpacePlane * (2.0f / glm::dot(viewSpacePlane, q));
    outProj[0][2] = c.x;
    outProj[1][2] = c.y;
    outProj[2][2] = c.z + 1.0f;
    outProj[3][2] = c.w;
}

void PlanarReflection::beginRender() {
    stateGuard = std::make_unique<GLStateGuard>();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, resolution, resolution);
    glClearColor(0.04f, 0.03f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW); // Inverted face winding due to reflection
    glEnable(GL_CLIP_DISTANCE0);
}

void PlanarReflection::endRender() {
    glDisable(GL_CLIP_DISTANCE0);
    glFrontFace(GL_CCW);
    if (stateGuard) {
        stateGuard->restore();
        stateGuard.reset();
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
