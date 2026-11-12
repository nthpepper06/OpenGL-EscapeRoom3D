#include "scene/SceneSerializer.hpp"
#include "renderer/PrimitiveGenerator.hpp"
#include <fstream>
#include <iomanip>

using json = nlohmann::json;

static std::string primitiveTypeToString(PrimitiveType type) {
    switch (type) {
        case PrimitiveType::Cube: return "Cube";
        case PrimitiveType::Cylinder: return "Cylinder";
        case PrimitiveType::Plane: return "Plane";
        default: return "None";
    }
}

static PrimitiveType stringToPrimitiveType(const std::string& str) {
    if (str == "Cube") return PrimitiveType::Cube;
    if (str == "Cylinder") return PrimitiveType::Cylinder;
    if (str == "Plane") return PrimitiveType::Plane;
    return PrimitiveType::None;
}

static std::string interactTypeToString(InteractionType type) {
    switch (type) {
        case InteractionType::LightSwitch: return "LightSwitch";
        case InteractionType::DeskDrawer: return "DeskDrawer";
        case InteractionType::SecretNote: return "SecretNote";
        case InteractionType::SafeKeypad: return "SafeKeypad";
        case InteractionType::SafeDoor: return "SafeDoor";
        case InteractionType::GoldenKey: return "GoldenKey";
        case InteractionType::ExitDoor: return "ExitDoor";
        default: return "None";
    }
}

static InteractionType stringToInteractType(const std::string& str) {
    if (str == "LightSwitch") return InteractionType::LightSwitch;
    if (str == "DeskDrawer") return InteractionType::DeskDrawer;
    if (str == "SecretNote") return InteractionType::SecretNote;
    if (str == "SafeKeypad") return InteractionType::SafeKeypad;
    if (str == "SafeDoor") return InteractionType::SafeDoor;
    if (str == "GoldenKey") return InteractionType::GoldenKey;
    if (str == "ExitDoor") return InteractionType::ExitDoor;
    return InteractionType::None;
}

static json serializeObject(const std::shared_ptr<SceneObject>& obj) {
    json j;
    j["id"] = obj->id;
    j["name"] = obj->name;
    j["primitiveType"] = primitiveTypeToString(obj->primitiveType);

    j["position"] = { obj->position.x, obj->position.y, obj->position.z };
    j["rotation"] = { obj->rotation.x, obj->rotation.y, obj->rotation.z };
    j["scale"] = { obj->scale.x, obj->scale.y, obj->scale.z };
    j["isVisible"] = obj->isVisible;

    j["material"] = {
        {"diffuse", { obj->material.diffuseColor.r, obj->material.diffuseColor.g, obj->material.diffuseColor.b }},
        {"specular", { obj->material.specularColor.r, obj->material.specularColor.g, obj->material.specularColor.b }},
        {"shininess", obj->material.shininess},
        {"useTexture", obj->material.useTexture}
    };

    j["interactType"] = interactTypeToString(obj->interactType);
    j["isOpen"] = obj->isOpen;
    j["isLocked"] = obj->isLocked;

    j["openPositionOffset"] = { obj->openPositionOffset.x, obj->openPositionOffset.y, obj->openPositionOffset.z };
    j["openRotationOffset"] = { obj->openRotationOffset.x, obj->openRotationOffset.y, obj->openRotationOffset.z };

    j["children"] = json::array();
    for (const auto& child : obj->children) {
        if (child) {
            j["children"].push_back(serializeObject(child));
        }
    }

    return j;
}

