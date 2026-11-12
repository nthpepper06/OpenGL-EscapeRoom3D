#include "rooms/OccultRitualRoom.hpp"
#include "renderer/PrimitiveGenerator.hpp"
#include "renderer/Texture.hpp"
#include "renderer/MeshBatcher.hpp"
#include "scene/EscapeRoomBuilder.hpp"

OccultRitualRoom::OccultRitualRoom() {
    theme = RoomTheme::OccultRitual;
    name = "Occult Ritual Grand Chamber";
    description = "A colossal subterranean stone crypt with 4 towering pillars, carved shadow-totems, and a sealed stone altar.";
    jsonFileName = "occult_room.json";
    cluePin = "6661";

    // New 12m x 12m spatial dimensions
    roomBounds = glm::vec4(-6.0f, 6.0f, -6.0f, 6.0f);
    spawnPosition = glm::vec3(0.0f, 1.7f, 4.2f);
    spawnYaw = -90.0f;
    spawnPitch = 0.0f;

    // Atmospheric horror lighting (Eerie amber/red flickering candle light retuned for HDR)
    pointLight.position = glm::vec3(0.0f, 3.8f, 1.2f);
    pointLight.color = glm::vec3(1.0f, 0.45f, 0.20f);
    pointLight.baseIntensity = 12.0f;
    pointLight.intensity = 12.0f;
    pointLight.constant = 1.0f;
    pointLight.linear = 0.10f;
    pointLight.quadratic = 0.04f;
    pointLight.flickerEnabled = true;
    pointLight.flickerSpeed = 7.5f;
    pointLight.flickerNoiseDepth = 0.40f;

    ambientLight.color = glm::vec3(0.25f, 0.08f, 0.08f);
    ambientLight.intensity = 0.25f;
    iblIntensity = 1.15f;
    ambientAO = 1.0f;
    fogColor = glm::vec3(0.12f, 0.04f, 0.04f);
    fogDensity = 0.045f;
    fogHeightFalloff = 0.35f;
    inscatterIntensity = 1.0f;

    fearSourcePosition = glm::vec3(0.0f, 0.8f, 0.0f);
    fearDistanceThreshold = 4.0f;
    baseVignette = 0.50f;
    baseChromatic = 0.004f;
}

std::string OccultRitualRoom::getObjectiveHint(const InteractionManager& im) const {
    if (im.hasGoldenKey) {
        return "Ritual Key acquired! Proceed to the South Exit Door to continue.";
    }
    if (im.altarUnlocked) {
        return "Altar slab has slid open! Retrieve the Ritual Key & Sigil Fragment #1 from the crypt altar.";
    }
    return "Rotate 3 Totems [E] so their projected shadows align with the 3 Runes on the North wall.";
}

