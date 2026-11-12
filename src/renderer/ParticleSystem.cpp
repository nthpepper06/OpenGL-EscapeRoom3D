#include "renderer/ParticleSystem.hpp"
#include <random>
#include <iostream>

ParticleSystem::ParticleSystem() {}

ParticleSystem::~ParticleSystem() {
    cleanup();
}

void ParticleSystem::resetParticle(int index, const glm::vec3& center) {
    static std::mt19937 rng(1337);
    std::uniform_real_distribution<float> distPos(-4.0f, 4.0f);
    std::uniform_real_distribution<float> distY(0.3f, 3.8f);
    std::uniform_real_distribution<float> distVel(-0.04f, 0.04f);
    std::uniform_real_distribution<float> distPhase(0.0f, 6.28318f);
    std::uniform_real_distribution<float> distScale(0.025f, 0.065f);

    particles[index].position = glm::vec3(distPos(rng), distY(rng), distPos(rng));
    particles[index].velocity = glm::vec3(distVel(rng), distVel(rng) * 0.4f, distVel(rng));
    particles[index].size = distScale(rng);
    particles[index].phase = distPhase(rng);
}

bool ParticleSystem::init(int maxCount) {
    maxParticles = std::max(50, maxCount);
    activeCount = std::min(maxParticles, 400);

    if (!particleShader.loadFromFiles("assets/shaders/particle.vert", "assets/shaders/particle.frag")) {
        std::cerr << "[ParticleSystem] Failed to load particle shader\n";
        return false;
    }

    particles.resize(maxParticles);
    vertexBufferData.resize(maxParticles * 4); // x, y, z, size

    for (int i = 0; i < maxParticles; ++i) {
        resetParticle(i, glm::vec3(0.0f));
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferData.size() * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    // layout (location = 0) vec3 aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // layout (location = 1) float aSize
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    return true;
}

void ParticleSystem::cleanup() {
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
}

void ParticleSystem::update(float deltaTime, float totalTime, const glm::vec3& cameraPos) {
    if (!enabled) return;

    int count = std::min(activeCount, maxParticles);
    for (int i = 0; i < count; ++i) {
        // Slow organic Brownian drift
        float p = particles[i].phase;
        glm::vec3 drift(
            std::sin(totalTime * 0.8f + p) * 0.035f,
            std::cos(totalTime * 0.5f + p * 1.3f) * 0.02f,
            std::sin(totalTime * 0.6f + p * 0.7f) * 0.035f
        );

        particles[i].position += (particles[i].velocity + drift) * deltaTime;

        // Wrap around room boundary box [-4.5, 4.5]
        if (particles[i].position.x > 4.4f) particles[i].position.x = -4.4f;
        if (particles[i].position.x < -4.4f) particles[i].position.x = 4.4f;
        if (particles[i].position.z > 4.4f) particles[i].position.z = -4.4f;
        if (particles[i].position.z < -4.4f) particles[i].position.z = 4.4f;
        if (particles[i].position.y > 3.8f) particles[i].position.y = 0.2f;
        if (particles[i].position.y < 0.2f) particles[i].position.y = 3.8f;

        // Populate flat buffer
        int baseIdx = i * 4;
        vertexBufferData[baseIdx + 0] = particles[i].position.x;
        vertexBufferData[baseIdx + 1] = particles[i].position.y;
        vertexBufferData[baseIdx + 2] = particles[i].position.z;
        vertexBufferData[baseIdx + 3] = particles[i].size * (particleBaseSize / 0.04f);
    }

    // Stream to GPU
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * 4 * sizeof(float), vertexBufferData.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleSystem::render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos, const SpotLight& spot) {
    if (!enabled || activeCount <= 0) return;

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for dust motes
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE); // Don't write to depth buffer so particles don't occlude solid meshes

    particleShader.use();
    particleShader.setMat4("view", view);
    particleShader.setMat4("projection", projection);
    particleShader.setVec3("viewPos", viewPos);

    particleShader.setBool("spotEnabled", spot.enabled);
    particleShader.setVec3("spotPos", spot.position);
    particleShader.setVec3("spotDir", spot.direction);
    particleShader.setFloat("spotCutOff", spot.cutOff);
    particleShader.setFloat("spotOuterCutOff", spot.outerCutOff);
    particleShader.setVec3("spotColor", spot.color);

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, std::min(activeCount, maxParticles));
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
