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

std::shared_ptr<Mesh> PrimitiveGenerator::createBeveledCube(float width, float height, float depth, float bevelRadius, int bevelSegments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    bevelSegments = std::clamp(bevelSegments, 1, 8);
    float maxR = std::min(width, std::min(height, depth)) * 0.45f;
    bevelRadius = std::clamp(bevelRadius, 0.001f, maxR);

    float hx = width * 0.5f - bevelRadius;
    float hy = height * 0.5f - bevelRadius;
    float hz = depth * 0.5f - bevelRadius;

    auto calcUV = [&](const glm::vec3& p, const glm::vec3& n) -> glm::vec2 {
        glm::vec3 an = glm::abs(n);
        if (an.x >= an.y && an.x >= an.z) {
            return glm::vec2((p.z / depth) + 0.5f, (p.y / height) + 0.5f);
        } else if (an.y >= an.z) {
            return glm::vec2((p.x / width) + 0.5f, (p.z / depth) + 0.5f);
        } else {
            return glm::vec2((p.x / width) + 0.5f, (p.y / height) + 0.5f);
        }
    };

    auto addQuad = [&](const Vertex& v0, const Vertex& v1, const Vertex& v2, const Vertex& v3) {
        unsigned int idx = static_cast<unsigned int>(vertices.size());
        vertices.push_back(v0);
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
        indices.push_back(idx);
        indices.push_back(idx + 1);
        indices.push_back(idx + 2);
        indices.push_back(idx);
        indices.push_back(idx + 2);
        indices.push_back(idx + 3);
    };

    // 1. SIX FLAT FACES
    // Front (+Z)
    addQuad(
        { {-hx, -hy, hz + bevelRadius}, {0, 0, 1}, calcUV({-hx, -hy, hz + bevelRadius}, {0,0,1}) },
        { { hx, -hy, hz + bevelRadius}, {0, 0, 1}, calcUV({ hx, -hy, hz + bevelRadius}, {0,0,1}) },
        { { hx,  hy, hz + bevelRadius}, {0, 0, 1}, calcUV({ hx,  hy, hz + bevelRadius}, {0,0,1}) },
        { {-hx,  hy, hz + bevelRadius}, {0, 0, 1}, calcUV({-hx,  hy, hz + bevelRadius}, {0,0,1}) }
    );
    // Back (-Z)
    addQuad(
        { { hx, -hy, -hz - bevelRadius}, {0, 0, -1}, calcUV({ hx, -hy, -hz - bevelRadius}, {0,0,-1}) },
        { {-hx, -hy, -hz - bevelRadius}, {0, 0, -1}, calcUV({-hx, -hy, -hz - bevelRadius}, {0,0,-1}) },
        { {-hx,  hy, -hz - bevelRadius}, {0, 0, -1}, calcUV({-hx,  hy, -hz - bevelRadius}, {0,0,-1}) },
        { { hx,  hy, -hz - bevelRadius}, {0, 0, -1}, calcUV({ hx,  hy, -hz - bevelRadius}, {0,0,-1}) }
    );
    // Top (+Y)
    addQuad(
        { {-hx, hy + bevelRadius,  hz}, {0, 1, 0}, calcUV({-hx, hy + bevelRadius,  hz}, {0,1,0}) },
        { { hx, hy + bevelRadius,  hz}, {0, 1, 0}, calcUV({ hx, hy + bevelRadius,  hz}, {0,1,0}) },
        { { hx, hy + bevelRadius, -hz}, {0, 1, 0}, calcUV({ hx, hy + bevelRadius, -hz}, {0,1,0}) },
        { {-hx, hy + bevelRadius, -hz}, {0, 1, 0}, calcUV({-hx, hy + bevelRadius, -hz}, {0,1,0}) }
    );
    // Bottom (-Y)
    addQuad(
        { {-hx, -hy - bevelRadius, -hz}, {0, -1, 0}, calcUV({-hx, -hy - bevelRadius, -hz}, {0,-1,0}) },
        { { hx, -hy - bevelRadius, -hz}, {0, -1, 0}, calcUV({ hx, -hy - bevelRadius, -hz}, {0,-1,0}) },
        { { hx, -hy - bevelRadius,  hz}, {0, -1, 0}, calcUV({ hx, -hy - bevelRadius,  hz}, {0,-1,0}) },
        { {-hx, -hy - bevelRadius,  hz}, {0, -1, 0}, calcUV({-hx, -hy - bevelRadius,  hz}, {0,-1,0}) }
    );
    // Right (+X)
    addQuad(
        { {hx + bevelRadius, -hy,  hz}, {1, 0, 0}, calcUV({hx + bevelRadius, -hy,  hz}, {1,0,0}) },
        { {hx + bevelRadius, -hy, -hz}, {1, 0, 0}, calcUV({hx + bevelRadius, -hy, -hz}, {1,0,0}) },
        { {hx + bevelRadius,  hy, -hz}, {1, 0, 0}, calcUV({hx + bevelRadius,  hy, -hz}, {1,0,0}) },
        { {hx + bevelRadius,  hy,  hz}, {1, 0, 0}, calcUV({hx + bevelRadius,  hy,  hz}, {1,0,0}) }
    );
    // Left (-X)
    addQuad(
        { {-hx - bevelRadius, -hy, -hz}, {-1, 0, 0}, calcUV({-hx - bevelRadius, -hy, -hz}, {-1,0,0}) },
        { {-hx - bevelRadius, -hy,  hz}, {-1, 0, 0}, calcUV({-hx - bevelRadius, -hy,  hz}, {-1,0,0}) },
        { {-hx - bevelRadius,  hy,  hz}, {-1, 0, 0}, calcUV({-hx - bevelRadius,  hy,  hz}, {-1,0,0}) },
        { {-hx - bevelRadius,  hy, -hz}, {-1, 0, 0}, calcUV({-hx - bevelRadius,  hy, -hz}, {-1,0,0}) }
    );

    // 2. TWELVE CYLINDRICAL BEVEL EDGES
    // 4 Edges parallel to Z
    auto addEdgeZ = [&](float cx, float cy, float startAngle, float endAngle) {
        for (int i = 0; i < bevelSegments; ++i) {
            float a0 = startAngle + (endAngle - startAngle) * ((float)i / bevelSegments);
            float a1 = startAngle + (endAngle - startAngle) * ((float)(i + 1) / bevelSegments);
            glm::vec3 n0(std::cos(a0), std::sin(a0), 0.0f);
            glm::vec3 n1(std::cos(a1), std::sin(a1), 0.0f);
            glm::vec3 p0A(cx + bevelRadius * n0.x, cy + bevelRadius * n0.y, -hz);
            glm::vec3 p0B(cx + bevelRadius * n0.x, cy + bevelRadius * n0.y,  hz);
            glm::vec3 p1B(cx + bevelRadius * n1.x, cy + bevelRadius * n1.y,  hz);
            glm::vec3 p1A(cx + bevelRadius * n1.x, cy + bevelRadius * n1.y, -hz);
            addQuad(
                { p0A, n0, calcUV(p0A, n0) },
                { p0B, n0, calcUV(p0B, n0) },
                { p1B, n1, calcUV(p1B, n1) },
                { p1A, n1, calcUV(p1A, n1) }
            );
        }
    };
    addEdgeZ( hx,  hy, 0.0f, (float)M_PI * 0.5f);
    addEdgeZ(-hx,  hy, (float)M_PI * 0.5f, (float)M_PI);
    addEdgeZ(-hx, -hy, (float)M_PI, (float)M_PI * 1.5f);
    addEdgeZ( hx, -hy, (float)M_PI * 1.5f, (float)M_PI * 2.0f);

    // 4 Edges parallel to X
    auto addEdgeX = [&](float cy, float cz, float startAngle, float endAngle) {
        for (int i = 0; i < bevelSegments; ++i) {
            float a0 = startAngle + (endAngle - startAngle) * ((float)i / bevelSegments);
            float a1 = startAngle + (endAngle - startAngle) * ((float)(i + 1) / bevelSegments);
            glm::vec3 n0(0.0f, std::sin(a0), std::cos(a0));
            glm::vec3 n1(0.0f, std::sin(a1), std::cos(a1));
            glm::vec3 p0A(-hx, cy + bevelRadius * n0.y, cz + bevelRadius * n0.z);
            glm::vec3 p0B( hx, cy + bevelRadius * n0.y, cz + bevelRadius * n0.z);
            glm::vec3 p1B( hx, cy + bevelRadius * n1.y, cz + bevelRadius * n1.z);
            glm::vec3 p1A(-hx, cy + bevelRadius * n1.y, cz + bevelRadius * n1.z);
            addQuad(
                { p0A, n0, calcUV(p0A, n0) },
                { p0B, n0, calcUV(p0B, n0) },
                { p1B, n1, calcUV(p1B, n1) },
                { p1A, n1, calcUV(p1A, n1) }
            );
        }
    };
    addEdgeX( hy,  hz, (float)M_PI * 0.5f, 0.0f);
    addEdgeX( hy, -hz, (float)M_PI, (float)M_PI * 0.5f);
    addEdgeX(-hy, -hz, (float)M_PI * 1.5f, (float)M_PI);
    addEdgeX(-hy,  hz, 0.0f, (float)M_PI * -0.5f);

    // 4 Edges parallel to Y
    auto addEdgeY = [&](float cx, float cz, float startAngle, float endAngle) {
        for (int i = 0; i < bevelSegments; ++i) {
            float a0 = startAngle + (endAngle - startAngle) * ((float)i / bevelSegments);
            float a1 = startAngle + (endAngle - startAngle) * ((float)(i + 1) / bevelSegments);
            glm::vec3 n0(std::sin(a0), 0.0f, std::cos(a0));
            glm::vec3 n1(std::sin(a1), 0.0f, std::cos(a1));
            glm::vec3 p0A(cx + bevelRadius * n0.x, -hy, cz + bevelRadius * n0.z);
            glm::vec3 p0B(cx + bevelRadius * n0.x,  hy, cz + bevelRadius * n0.z);
            glm::vec3 p1B(cx + bevelRadius * n1.x,  hy, cz + bevelRadius * n1.z);
            glm::vec3 p1A(cx + bevelRadius * n1.x, -hy, cz + bevelRadius * n1.z);
            addQuad(
                { p0A, n0, calcUV(p0A, n0) },
                { p0B, n0, calcUV(p0B, n0) },
                { p1B, n1, calcUV(p1B, n1) },
                { p1A, n1, calcUV(p1A, n1) }
            );
        }
    };
    addEdgeY( hx,  hz, 0.0f, (float)M_PI * 0.5f);
    addEdgeY(-hx,  hz, (float)M_PI * 0.5f, (float)M_PI);
    addEdgeY(-hx, -hz, (float)M_PI, (float)M_PI * 1.5f);
    addEdgeY( hx, -hz, (float)M_PI * 1.5f, (float)M_PI * 2.0f);

    // 3. EIGHT SPHERICAL CORNERS
    auto addCorner = [&](float cx, float cy, float cz) {
        float sx = (cx > 0.0f) ? 1.0f : -1.0f;
        float sy = (cy > 0.0f) ? 1.0f : -1.0f;
        float sz = (cz > 0.0f) ? 1.0f : -1.0f;

        for (int j = 0; j < bevelSegments; ++j) {
            float phi0 = ((float)j / bevelSegments) * ((float)M_PI * 0.5f);
            float phi1 = ((float)(j + 1) / bevelSegments) * ((float)M_PI * 0.5f);

            for (int i = 0; i < bevelSegments; ++i) {
                float theta0 = ((float)i / bevelSegments) * ((float)M_PI * 0.5f);
                float theta1 = ((float)(i + 1) / bevelSegments) * ((float)M_PI * 0.5f);

                auto getCornerVertex = [&](float theta, float phi) -> Vertex {
                    glm::vec3 n(
                        sx * std::cos(phi) * std::sin(theta),
                        sy * std::sin(phi),
                        sz * std::cos(phi) * std::cos(theta)
                    );
                    glm::vec3 p(cx + bevelRadius * n.x, cy + bevelRadius * n.y, cz + bevelRadius * n.z);
                    return { p, glm::normalize(n), calcUV(p, n) };
                };

                Vertex v0 = getCornerVertex(theta0, phi0);
                Vertex v1 = getCornerVertex(theta1, phi0);
                Vertex v2 = getCornerVertex(theta1, phi1);
                Vertex v3 = getCornerVertex(theta0, phi1);

                if ((sx * sy * sz) > 0.0f) {
                    addQuad(v0, v1, v2, v3);
                } else {
                    addQuad(v0, v3, v2, v1);
                }
            }
        }
    };

    addCorner( hx,  hy,  hz);
    addCorner(-hx,  hy,  hz);
    addCorner(-hx, -hy,  hz);
    addCorner( hx, -hy,  hz);
    addCorner( hx,  hy, -hz);
    addCorner(-hx,  hy, -hz);
    addCorner(-hx, -hy, -hz);
    addCorner( hx, -hy, -hz);

    return std::make_shared<Mesh>(vertices, indices);
}
