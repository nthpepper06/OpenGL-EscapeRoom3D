#include "renderer/MeshBatcher.hpp"
#include <iostream>

std::shared_ptr<SceneObject> MeshBatcher::batchObjects(
    const std::string& batchId,
    const std::string& batchName,
    const std::vector<std::shared_ptr<SceneObject>>& objects,
    const Material& sharedMaterial
) {
    std::vector<Vertex> batchedVertices;
    std::vector<unsigned int> batchedIndices;

    for (const auto& obj : objects) {
        if (!obj || !obj->mesh) continue;

        glm::mat4 modelMat = obj->getWorldMatrix();
        glm::mat3 normMat = obj->getNormalMatrix();

        const auto& srcVerts = obj->mesh->getVertices();
        const auto& srcInds = obj->mesh->getIndices();

        unsigned int baseIndex = static_cast<unsigned int>(batchedVertices.size());

        for (const auto& v : srcVerts) {
            Vertex transformedV = v;
            transformedV.position = glm::vec3(modelMat * glm::vec4(v.position, 1.0f));
            transformedV.normal = glm::normalize(normMat * v.normal);
            transformedV.tangent = glm::normalize(normMat * v.tangent);
            transformedV.bitangent = glm::normalize(normMat * v.bitangent);
            batchedVertices.push_back(transformedV);
        }

        for (unsigned int idx : srcInds) {
            batchedIndices.push_back(baseIndex + idx);
        }
    }

    if (batchedVertices.empty()) {
        return nullptr;
    }

    auto batchedObj = std::make_shared<SceneObject>(batchId, batchName, PrimitiveType::Cube);
    batchedObj->mesh = std::make_shared<Mesh>(batchedVertices, batchedIndices);
    batchedObj->material = sharedMaterial;
    batchedObj->position = glm::vec3(0.0f);
    batchedObj->rotation = glm::vec3(0.0f);
    batchedObj->scale = glm::vec3(1.0f);

    return batchedObj;
}