void OccultRitualRoom::build(InteractionManager& interactionMgr) {
    auto cubeMesh = PrimitiveGenerator::createCube();
    auto cylinderMesh = PrimitiveGenerator::createCylinder(32);
    auto planeMesh = PrimitiveGenerator::createPlane();

    // Procedural PBR Texture Sets
    PbrTextureSet stonePbr = Texture::createStoneBrickPbr(512, 512);
    PbrTextureSet ritualPbr = Texture::createRitualCirclePbr(512, 512);
    PbrTextureSet metalPbr = Texture::createMetalPbr(512, 512);

    auto applyStonePbr = [&](const std::shared_ptr<SceneObject>& obj, const glm::vec3& tint = glm::vec3(1.0f)) {
        obj->material.albedoColor = tint;
        obj->material.albedoMap = stonePbr.albedo;
        obj->material.normalMap = stonePbr.normal;
        obj->material.roughnessMap = stonePbr.roughness;
        obj->material.aoMap = stonePbr.ao;
        obj->material.useAlbedoMap = true;
        obj->material.useNormalMap = true;
        obj->material.useRoughnessMap = true;
        obj->material.useAoMap = true;
        obj->material.useTexture = true;
        obj->material.textureId = stonePbr.albedo;
    };

    rootNode = std::make_shared<SceneObject>("room_occult_root", "Occult Ritual Room Root", PrimitiveType::None);

    // ==========================================
    // 1. EXPANDED ROOM SHELL (12m x 5.5m x 12m)
    // ==========================================
    // Floor with Blood Pentagram Rune
    auto floor = std::make_shared<SceneObject>("floor_occult", "Ritual Floor", PrimitiveType::Plane);
    floor->mesh = planeMesh;
    floor->position = glm::vec3(0.0f, 0.0f, 0.0f);
    floor->scale = glm::vec3(12.0f, 1.0f, 12.0f);
    floor->material.albedoColor = glm::vec3(1.0f);
    floor->material.albedoMap = ritualPbr.albedo;
    floor->material.normalMap = ritualPbr.normal;
    floor->material.roughnessMap = ritualPbr.roughness;
    floor->material.metallicMap = ritualPbr.metallic;
    floor->material.aoMap = ritualPbr.ao;
    floor->material.emissiveMap = ritualPbr.emissive;
    floor->material.emissiveIntensity = 1.6f;
    floor->material.useAlbedoMap = true;
    floor->material.useNormalMap = true;
    floor->material.useRoughnessMap = true;
    floor->material.useAoMap = true;
    floor->material.useEmissiveMap = true;
    floor->material.useTexture = true;
    floor->material.textureId = ritualPbr.albedo;
    rootNode->addChild(floor);

    // Ceiling (5.5m height)
    auto ceiling = std::make_shared<SceneObject>("ceiling_occult", "Crypt Vaulted Ceiling", PrimitiveType::Cube);
    ceiling->mesh = cubeMesh;
    ceiling->position = glm::vec3(0.0f, 5.5f, 0.0f);
    ceiling->scale = glm::vec3(12.0f, 0.2f, 12.0f);
    applyStonePbr(ceiling, glm::vec3(0.42f, 0.38f, 0.36f));
    rootNode->addChild(ceiling);

    // North Wall (-Z, with shadow target engravings)
    auto backWall = std::make_shared<SceneObject>("back_wall_occult", "Crypt North Wall", PrimitiveType::Cube);
    backWall->mesh = cubeMesh;
    backWall->position = glm::vec3(0.0f, 2.75f, -6.0f);
    backWall->scale = glm::vec3(12.0f, 5.5f, 0.2f);
    applyStonePbr(backWall, glm::vec3(0.85f));

    // South Wall (+Z, with exit door)
    auto frontWall = std::make_shared<SceneObject>("front_wall_occult", "Crypt South Wall", PrimitiveType::Cube);
    frontWall->mesh = cubeMesh;
    frontWall->position = glm::vec3(0.0f, 2.75f, 6.0f);
    frontWall->scale = glm::vec3(12.0f, 5.5f, 0.2f);
    applyStonePbr(frontWall, glm::vec3(0.85f));

    // West Wall (-X)
    auto leftWall = std::make_shared<SceneObject>("left_wall_occult", "Crypt West Wall", PrimitiveType::Cube);
    leftWall->mesh = cubeMesh;
    leftWall->position = glm::vec3(-6.0f, 2.75f, 0.0f);
    leftWall->scale = glm::vec3(0.2f, 5.5f, 12.0f);
    applyStonePbr(leftWall, glm::vec3(0.85f));

    // East Wall (+X)
    auto rightWall = std::make_shared<SceneObject>("right_wall_occult", "Crypt East Wall", PrimitiveType::Cube);
    rightWall->mesh = cubeMesh;
    rightWall->position = glm::vec3(6.0f, 2.75f, 0.0f);
    rightWall->scale = glm::vec3(0.2f, 5.5f, 12.0f);
    applyStonePbr(rightWall, glm::vec3(0.85f));

    // Batch static outer stone walls
    std::vector<std::shared_ptr<SceneObject>> staticWalls = { backWall, frontWall, leftWall, rightWall };
    auto batchedWalls = MeshBatcher::batchObjects("walls_occult_batched", "Crypt Stone Walls", staticWalls, backWall->material);
    if (batchedWalls) {
        rootNode->addChild(batchedWalls);
    } else {
        for (auto& w : staticWalls) rootNode->addChild(w);
    }

    // ==========================================
    // 2. 4 MASSIVE CENTRAL STONE PILLARS
    // ==========================================
    glm::vec3 pillarPositions[4] = {
        {-3.2f, 2.75f, -3.2f},
        { 3.2f, 2.75f, -3.2f},
        {-3.2f, 2.75f,  3.2f},
        { 3.2f, 2.75f,  3.2f}
    };
    for (int i = 0; i < 4; ++i) {
        auto pillar = std::make_shared<SceneObject>("pillar_occult_" + std::to_string(i+1), "Colossal Stone Pillar", PrimitiveType::Cylinder);
        pillar->mesh = cylinderMesh;
        pillar->position = pillarPositions[i];
        pillar->scale = glm::vec3(0.70f, 5.54f, 0.70f); // 5.54m height penetrates floor and ceiling to eliminate Z-fighting
        applyStonePbr(pillar, glm::vec3(0.75f));
        rootNode->addChild(pillar);
    }

    // ==========================================
    // 3. SACRIFICIAL ALTAR & HIDDEN KEY MECHANISM
    // ==========================================
    auto altarRoot = std::make_shared<SceneObject>("altar_root", "Sacrificial Altar Root", PrimitiveType::None);
    altarRoot->position = glm::vec3(0.0f, 0.0f, 0.0f);
    rootNode->addChild(altarRoot);

    auto altarBase = std::make_shared<SceneObject>("altar_base", "Altar Slab Base", PrimitiveType::Cube);
    altarBase->mesh = PrimitiveGenerator::createBeveledCube(3.2f, 0.9f, 1.8f, 0.06f, 2);
    altarBase->position = glm::vec3(0.0f, 0.45f, 0.0f);
    altarBase->scale = glm::vec3(1.0f, 1.0f, 1.0f);
    applyStonePbr(altarBase, glm::vec3(0.35f, 0.30f, 0.32f));
    altarRoot->addChild(altarBase);

    // Sliding Altar Lid (opens upon solving the shadow alignment puzzle)
    auto altarLid = std::make_shared<SceneObject>("altar_lid", "Altar Sliding Stone Slab", PrimitiveType::Cube);
    altarLid->mesh = PrimitiveGenerator::createBeveledCube(3.1f, 0.15f, 1.7f, 0.04f, 2);
    altarLid->position = glm::vec3(0.0f, 0.95f, 0.0f);
    altarLid->basePosition = altarLid->position;
    altarLid->openPositionOffset = glm::vec3(0.0f, 0.0f, -1.3f); // Slides back when puzzle solved
    applyStonePbr(altarLid, glm::vec3(0.40f, 0.34f, 0.36f));
    altarRoot->addChild(altarLid);
    interactionMgr.altarLidObject = altarLid;

    // Skull on Altar Lid (Weathered bone effigy)
    auto skull = std::make_shared<SceneObject>("altar_skull", "Occult Skull", PrimitiveType::Cube);
    skull->mesh = PrimitiveGenerator::createBeveledCube(0.32f, 0.28f, 0.36f, 0.03f, 2);
    skull->position = glm::vec3(0.0f, 0.18f, 0.0f);
    skull->scale = glm::vec3(1.0f);
    skull->material.albedoColor = glm::vec3(0.55f, 0.52f, 0.45f);
    skull->material.roughness = 0.75f;
    skull->material.metallic = 0.05f;
    altarLid->addChild(skull);

    // 4 Bloody Candles on Altar Corners
    glm::vec3 candleOffsets[4] = {
        {-1.25f, 0.20f, -0.65f},
        { 1.25f, 0.20f, -0.65f},
        {-1.25f, 0.20f,  0.65f},
        { 1.25f, 0.20f,  0.65f}
    };
    for (int i = 0; i < 4; ++i) {
        auto candle = std::make_shared<SceneObject>("candle_" + std::to_string(i+1), "Ritual Candle", PrimitiveType::Cylinder);
        candle->mesh = cylinderMesh;
        candle->position = candleOffsets[i];
        candle->scale = glm::vec3(0.06f, 0.35f, 0.06f);
        candle->material.albedoColor = glm::vec3(0.75f, 0.12f, 0.15f);
        candle->material.roughness = 0.35f;
        altarLid->addChild(candle);

        auto flame = std::make_shared<SceneObject>("flame_" + std::to_string(i+1), "Candle Flame", PrimitiveType::Cube);
        flame->mesh = cubeMesh;
        flame->position = candleOffsets[i] + glm::vec3(0.0f, 0.22f, 0.0f);
        flame->scale = glm::vec3(0.04f, 0.08f, 0.04f);
        flame->material.albedoColor = glm::vec3(1.0f, 0.85f, 0.3f);
        flame->material.emissiveColor = glm::vec3(1.0f, 0.85f, 0.3f);
        flame->material.emissiveIntensity = 4.0f;
        altarLid->addChild(flame);
    }

    // Ritual Golden Key inside the altar compartment
    auto ritualKey = EscapeRoomBuilder::buildOrnateGoldenKey(interactionMgr);
    ritualKey->position = glm::vec3(0.0f, 0.72f, 0.0f);
    ritualKey->scale = glm::vec3(0.24f);
    ritualKey->interactType = InteractionType::GoldenKey;
    altarRoot->addChild(ritualKey);
    interactionMgr.keyObject = ritualKey;

    // ==========================================
    // 4. SHADOW ALIGNMENT PUZZLE: 3 ROTATABLE TOTEMS
    // ==========================================
    glm::vec3 totemPositions[3] = {
        {-2.2f, 0.0f, -2.0f},
        { 0.0f, 0.0f, -2.8f},
        { 2.2f, 0.0f, -2.0f}
    };

    for (int i = 0; i < 3; ++i) {
        auto totemBase = std::make_shared<SceneObject>("totem_pedestal_" + std::to_string(i+1), "Totem Pedestal", PrimitiveType::Cylinder);
        totemBase->mesh = cylinderMesh;
        totemBase->position = totemPositions[i];
        totemBase->scale = glm::vec3(0.45f, 0.5f, 0.45f);
        applyStonePbr(totemBase, glm::vec3(0.5f));
        rootNode->addChild(totemBase);

        // Rotatable effigy top
        auto totemStatue = std::make_shared<SceneObject>("totem_" + std::to_string(i+1), "Rotatable Shadow Totem #" + std::to_string(i+1), PrimitiveType::Cube);
        totemStatue->mesh = PrimitiveGenerator::createBeveledCube(0.40f, 1.4f, 0.30f, 0.03f, 2);
        totemStatue->position = glm::vec3(0.0f, 0.95f, 0.0f);
        totemStatue->scale = glm::vec3(1.0f, 1.0f, 1.0f);
        totemStatue->interactType = InteractionType::Totem;
        totemStatue->rotation = glm::vec3(0.0f, interactionMgr.totemAngles[i], 0.0f);
        applyStonePbr(totemStatue, glm::vec3(0.80f, 0.70f, 0.65f));
        totemBase->addChild(totemStatue);

        // Distinct carving feature that casts directional shadows
        auto arm = std::make_shared<SceneObject>("totem_arm_" + std::to_string(i+1), "Totem Projection Arm", PrimitiveType::Cube);
        arm->mesh = cubeMesh;
        arm->position = glm::vec3(0.28f, 0.25f, 0.0f);
        arm->scale = glm::vec3(0.35f, 0.15f, 0.15f);
        arm->material.albedoColor = glm::vec3(0.4f, 0.2f, 0.2f);
        totemStatue->addChild(arm);
    }

    // 3 Target Rune Inscriptions on North Wall (-Z)
    glm::vec3 runePositions[3] = {
        {-2.2f, 2.4f, -5.88f},
        { 0.0f, 2.4f, -5.88f},
        { 2.2f, 2.4f, -5.88f}
    };
    for (int i = 0; i < 3; ++i) {
        auto rune = std::make_shared<SceneObject>("rune_target_" + std::to_string(i+1), "Shadow Target Rune", PrimitiveType::Cube);
        rune->mesh = cubeMesh;
        rune->position = runePositions[i];
        rune->scale = glm::vec3(0.9f, 0.9f, 0.04f);
        rune->material.albedoColor = glm::vec3(0.3f, 0.1f, 0.1f);
        rune->material.emissiveColor = glm::vec3(0.9f, 0.2f, 0.15f);
        rune->material.emissiveIntensity = 2.2f; // Glowing blood rune on wall
        rootNode->addChild(rune);
    }

    // ==========================================
    // 5. ILLUMINATED SOUTH EXIT DOOR
    // ==========================================
    auto exitPortal = EscapeRoomBuilder::buildExitDoor(
        interactionMgr, "occult_door",
        glm::vec3(0.0f, 0.0f, 5.88f), 0.0f,
        stonePbr.albedo, metalPbr.albedo,
        glm::vec3(0.38f, 0.35f, 0.38f),
        glm::vec3(0.36f, 0.15f, 0.15f), // Crimson demonic door
        glm::vec3(1.0f, 0.35f, 0.10f)  // Fiery amber glowing EXIT sign
    );
    rootNode->addChild(exitPortal);
}
