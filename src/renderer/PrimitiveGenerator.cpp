#include "renderer/PrimitiveGenerator.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::shared_ptr<Mesh> PrimitiveGenerator::createCube() {
    std::vector<Vertex> vertices = {
        // Back face (normal: 0, 0, -1)
        { {-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 1.0f} },

        // Front face (normal: 0, 0, 1)
        { {-0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, 1.0f} },

        // Left face (normal: -1, 0, 0)
        { {-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f} },
        { {-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },
        { {-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f} },

        // Right face (normal: 1, 0, 0)
        { { 0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {1.0f, 1.0f} },
        { { 0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },
        { { 0.5f, -0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 0.0f} },

        // Bottom face (normal: 0, -1, 0)
        { {-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 1.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {1.0f, 1.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {1.0f, 0.0f} },
        { {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f} },

        // Top face (normal: 0, 1, 0)
        { {-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 1.0f} },
        { { 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {1.0f, 1.0f} },
        { { 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f} }
    };

    std::vector<unsigned int> indices = {
        0,  1,  2,   2,  3,  0,   // Back
        4,  5,  6,   6,  7,  4,   // Front
        8,  9,  10,  10, 11, 8,   // Left
        12, 14, 13,  12, 15, 14,  // Right
        16, 17, 18,  18, 19, 16,  // Bottom
        20, 22, 21,  20, 23, 22   // Top
    };

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> PrimitiveGenerator::createPlane() {
    // Horizontal plane on XZ, normal pointing UP (0, 1, 0)
    std::vector<Vertex> vertices = {
        { {-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
        { { 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
        { { 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
        { {-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} }
    };

    std::vector<unsigned int> indices = {
        0, 2, 1,
        0, 3, 2
    };

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> PrimitiveGenerator::createCylinder(int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float radius = 0.5f;
    float halfHeight = 0.5f;

    // Side vertices
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i / (float)segments * 2.0f * (float)M_PI;
        float cosTheta = std::cos(theta);
        float sinTheta = std::sin(theta);

        glm::vec3 normal(cosTheta, 0.0f, sinTheta);
        float u = (float)i / (float)segments;

        // Bottom vertex
        vertices.push_back({ {radius * cosTheta, -halfHeight, radius * sinTheta}, normal, {u, 0.0f} });
        // Top vertex
        vertices.push_back({ {radius * cosTheta,  halfHeight, radius * sinTheta}, normal, {u, 1.0f} });
    }

    for (int i = 0; i < segments; ++i) {
        int b1 = i * 2;
        int t1 = b1 + 1;
        int b2 = (i + 1) * 2;
        int t2 = b2 + 1;

        indices.push_back(b1);
        indices.push_back(b2);
        indices.push_back(t1);

        indices.push_back(t1);
        indices.push_back(b2);
        indices.push_back(t2);
    }

    // Top cap
    int topCenterIdx = (int)vertices.size();
    vertices.push_back({ {0.0f, halfHeight, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f} });
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i / (float)segments * 2.0f * (float)M_PI;
        float cosTheta = std::cos(theta);
        float sinTheta = std::sin(theta);
        vertices.push_back({ {radius * cosTheta, halfHeight, radius * sinTheta}, {0.0f, 1.0f, 0.0f}, {0.5f + 0.5f * cosTheta, 0.5f + 0.5f * sinTheta} });
    }
    for (int i = 0; i < segments; ++i) {
        indices.push_back(topCenterIdx);
        indices.push_back(topCenterIdx + 1 + i);
        indices.push_back(topCenterIdx + 2 + i);
    }

    // Bottom cap
    int bottomCenterIdx = (int)vertices.size();
    vertices.push_back({ {0.0f, -halfHeight, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f} });
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i / (float)segments * 2.0f * (float)M_PI;
        float cosTheta = std::cos(theta);
        float sinTheta = std::sin(theta);
        vertices.push_back({ {radius * cosTheta, -halfHeight, radius * sinTheta}, {0.0f, -1.0f, 0.0f}, {0.5f + 0.5f * cosTheta, 0.5f + 0.5f * sinTheta} });
    }
    for (int i = 0; i < segments; ++i) {
        indices.push_back(bottomCenterIdx);
        indices.push_back(bottomCenterIdx + 2 + i);
        indices.push_back(bottomCenterIdx + 1 + i);
    }

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> PrimitiveGenerator::createSphere(int sectorCount, int stackCount) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float radius = 0.5f;

    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = (float)M_PI / 2.0f - (float)i / (float)stackCount * (float)M_PI;
        float xy = radius * std::cos(stackAngle);
        float z = radius * std::sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = (float)j / (float)sectorCount * 2.0f * (float)M_PI;

            float x = xy * std::cos(sectorAngle);
            float y = xy * std::sin(sectorAngle);

            glm::vec3 pos(x, z, y);
            glm::vec3 normal = glm::normalize(pos);
            glm::vec2 tex((float)j / sectorCount, (float)i / stackCount);

            vertices.push_back({ pos, normal, tex });
        }
    }

    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}
