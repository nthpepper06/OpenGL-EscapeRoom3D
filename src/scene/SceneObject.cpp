#include "scene/SceneObject.hpp"

SceneObject::SceneObject(const std::string& objId, const std::string& objName, PrimitiveType type)
    : id(objId), name(objName), primitiveType(type) {
}

void SceneObject::addChild(std::shared_ptr<SceneObject> child) {
    if (!child) return;
    child->parent = this;
    children.push_back(child);
}

bool SceneObject::removeChild(const std::string& childId) {
    for (auto it = children.begin(); it != children.end(); ++it) {
        if ((*it)->id == childId) {
            (*it)->parent = nullptr;
            children.erase(it);
            return true;
        }
        if ((*it)->removeChild(childId)) {
            return true;
        }
    }
    return false;
}

glm::mat4 SceneObject::getLocalMatrix() const {
    glm::mat4 mat = glm::mat4(1.0f);
    mat = glm::translate(mat, position);

    // Apply rotation XYZ
    mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    mat = glm::scale(mat, scale);
    return mat;
}

void SceneObject::setPosition(const glm::vec3& newPos) {
    if (position != newPos) {
        position = newPos;
        markTransformDirty();
    }
}

void SceneObject::setRotation(const glm::vec3& newRot) {
    if (rotation != newRot) {
        rotation = newRot;
        markTransformDirty();
    }
}

void SceneObject::setScale(const glm::vec3& newScale) {
    if (scale != newScale) {
        scale = newScale;
        markTransformDirty();
    }
}

void SceneObject::markTransformDirty() {
    isTransformDirty = true;
    for (auto& child : children) {
        if (child) {
            child->markTransformDirty();
        }
    }
}

glm::mat4 SceneObject::getWorldMatrix() const {
    if (position != lastPosition || rotation != lastRotation || scale != lastScale) {
        const_cast<SceneObject*>(this)->markTransformDirty();
        lastPosition = position;
        lastRotation = rotation;
        lastScale = scale;
    }

    if (isTransformDirty) {
        if (parent != nullptr) {
            cachedWorldMatrix = parent->getWorldMatrix() * getLocalMatrix();
        } else {
            cachedWorldMatrix = getLocalMatrix();
        }
        cachedNormalMatrix = glm::transpose(glm::inverse(glm::mat3(cachedWorldMatrix)));
        isTransformDirty = false;
    }
    return cachedWorldMatrix;
}

glm::mat3 SceneObject::getNormalMatrix() const {
    getWorldMatrix(); // ensures cachedNormalMatrix is updated if dirty
    return cachedNormalMatrix;
}

AABB SceneObject::getWorldAABB() const {
    AABB worldBox;
    AABB localBox;
    if (mesh) {
        localBox = mesh->localAABB;
    } else {
        localBox.min = glm::vec3(-0.5f);
        localBox.max = glm::vec3(0.5f);
    }

    glm::mat4 worldMat = getWorldMatrix();
    glm::vec3 corners[8] = {
        glm::vec3(localBox.min.x, localBox.min.y, localBox.min.z),
        glm::vec3(localBox.max.x, localBox.min.y, localBox.min.z),
        glm::vec3(localBox.min.x, localBox.max.y, localBox.min.z),
        glm::vec3(localBox.max.x, localBox.max.y, localBox.min.z),
        glm::vec3(localBox.min.x, localBox.min.y, localBox.max.z),
        glm::vec3(localBox.max.x, localBox.min.y, localBox.max.z),
        glm::vec3(localBox.min.x, localBox.max.y, localBox.max.z),
        glm::vec3(localBox.max.x, localBox.max.y, localBox.max.z)
    };

    glm::vec3 firstTransformed = glm::vec3(worldMat * glm::vec4(corners[0], 1.0f));
    worldBox.min = firstTransformed;
    worldBox.max = firstTransformed;

    for (int i = 1; i < 8; ++i) {
        glm::vec3 trans = glm::vec3(worldMat * glm::vec4(corners[i], 1.0f));
        worldBox.min = glm::min(worldBox.min, trans);
        worldBox.max = glm::max(worldBox.max, trans);
    }

    return worldBox;
}

void SceneObject::update(float deltaTime) {
    // Smooth animation if offsets are defined
    if (glm::length(openPositionOffset) > 0.0001f || glm::length(openRotationOffset) > 0.0001f) {
        if (isOpen) {
            animProgress = std::min(1.0f, animProgress + animSpeed * deltaTime);
        } else {
            animProgress = std::max(0.0f, animProgress - animSpeed * deltaTime);
        }

        // Smooth cubic ease in-out
        float t = animProgress * animProgress * (3.0f - 2.0f * animProgress);
        setPosition(basePosition + openPositionOffset * t);
        setRotation(baseRotation + openRotationOffset * t);
    }

    for (auto& child : children) {
        if (child) {
            child->update(deltaTime);
        }
    }
}

