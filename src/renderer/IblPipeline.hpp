#pragma once

#include "core/Common.hpp"
#include <vector>

class IblPipeline {
public:
    IblPipeline();
    ~IblPipeline();

    // Initializes and generates all IBL Split-Sum assets:
    // 1. Procedural dim interior environment cubemap
    // 2. Diffuse Irradiance cubemap (32x32)
    // 3. Prefiltered Specular cubemap (128x128, 5 mips)
    // 4. Precomputed 2D BRDF LUT (512x512 RG16F)
    bool init();
    void cleanup();

    unsigned int getEnvironmentCubemap() const { return envCubemap; }
    unsigned int getIrradianceMap() const { return irradianceMap; }
    unsigned int getPrefilterMap() const { return prefilterMap; }
    unsigned int getBrdfLUT() const { return brdfLUT; }

private:
    unsigned int envCubemap = 0;
    unsigned int irradianceMap = 0;
    unsigned int prefilterMap = 0;
    unsigned int brdfLUT = 0;

    // Generation helpers
    void generateInteriorEnvironment(int size, std::vector<std::vector<glm::vec3>>& faces);
    void generateIrradianceMap(const std::vector<std::vector<glm::vec3>>& envFaces, int envSize, int irrSize);
    void generatePrefilterMap(const std::vector<std::vector<glm::vec3>>& envFaces, int envSize, int baseSize, int mipLevels);
    void generateBrdfLUT(int size);

    // Mathematical sampling utilities
    static glm::vec3 sampleCubemap(const std::vector<std::vector<glm::vec3>>& faces, int size, const glm::vec3& dir);
    static glm::vec3 getDirectionForFace(int face, float u, float v);
    static glm::vec2 hammersley(unsigned int i, unsigned int N);
    static glm::vec3 importanceSampleGGX(const glm::vec2& Xi, const glm::vec3& N, float roughness);
    static float geometrySmithIBL(const glm::vec3& N, const glm::vec3& V, const glm::vec3& L, float roughness);
    static glm::vec2 integrateBRDF(float NdotV, float roughness);
};
