#include "renderer/IblPipeline.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

IblPipeline::IblPipeline() {}

IblPipeline::~IblPipeline() {
    cleanup();
}

void IblPipeline::cleanup() {
    if (envCubemap != 0) { glDeleteTextures(1, &envCubemap); envCubemap = 0; }
    if (irradianceMap != 0) { glDeleteTextures(1, &irradianceMap); irradianceMap = 0; }
    if (prefilterMap != 0) { glDeleteTextures(1, &prefilterMap); prefilterMap = 0; }
    if (brdfLUT != 0) { glDeleteTextures(1, &brdfLUT); brdfLUT = 0; }
}

bool IblPipeline::init() {
    cleanup();

    // Enable seamless cubemap filtering globally
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    const int envSize = 128;
    const int irrSize = 32;
    const int prefilterSize = 128;
    const int brdfSize = 512;

    std::vector<std::vector<glm::vec3>> envFaces;
    generateInteriorEnvironment(envSize, envFaces);

    generateIrradianceMap(envFaces, envSize, irrSize);
    generatePrefilterMap(envFaces, envSize, prefilterSize, 5);
    generateBrdfLUT(brdfSize);

    return true;
}

// ----------------------------------------------------------------------------
// Procedural Dim Interior Escape Room Cubemap (Motivated Light Sources)
// ----------------------------------------------------------------------------
void IblPipeline::generateInteriorEnvironment(int size, std::vector<std::vector<glm::vec3>>& faces) {
    faces.resize(6);
    for (int f = 0; f < 6; ++f) {
        faces[f].resize(size * size);
        for (int y = 0; y < size; ++y) {
            float v = (2.0f * (y + 0.5f) / float(size)) - 1.0f;
            for (int x = 0; x < size; ++x) {
                float u = (2.0f * (x + 0.5f) / float(size)) - 1.0f;
                glm::vec3 dir = getDirectionForFace(f, u, v);

                glm::vec3 color(0.06f, 0.05f, 0.05f); // Deep interior ambient base

                // Ceiling fixture (Point light / chandelier bounce at +Y)
                if (dir.y > 0.0f) {
                    float upWeight = std::max(dir.y, 0.0f);
                    float distToFixture = std::sqrt(dir.x * dir.x + dir.z * dir.z);

                    // Central overhead light source
                    if (dir.y > 0.78f && distToFixture < 0.32f) {
                        float spot = 1.0f - (distToFixture / 0.32f);
                        color += glm::vec3(12.0f, 9.0f, 4.5f) * (spot * spot);
                    }

                    // Vaulted ceiling warm glow
                    color += glm::vec3(0.24f, 0.16f, 0.10f) * upWeight;
                } else {
                    // Dark floor damp stone reflection
                    float downWeight = std::max(-dir.y, 0.0f);
                    color = glm::mix(color, glm::vec3(0.035f, 0.030f, 0.030f), downWeight);
                }

                // Motivated wall leakage & subtle accents
                if (dir.z > 0.7f) {
                    // Front door amber/crimson glowing exit leak
                    float zDist = (dir.z - 0.7f) / 0.3f;
                    color += glm::vec3(0.18f, 0.08f, 0.04f) * zDist;
                } else if (dir.z < -0.7f) {
                    // Back wall cool crypt stone tone
                    float bDist = (-dir.z - 0.7f) / 0.3f;
                    color += glm::vec3(0.04f, 0.05f, 0.07f) * bDist;
                }

                faces[f][y * size + x] = color;
            }
        }
    }

    // Upload to OpenGL Environment Cubemap
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    for (int f = 0; f < 6; ++f) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, faces[f].data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

// ----------------------------------------------------------------------------
// Diffuse Irradiance Cubemap (32x32 per face)
// ----------------------------------------------------------------------------
void IblPipeline::generateIrradianceMap(const std::vector<std::vector<glm::vec3>>& envFaces, int envSize, int irrSize) {
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

    const unsigned int NUM_SAMPLES = 128u;
    std::vector<glm::vec3> irrFace(irrSize * irrSize);

    for (int f = 0; f < 6; ++f) {
        for (int y = 0; y < irrSize; ++y) {
            float v = (2.0f * (y + 0.5f) / float(irrSize)) - 1.0f;
            for (int x = 0; x < irrSize; ++x) {
                float u = (2.0f * (x + 0.5f) / float(irrSize)) - 1.0f;
                glm::vec3 N = getDirectionForFace(f, u, v);

                glm::vec3 irradiance(0.0f);
                float totalWeight = 0.0f;

                for (unsigned int i = 0u; i < NUM_SAMPLES; ++i) {
                    glm::vec2 Xi = hammersley(i, NUM_SAMPLES);
                    // Cosine-weighted hemisphere sampling
                    float phi = 2.0f * float(M_PI) * Xi.x;
                    float cosTheta = std::sqrt(1.0f - Xi.y);
                    float sinTheta = std::sqrt(Xi.y);

                    glm::vec3 sampleTangent(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);

                    // Build orthonormal tangent basis around N
                    glm::vec3 up = std::abs(N.z) < 0.999f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
                    glm::vec3 tangent = glm::normalize(glm::cross(up, N));
                    glm::vec3 bitangent = glm::cross(N, tangent);

                    glm::vec3 sampleDir = glm::normalize(tangent * sampleTangent.x + bitangent * sampleTangent.y + N * sampleTangent.z);

                    float NdotL = std::max(glm::dot(N, sampleDir), 0.0f);
                    if (NdotL > 0.0f) {
                        irradiance += sampleCubemap(envFaces, envSize, sampleDir) * NdotL;
                        totalWeight += NdotL;
                    }
                }

                if (totalWeight > 0.0001f) {
                    irradiance /= totalWeight;
                }

                irrFace[y * irrSize + x] = irradiance;
            }
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_RGB16F, irrSize, irrSize, 0, GL_RGB, GL_FLOAT, irrFace.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

// ----------------------------------------------------------------------------
// Prefiltered Specular Cubemap (128x128, 5 Mip Levels)
// ----------------------------------------------------------------------------
void IblPipeline::generatePrefilterMap(const std::vector<std::vector<glm::vec3>>& envFaces, int envSize, int baseSize, int mipLevels) {
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    const unsigned int NUM_SAMPLES = 64u;

    for (int mip = 0; mip < mipLevels; ++mip) {
        int mipSize = baseSize >> mip;
        if (mipSize < 1) mipSize = 1;
        float roughness = float(mip) / float(mipLevels - 1);

        std::vector<glm::vec3> faceData(mipSize * mipSize);

        for (int f = 0; f < 6; ++f) {
            for (int y = 0; y < mipSize; ++y) {
                float v = (2.0f * (y + 0.5f) / float(mipSize)) - 1.0f;
                for (int x = 0; x < mipSize; ++x) {
                    float u = (2.0f * (x + 0.5f) / float(mipSize)) - 1.0f;
                    glm::vec3 R = getDirectionForFace(f, u, v);
                    glm::vec3 N = R;
                    glm::vec3 V = R;

                    glm::vec3 prefilteredColor(0.0f);
                    float totalWeight = 0.0f;

                    if (roughness < 0.001f) {
                        // Mip 0: Mirror reflection directly samples the environment
                        prefilteredColor = sampleCubemap(envFaces, envSize, R);
                        totalWeight = 1.0f;
                    } else {
                        for (unsigned int i = 0u; i < NUM_SAMPLES; ++i) {
                            glm::vec2 Xi = hammersley(i, NUM_SAMPLES);
                            glm::vec3 H = importanceSampleGGX(Xi, N, roughness);
                            glm::vec3 L = glm::normalize(2.0f * glm::dot(V, H) * H - V);

                            float NdotL = std::max(glm::dot(N, L), 0.0f);
                            if (NdotL > 0.0f) {
                                prefilteredColor += sampleCubemap(envFaces, envSize, L) * NdotL;
                                totalWeight += NdotL;
                            }
                        }
                        if (totalWeight > 0.0001f) {
                            prefilteredColor /= totalWeight;
                        }
                    }

                    faceData[y * mipSize + x] = prefilteredColor;
                }
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, mip, GL_RGB16F, mipSize, mipSize, 0, GL_RGB, GL_FLOAT, faceData.data());
        }
    }
}

// ----------------------------------------------------------------------------
// 2D BRDF LUT (512x512 RG16F)
// ----------------------------------------------------------------------------
void IblPipeline::generateBrdfLUT(int size) {
    std::vector<glm::vec2> lutData(size * size);

    for (int y = 0; y < size; ++y) {
        float roughness = (float(y) + 0.5f) / float(size);
        for (int x = 0; x < size; ++x) {
            float NdotV = (float(x) + 0.5f) / float(size);
            lutData[y * size + x] = integrateBRDF(std::max(NdotV, 0.001f), roughness);
        }
    }

    glGenTextures(1, &brdfLUT);
    glBindTexture(GL_TEXTURE_2D, brdfLUT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, size, size, 0, GL_RG, GL_FLOAT, lutData.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

// ----------------------------------------------------------------------------
// Mathematical Helpers
// ----------------------------------------------------------------------------
glm::vec3 IblPipeline::getDirectionForFace(int face, float u, float v) {
    switch (face) {
        case 0: return glm::normalize(glm::vec3( 1.0f, -v, -u)); // +X
        case 1: return glm::normalize(glm::vec3(-1.0f, -v,  u)); // -X
        case 2: return glm::normalize(glm::vec3( u,  1.0f,  v)); // +Y
        case 3: return glm::normalize(glm::vec3( u, -1.0f, -v)); // -Y
        case 4: return glm::normalize(glm::vec3( u, -v,  1.0f)); // +Z
        case 5: return glm::normalize(glm::vec3(-u, -v, -1.0f)); // -Z
        default: return glm::vec3(0.0f, 1.0f, 0.0f);
    }
}

glm::vec3 IblPipeline::sampleCubemap(const std::vector<std::vector<glm::vec3>>& faces, int size, const glm::vec3& dir) {
    glm::vec3 absDir = glm::abs(dir);
    int face = 0;
    float u = 0.0f, v = 0.0f;

    if (absDir.x >= absDir.y && absDir.x >= absDir.z) {
        if (dir.x > 0.0f) { face = 0; u = -dir.z / absDir.x; v = -dir.y / absDir.x; }
        else              { face = 1; u =  dir.z / absDir.x; v = -dir.y / absDir.x; }
    } else if (absDir.y >= absDir.x && absDir.y >= absDir.z) {
        if (dir.y > 0.0f) { face = 2; u =  dir.x / absDir.y; v =  dir.z / absDir.y; }
        else              { face = 3; u =  dir.x / absDir.y; v = -dir.z / absDir.y; }
    } else {
        if (dir.z > 0.0f) { face = 4; u =  dir.x / absDir.z; v = -dir.y / absDir.z; }
        else              { face = 5; u = -dir.x / absDir.z; v = -dir.y / absDir.z; }
    }

    int tx = std::clamp(static_cast<int>((u * 0.5f + 0.5f) * size), 0, size - 1);
    int ty = std::clamp(static_cast<int>((v * 0.5f + 0.5f) * size), 0, size - 1);
    return faces[face][ty * size + tx];
}

glm::vec2 IblPipeline::hammersley(unsigned int i, unsigned int N) {
    unsigned int bits = i;
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float radical = float(bits) * 2.3283064365386963e-10f;
    return glm::vec2(float(i) / float(N), radical);
}

glm::vec3 IblPipeline::importanceSampleGGX(const glm::vec2& Xi, const glm::vec3& N, float roughness) {
    float a = roughness * roughness;
    float phi = 2.0f * float(M_PI) * Xi.x;
    float cosTheta = std::sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
    float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

    glm::vec3 H(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);

    glm::vec3 up = std::abs(N.z) < 0.999f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 tangent = glm::normalize(glm::cross(up, N));
    glm::vec3 bitangent = glm::cross(N, tangent);

    return glm::normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

float IblPipeline::geometrySmithIBL(const glm::vec3& N, const glm::vec3& V, const glm::vec3& L, float roughness) {
    float k = (roughness * roughness) / 2.0f;
    float NdotV = std::max(glm::dot(N, V), 0.0f);
    float NdotL = std::max(glm::dot(N, L), 0.0f);
    float ggx1 = NdotV / (NdotV * (1.0f - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0f - k) + k);
    return ggx1 * ggx2;
}

glm::vec2 IblPipeline::integrateBRDF(float NdotV, float roughness) {
    glm::vec3 V;
    V.x = std::sqrt(1.0f - NdotV * NdotV);
    V.y = 0.0f;
    V.z = NdotV;

    float A = 0.0f;
    float B = 0.0f;
    glm::vec3 N(0.0f, 0.0f, 1.0f);

    const unsigned int SAMPLE_COUNT = 128u;
    for (unsigned int i = 0u; i < SAMPLE_COUNT; ++i) {
        glm::vec2 Xi = hammersley(i, SAMPLE_COUNT);
        glm::vec3 H = importanceSampleGGX(Xi, N, roughness);
        glm::vec3 L = glm::normalize(2.0f * glm::dot(V, H) * H - V);

        float NdotL = std::max(L.z, 0.0f);
        float NdotH = std::max(H.z, 0.0f);
        float VdotH = std::max(glm::dot(V, H), 0.0f);

        if (NdotL > 0.0f) {
            float G = geometrySmithIBL(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = std::pow(1.0f - VdotH, 5.0f);

            A += (1.0f - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    return glm::vec2(A / float(SAMPLE_COUNT), B / float(SAMPLE_COUNT));
}