void SceneObject::draw(const Shader& shader, const std::string& hoveredId, const std::string& selectedId, RenderLayer passLayer) const {
    if (!isVisible) return;

    // Filter by RenderLayer:
    // If drawing real world only, skip mirror-only objects
    if (passLayer == RenderLayer::RealOnly && renderLayer == RenderLayer::MirrorOnly) return;
    // If drawing planar mirror reflection pass only, skip real-only objects
    if (passLayer == RenderLayer::MirrorOnly && renderLayer == RenderLayer::RealOnly) return;

    if (mesh && primitiveType != PrimitiveType::None) {
        glm::mat4 model = getWorldMatrix();
        glm::mat3 normalMat = getNormalMatrix();

        shader.setMat4("model", model);
        shader.setMat3("normalMatrix", normalMat);

        // Resolve effective PBR material parameters
        glm::vec3 albedo = material.getEffectiveAlbedo();
        float metallic = material.getEffectiveMetallic();
        float roughness = material.getEffectiveRoughness();
        float ao = material.ao;

        shader.setVec3("material.albedoColor", albedo);
        shader.setFloat("material.metallic", metallic);
        shader.setFloat("material.roughness", roughness);
        shader.setFloat("material.ao", ao);
        shader.setVec3("material.emissiveColor", material.emissiveColor);
        shader.setFloat("material.emissiveIntensity", material.emissiveIntensity);
        shader.setBool("material.isFluorescent", material.isFluorescent);
        shader.setVec3("material.fluorescentColor", material.fluorescentColor);
        shader.setBool("material.uIsUVReactive", material.uIsUVReactive);

        // Resolve maps with seamless fallback to diffuse/textureId
        unsigned int albedoTex = material.albedoMap ? material.albedoMap : (material.useTexture ? material.textureId : 0);
        bool useAlbedo = (material.useAlbedoMap || material.useTexture) && albedoTex != 0;
        shader.setBool("material.useAlbedoMap", useAlbedo);
        if (useAlbedo) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, albedoTex);
            shader.setInt("material.albedoMap", 0);
        }

        bool useNormal = material.useNormalMap && material.normalMap != 0;
        shader.setBool("material.useNormalMap", useNormal);
        if (useNormal) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, material.normalMap);
            shader.setInt("material.normalMap", 1);
        }

        bool useMetallic = material.useMetallicMap && material.metallicMap != 0;
        shader.setBool("material.useMetallicMap", useMetallic);
        if (useMetallic) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, material.metallicMap);
            shader.setInt("material.metallicMap", 2);
        }

        bool useRoughness = material.useRoughnessMap && material.roughnessMap != 0;
        shader.setBool("material.useRoughnessMap", useRoughness);
        if (useRoughness) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, material.roughnessMap);
            shader.setInt("material.roughnessMap", 3);
        }

        bool useAo = material.useAoMap && material.aoMap != 0;
        shader.setBool("material.useAoMap", useAo);
        if (useAo) {
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, material.aoMap);
            shader.setInt("material.aoMap", 4);
        }

        bool useEmissive = material.useEmissiveMap && material.emissiveMap != 0;
        shader.setBool("material.useEmissiveMap", useEmissive);
        if (useEmissive) {
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, material.emissiveMap);
            shader.setInt("material.emissiveMap", 5);
        }

        // Legacy Blinn-Phong fallback uniforms
        shader.setVec3("material.diffuseColor", material.diffuseColor);
        shader.setVec3("material.specularColor", material.specularColor);
        shader.setFloat("material.shininess", material.shininess);
        shader.setBool("material.useTexture", material.useTexture && material.textureId != 0);
        if (material.useTexture && material.textureId != 0) {
            shader.setInt("material.diffuseTexture", 0);
        }

        // Highlight logic
        if (id == selectedId && !selectedId.empty()) {
            shader.setVec3("highlightColor", glm::vec3(0.2f, 0.7f, 1.0f)); // Cyan for Inspector selection
            shader.setFloat("highlightFactor", 0.5f);
        } else if (id == hoveredId && !hoveredId.empty()) {
            shader.setVec3("highlightColor", glm::vec3(1.0f, 0.9f, 0.2f)); // Warm gold for hover
            shader.setFloat("highlightFactor", 0.35f);
        } else {
            shader.setFloat("highlightFactor", 0.0f);
        }

        mesh->draw();
    }

    for (const auto& child : children) {
        if (child) {
            child->draw(shader, hoveredId, selectedId, passLayer);
        }
    }
}

std::shared_ptr<SceneObject> SceneObject::findById(const std::string& targetId) {
    if (id == targetId) {
        return shared_from_this();
    }
    for (auto& child : children) {
        if (child) {
            auto found = child->findById(targetId);
            if (found) return found;
        }
    }
    return nullptr;
}

void SceneObject::collectAll(std::vector<std::shared_ptr<SceneObject>>& outList) {
    outList.push_back(shared_from_this());
    for (auto& child : children) {
        if (child) {
            child->collectAll(outList);
        }
    }
}
