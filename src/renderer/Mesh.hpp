#pragma once

#include "core/Common.hpp"

struct Vertex {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec2 texCoords = glm::vec2(0.0f);
    glm::vec3 tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    AABB localAABB;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;

    Mesh() = default;
    Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds);
    ~Mesh();

    void setupMesh();
    void computeTangents();
    void draw() const;

    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<unsigned int>& getIndices() const { return indices; }

private:
    void computeAABB();
};