static std::shared_ptr<SceneObject> deserializeObject(const json& j) {
    std::string id = j.value("id", "obj");
    std::string name = j.value("name", "Object");
    PrimitiveType pType = stringToPrimitiveType(j.value("primitiveType", "None"));

    auto obj = std::make_shared<SceneObject>(id, name, pType);

    if (pType == PrimitiveType::Cube)
        obj->mesh = PrimitiveGenerator::createCube();
    else if (pType == PrimitiveType::Cylinder)
        obj->mesh = PrimitiveGenerator::createCylinder(32);
    else if (pType == PrimitiveType::Plane)
        obj->mesh = PrimitiveGenerator::createPlane();

    if (j.contains("position") && j["position"].is_array() && j["position"].size() == 3) {
        obj->position = glm::vec3(j["position"][0], j["position"][1], j["position"][2]);
        obj->basePosition = obj->position;
    }

    if (j.contains("rotation") && j["rotation"].is_array() && j["rotation"].size() == 3) {
        obj->rotation = glm::vec3(j["rotation"][0], j["rotation"][1], j["rotation"][2]);
        obj->baseRotation = obj->rotation;
    }

    if (j.contains("scale") && j["scale"].is_array() && j["scale"].size() == 3) {
        obj->scale = glm::vec3(j["scale"][0], j["scale"][1], j["scale"][2]);
    }

    obj->isVisible = j.value("isVisible", true);

    if (j.contains("material")) {
        auto mat = j["material"];
        if (mat.contains("diffuse") && mat["diffuse"].size() == 3) {
            obj->material.diffuseColor = glm::vec3(mat["diffuse"][0], mat["diffuse"][1], mat["diffuse"][2]);
        }
        if (mat.contains("specular") && mat["specular"].size() == 3) {
            obj->material.specularColor = glm::vec3(mat["specular"][0], mat["specular"][1], mat["specular"][2]);
        }
        obj->material.shininess = mat.value("shininess", 32.0f);
        obj->material.useTexture = mat.value("useTexture", false);
    }

    obj->interactType = stringToInteractType(j.value("interactType", "None"));
    obj->isOpen = j.value("isOpen", false);
    obj->isLocked = j.value("isLocked", false);

    if (j.contains("openPositionOffset") && j["openPositionOffset"].size() == 3) {
        obj->openPositionOffset = glm::vec3(j["openPositionOffset"][0], j["openPositionOffset"][1], j["openPositionOffset"][2]);
    }

    if (j.contains("openRotationOffset") && j["openRotationOffset"].size() == 3) {
        obj->openRotationOffset = glm::vec3(j["openRotationOffset"][0], j["openRotationOffset"][1], j["openRotationOffset"][2]);
    }

    if (j.contains("children") && j["children"].is_array()) {
        for (const auto& childJson : j["children"]) {
            auto childObj = deserializeObject(childJson);
            obj->addChild(childObj);
        }
    }

    return obj;
}

bool SceneSerializer::saveToFile(const std::string& filepath,
                                 const std::shared_ptr<SceneObject>& root,
                                 const PointLight& pointLight,
                                 const SpotLight& spotLight,
                                 const AmbientLight& ambientLight) {
    if (!root) return false;

    json rootJson;
    rootJson["pointLight"] = {
        {"position", { pointLight.position.x, pointLight.position.y, pointLight.position.z }},
        {"color", { pointLight.color.r, pointLight.color.g, pointLight.color.b }},
        {"intensity", pointLight.intensity},
        {"enabled", pointLight.enabled}
    };

    rootJson["spotLight"] = {
        {"color", { spotLight.color.r, spotLight.color.g, spotLight.color.b }},
        {"intensity", spotLight.intensity},
        {"enabled", spotLight.enabled}
    };

    rootJson["ambient"] = {
        {"color", { ambientLight.color.r, ambientLight.color.g, ambientLight.color.b }},
        {"intensity", ambientLight.intensity}
    };

    rootJson["scene"] = serializeObject(root);

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to save scene to: " << filepath << std::endl;
        return false;
    }

    file << std::setw(2) << rootJson << std::endl;
    file.close();
    std::cout << "Successfully saved scene to " << filepath << std::endl;
    return true;
}

bool SceneSerializer::loadFromFile(const std::string& filepath,
                                   std::shared_ptr<SceneObject>& root,
                                   PointLight& pointLight,
                                   SpotLight& spotLight,
                                   AmbientLight& ambientLight) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open scene file: " << filepath << std::endl;
        return false;
    }

    json rootJson;
    try {
        file >> rootJson;
    } catch (const std::exception& e) {
        std::cerr << "JSON Parse error: " << e.what() << std::endl;
        return false;
    }
    file.close();

    if (rootJson.contains("pointLight")) {
        auto pl = rootJson["pointLight"];
        if (pl.contains("position") && pl["position"].size() == 3)
            pointLight.position = glm::vec3(pl["position"][0], pl["position"][1], pl["position"][2]);
        if (pl.contains("color") && pl["color"].size() == 3)
            pointLight.color = glm::vec3(pl["color"][0], pl["color"][1], pl["color"][2]);
        pointLight.intensity = pl.value("intensity", pointLight.intensity);
        pointLight.enabled = pl.value("enabled", pointLight.enabled);
    }

    if (rootJson.contains("spotLight")) {
        auto sl = rootJson["spotLight"];
        if (sl.contains("color") && sl["color"].size() == 3)
            spotLight.color = glm::vec3(sl["color"][0], sl["color"][1], sl["color"][2]);
        spotLight.intensity = sl.value("intensity", spotLight.intensity);
        spotLight.enabled = sl.value("enabled", spotLight.enabled);
    }

    if (rootJson.contains("ambient")) {
        auto amb = rootJson["ambient"];
        if (amb.contains("color") && amb["color"].size() == 3)
            ambientLight.color = glm::vec3(amb["color"][0], amb["color"][1], amb["color"][2]);
        ambientLight.intensity = amb.value("intensity", ambientLight.intensity);
    }

    if (rootJson.contains("scene")) {
        root = deserializeObject(rootJson["scene"]);
        std::cout << "Successfully loaded scene from " << filepath << std::endl;
        return true;
    }

    return false;
}
