#pragma once

#include "core/Common.hpp"
#include "renderer/Shader.hpp"
#include "renderer/Light.hpp"
#include <vector>

struct DustParticle {
    glm::vec3 position;
    glm::vec3 velocity;
    float size;
    float phase;
};

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    bool init(int maxCount = 450);
    void update(float deltaTime, float totalTime, const glm::vec3& cameraPos);
    void render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos, const SpotLight& spot);
    void cleanup();

    bool enabled = false;
    int activeCount = 400;
    float particleBaseSize = 0.04f;

private:
    int maxParticles = 500;
    std::vector<DustParticle> particles;
    std::vector<float> vertexBufferData; // x, y, z, size

    unsigned int vao = 0;
    unsigned int vbo = 0;

    Shader particleShader;

    void resetParticle(int index, const glm::vec3& center);
};
