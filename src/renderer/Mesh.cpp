#include "renderer/Mesh.hpp"

Mesh::Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds)
    : vertices(verts), indices(inds) {
    computeAABB();
    setupMesh();
}

Mesh::~Mesh() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        VAO = VBO = EBO = 0;
    }
}

void Mesh::computeAABB() {
    if (vertices.empty()) {
        localAABB.min = glm::vec3(-0.5f);
        localAABB.max = glm::vec3(0.5f);
        return;
    }

    localAABB.min = vertices[0].position;
    localAABB.max = vertices[0].position;

    for (const auto& v : vertices) {
        localAABB.min = glm::min(localAABB.min, v.position);
        localAABB.max = glm::max(localAABB.max, v.position);
    }
}

void Mesh::computeTangents() {
    if (indices.size() < 3) return;

    for (auto& v : vertices) {
        v.tangent = glm::vec3(0.0f);
        v.bitangent = glm::vec3(0.0f);
    }

    for (size_t i = 0; i < indices.size(); i += 3) {
        Vertex& v0 = vertices[indices[i]];
        Vertex& v1 = vertices[indices[i + 1]];
        Vertex& v2 = vertices[indices[i + 2]];

        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;
        glm::vec2 deltaUV1 = v1.texCoords - v0.texCoords;
        glm::vec2 deltaUV2 = v2.texCoords - v0.texCoords;

        float f = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        if (std::abs(f) < 1e-6f) f = 1e-6f;
        float r = 1.0f / f;

        glm::vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * r;

        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;

        v0.bitangent += bitangent;
        v1.bitangent += bitangent;
        v2.bitangent += bitangent;
    }

    // Gram-Schmidt orthogonalize
    for (auto& v : vertices) {
        if (glm::length(v.tangent) > 1e-5f) {
            v.tangent = glm::normalize(v.tangent - v.normal * glm::dot(v.normal, v.tangent));
        } else {
            glm::vec3 c1 = glm::cross(v.normal, glm::vec3(0.0f, 0.0f, 1.0f));
            glm::vec3 c2 = glm::cross(v.normal, glm::vec3(0.0f, 1.0f, 0.0f));
            v.tangent = glm::normalize(glm::length(c1) > glm::length(c2) ? c1 : c2);
        }
        if (glm::length(v.bitangent) > 1e-5f) {
            v.bitangent = glm::normalize(v.bitangent);
        } else {
            v.bitangent = glm::normalize(glm::cross(v.normal, v.tangent));
        }
    }
}

void Mesh::setupMesh() {
    if (vertices.empty()) return;

    computeTangents();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    if (!indices.empty()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    // 0: Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // 1: Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // 2: TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    // 3: Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    // 4: Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

    glBindVertexArray(0);
}

void Mesh::draw() const {
    if (VAO == 0) return;
    glBindVertexArray(VAO);
    if (!indices.empty()) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
    }
    glBindVertexArray(0);
}
