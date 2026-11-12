#include "rooms/OccultRitualRoom.hpp"
#include "renderer/PrimitiveGenerator.hpp"
#include "renderer/Texture.hpp"
#include "renderer/MeshBatcher.hpp"
#include "scene/EscapeRoomBuilder.hpp"

OccultRitualRoom::OccultRitualRoom() {
    theme = RoomTheme::OccultRitual;
    name = "Occult Ritual Chamber";
    description = "A grim subterranean stone crypt covered in blood runes and burning candles.";
    jsonFileName = "occult_room.json";
    cluePin = "6661";

    spawnPosition = glm::vec3(0.0f, 1.7f, 2.8f);
    spawnYaw = -90.0f;
    spawnPitch = 0.0f;

    // Atmospheric horror lighting (Eerie amber/red flickering candle light retuned for HDR)
    pointLight.position = glm::vec3(0.0f, 2.6f, -1.0f);
    pointLight.color = glm::vec3(1.0f, 0.45f, 0.20f);
    pointLight.baseIntensity = 9.5f;
    pointLight.intensity = 9.5f;
    pointLight.constant = 1.0f;
    pointLight.linear = 0.14f;
    pointLight.quadratic = 0.07f;
    pointLight.flickerEnabled = true;
    pointLight.flickerSpeed = 7.5f;
    pointLight.flickerNoiseDepth = 0.40f;

    ambientLight.color = glm::vec3(0.25f, 0.08f, 0.08f);
    ambientLight.intensity = 0.25f;
    iblIntensity = 1.15f;
    ambientAO = 1.0f;
    fogColor = glm::vec3(0.12f, 0.04f, 0.04f);
    fogDensity = 0.055f;
    fogHeightFalloff = 0.35f;
    inscatterIntensity = 1.0f;

    fearSourcePosition = glm::vec3(0.0f, 0.8f, -2.5f);
    fearDistanceThreshold = 3.2f;
    baseVignette = 0.50f;
    baseChromatic = 0.004f;
}

void OccultRitualRoom::build(InteractionManager& interactionMgr) {
    auto cubeMesh = PrimitiveGenerator::createCube();
    auto cylinderMesh = PrimitiveGenerator::createCylinder(32);
    auto planeMesh = PrimitiveGenerator::createPlane();

    // Procedural PBR Texture Sets
    PbrTextureSet stonePbr = Texture::createStoneBrickPbr(512, 512);
    PbrTextureSet ritualPbr = Texture::createRitualCirclePbr(512, 512);
    PbrTextureSet woodPbr = Texture::createWoodPbr(512, 512);
    PbrTextureSet metalPbr = Texture::createMetalPbr(512, 512);
    unsigned int noteTex = Texture::createNoteTexture("RITUAL CIPHER", "PIN: " + cluePin, 256, 256);

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
    // 1. ROOM SHELL (10m x 4.5m x 10m)
    // ==========================================
    // Floor with Blood Pentagram Rune
    auto floor = std::make_shared<SceneObject>("floor_occult", "Ritual Floor", PrimitiveType::Plane);
    floor->mesh = planeMesh;
    floor->position = glm::vec3(0.0f, 0.0f, 0.0f);
    floor->scale = glm::vec3(10.0f, 1.0f, 10.0f);
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

    // Ceiling
    auto ceiling = std::make_shared<SceneObject>("ceiling_occult", "Crypt Ceiling", PrimitiveType::Cube);
    ceiling->mesh = cubeMesh;
    ceiling->position = glm::vec3(0.0f, 4.5f, 0.0f);
    ceiling->scale = glm::vec3(10.0f, 0.2f, 10.0f);
    applyStonePbr(ceiling, glm::vec3(0.45f, 0.42f, 0.40f));
    rootNode->addChild(ceiling);

    // Back Wall (-Z)
    auto backWall = std::make_shared<SceneObject>("back_wall_occult", "Crypt Back Wall", PrimitiveType::Cube);
    backWall->mesh = cubeMesh;
    backWall->position = glm::vec3(0.0f, 2.25f, -5.0f);
    backWall->scale = glm::vec3(10.0f, 4.5f, 0.2f);
    applyStonePbr(backWall, glm::vec3(0.85f));

    // Front Wall (+Z)
    auto frontWall = std::make_shared<SceneObject>("front_wall_occult", "Crypt Front Wall", PrimitiveType::Cube);
    frontWall->mesh = cubeMesh;
    frontWall->position = glm::vec3(0.0f, 2.25f, 5.0f);
    frontWall->scale = glm::vec3(10.0f, 4.5f, 0.2f);
    applyStonePbr(frontWall, glm::vec3(0.85f));

    // Left Wall (-X)
    auto leftWall = std::make_shared<SceneObject>("left_wall_occult", "Crypt Left Wall", PrimitiveType::Cube);
    leftWall->mesh = cubeMesh;
    leftWall->position = glm::vec3(-5.0f, 2.25f, 0.0f);
    leftWall->scale = glm::vec3(0.2f, 4.5f, 10.0f);
    applyStonePbr(leftWall, glm::vec3(0.85f));

    // Right Wall (+X)
    auto rightWall = std::make_shared<SceneObject>("right_wall_occult", "Crypt Right Wall", PrimitiveType::Cube);
    rightWall->mesh = cubeMesh;
    rightWall->position = glm::vec3(5.0f, 2.25f, 0.0f);
    rightWall->scale = glm::vec3(0.2f, 4.5f, 10.0f);
    applyStonePbr(rightWall, glm::vec3(0.85f));

    // Static Mesh Batching (Phase 3: 4 walls batched into 1 draw call)
    std::vector<std::shared_ptr<SceneObject>> staticWalls = { backWall, frontWall, leftWall, rightWall };
    auto batchedWalls = MeshBatcher::batchObjects("walls_occult_batched", "Crypt Stone Walls", staticWalls, backWall->material);
    if (batchedWalls) {
        rootNode->addChild(batchedWalls);
    } else {
        for (auto& w : staticWalls) rootNode->addChild(w);
    }

    // 4 Crypt Stone Pillars
    glm::vec3 pillarPositions[4] = {
        {-3.5f, 2.25f, -3.5f},
        { 3.5f, 2.25f, -3.5f},
        {-3.5f, 2.25f,  3.5f},
        { 3.5f, 2.25f,  3.5f}
    };
    for (int i = 0; i < 4; ++i) {
        auto pillar = std::make_shared<SceneObject>("pillar_occult_" + std::to_string(i+1), "Stone Pillar", PrimitiveType::Cylinder);
        pillar->mesh = cylinderMesh;
        pillar->position = pillarPositions[i];
        pillar->scale = glm::vec3(0.45f, 4.5f, 0.45f);
        applyStonePbr(pillar, glm::vec3(0.7f));
        rootNode->addChild(pillar);
    }

    // ==========================================
    // 2. SACRIFICIAL ALTAR & CANDLES
    // ==========================================
    auto altarRoot = std::make_shared<SceneObject>("altar_root", "Sacrificial Altar Root", PrimitiveType::None);
    altarRoot->position = glm::vec3(0.0f, 0.0f, -2.5f);
    rootNode->addChild(altarRoot);

    auto altarBase = std::make_shared<SceneObject>("altar_base", "Altar Slab", PrimitiveType::Cube);
    altarBase->mesh = PrimitiveGenerator::createBeveledCube(2.6f, 0.9f, 1.4f, 0.05f, 2);
    altarBase->position = glm::vec3(0.0f, 0.45f, 0.0f);
    altarBase->scale = glm::vec3(1.0f, 1.0f, 1.0f);
    applyStonePbr(altarBase, glm::vec3(0.35f, 0.30f, 0.32f));
    altarRoot->addChild(altarBase);

    // Skull on Altar
    auto skull = std::make_shared<SceneObject>("altar_skull", "Occult Skull", PrimitiveType::Cube);
    skull->mesh = cubeMesh;
    skull->position = glm::vec3(0.0f, 1.05f, 0.0f);
    skull->scale = glm::vec3(0.35f, 0.32f, 0.40f);
    skull->material.albedoColor = glm::vec3(0.85f, 0.82f, 0.70f);
    skull->material.roughness = 0.65f;
    skull->material.diffuseColor = glm::vec3(0.85f, 0.82f, 0.70f);
    altarRoot->addChild(skull);

    // 4 Bloody Candles on Altar Corners
    glm::vec3 candleOffsets[4] = {
        {-1.05f, 1.05f, -0.45f},
        { 1.05f, 1.05f, -0.45f},
        {-1.05f, 1.05f,  0.45f},
        { 1.05f, 1.05f,  0.45f}
    };
    for (int i = 0; i < 4; ++i) {
        auto candle = std::make_shared<SceneObject>("candle_" + std::to_string(i+1), "Ritual Candle", PrimitiveType::Cylinder);
        candle->mesh = cylinderMesh;
        candle->position = candleOffsets[i];
        candle->scale = glm::vec3(0.06f, 0.35f, 0.06f);
        candle->material.albedoColor = glm::vec3(0.75f, 0.12f, 0.15f);
        candle->material.roughness = 0.35f; // Glossy wax
        candle->material.diffuseColor = glm::vec3(0.75f, 0.12f, 0.15f); // Red wax
        altarRoot->addChild(candle);

        // Candle Flame (Glowing Emissive Source)
        auto flame = std::make_shared<SceneObject>("flame_" + std::to_string(i+1), "Candle Flame", PrimitiveType::Cube);
        flame->mesh = cubeMesh;
        flame->position = candleOffsets[i] + glm::vec3(0.0f, 0.22f, 0.0f);
        flame->scale = glm::vec3(0.04f, 0.08f, 0.04f);
        flame->material.albedoColor = glm::vec3(1.0f, 0.85f, 0.3f);
        flame->material.emissiveColor = glm::vec3(1.0f, 0.85f, 0.3f);
        flame->material.emissiveIntensity = 4.0f; // Radiant HDR flame
        flame->material.diffuseColor = glm::vec3(1.0f, 0.8f, 0.2f);
        altarRoot->addChild(flame);
    }

    // ==========================================
    // 3. OFFERING CHEST & SLIDING DRAWER (PUZZLE)
    // ==========================================
    auto chestRoot = std::make_shared<SceneObject>("chest_root", "Offering Chest", PrimitiveType::Cube);
    chestRoot->mesh = cubeMesh;
    chestRoot->position = glm::vec3(-3.2f, 0.45f, -1.0f);
    chestRoot->scale = glm::vec3(1.1f, 0.9f, 1.3f);
    chestRoot->material.albedoColor = glm::vec3(0.45f, 0.25f, 0.15f);
    chestRoot->material.albedoMap = woodPbr.albedo;
    chestRoot->material.normalMap = woodPbr.normal;
    chestRoot->material.roughnessMap = woodPbr.roughness;
    chestRoot->material.useAlbedoMap = true;
    chestRoot->material.useNormalMap = true;
    chestRoot->material.useRoughnessMap = true;
    chestRoot->material.useTexture = true;
    chestRoot->material.textureId = woodPbr.albedo;
    rootNode->addChild(chestRoot);

    // Sliding Drawer Container (Interactive)
    auto drawer = std::make_shared<SceneObject>("desk_drawer", "Offering Drawer", PrimitiveType::None);
    drawer->position = glm::vec3(0.0f, 0.0f, 0.05f);
    drawer->basePosition = drawer->position;
    drawer->openPositionOffset = glm::vec3(0.0f, 0.0f, 0.85f); // Slides forward
    drawer->scale = glm::vec3(0.9f, 0.4f, 1.0f);
    drawer->interactType = InteractionType::DeskDrawer;
    chestRoot->addChild(drawer);
    interactionMgr.drawerObject = drawer;

    // Drawer Bottom Base
    auto drawerBottom = std::make_shared<SceneObject>("drawer_bottom", "Drawer Base", PrimitiveType::Cube);
    drawerBottom->mesh = cubeMesh;
    drawerBottom->position = glm::vec3(0.0f, -0.15f, 0.0f);
    drawerBottom->scale = glm::vec3(0.80f, 0.04f, 0.95f);
    drawerBottom->material.diffuseColor = glm::vec3(0.40f, 0.22f, 0.12f);
    drawerBottom->material.useTexture = true;
    drawerBottom->material.textureId = woodPbr.albedo;
    drawer->addChild(drawerBottom);

    // Drawer Front Panel
    auto drawerFront = std::make_shared<SceneObject>("drawer_front", "Drawer Front", PrimitiveType::Cube);
    drawerFront->mesh = cubeMesh;
    drawerFront->position = glm::vec3(0.0f, 0.0f, 0.45f);
    drawerFront->scale = glm::vec3(0.85f, 0.35f, 0.04f);
    drawerFront->material.diffuseColor = glm::vec3(0.48f, 0.25f, 0.14f);
    drawerFront->material.useTexture = true;
    drawerFront->material.textureId = woodPbr.albedo;
    drawerFront->interactType = InteractionType::DeskDrawer;
    drawer->addChild(drawerFront);

    // Drawer Handle
    auto drawerHandle = std::make_shared<SceneObject>("drawer_handle", "Drawer Handle", PrimitiveType::Cube);
    drawerHandle->mesh = cubeMesh;
    drawerHandle->position = glm::vec3(0.0f, 0.0f, 0.05f);
    drawerHandle->scale = glm::vec3(0.25f, 0.05f, 0.05f);
    drawerHandle->material.albedoColor = glm::vec3(0.85f, 0.70f, 0.2f);
    drawerHandle->material.metallic = 0.95f;
    drawerHandle->material.roughness = 0.22f;
    drawerHandle->material.diffuseColor = glm::vec3(0.8f, 0.65f, 0.2f);
    drawerFront->addChild(drawerHandle);

    // Clue Note inside Drawer
    auto clueNote = std::make_shared<SceneObject>("clue_note", "Ritual Cipher Note", PrimitiveType::Cube);
    clueNote->mesh = cubeMesh;
    clueNote->position = glm::vec3(0.0f, -0.11f, 0.05f);
    clueNote->scale = glm::vec3(0.55f, 0.02f, 0.40f);
    clueNote->material.diffuseColor = glm::vec3(1.0f);
    clueNote->material.useTexture = true;
    clueNote->material.textureId = noteTex;
    clueNote->interactType = InteractionType::SecretNote;
    drawer->addChild(clueNote);

    // ==========================================
    // 4. HEAVY CRYPT VAULT SAFE & ORNATE GOLDEN KEY (PUZZLE)
    // ==========================================
    auto safe = EscapeRoomBuilder::buildHeavyVaultSafe(interactionMgr, "occult_safe",
        glm::vec3(3.2f, 0.0f, -1.0f), 0.0f, metalPbr.albedo, stonePbr.albedo,
        glm::vec3(0.18f, 0.16f, 0.20f), glm::vec3(0.92f, 0.85f, 0.65f));
    rootNode->addChild(safe);

    // ==========================================
    // 5. HIGH-VISIBILITY ILLUMINATED EXIT DOOR
    // ==========================================
    auto exitPortal = EscapeRoomBuilder::buildExitDoor(
        interactionMgr, "occult_door",
        glm::vec3(0.0f, 0.0f, 4.88f), 0.0f,
        stonePbr.albedo, metalPbr.albedo,
        glm::vec3(0.38f, 0.35f, 0.38f),
        glm::vec3(0.36f, 0.15f, 0.15f), // Crimson demonic door
        glm::vec3(1.0f, 0.35f, 0.10f)  // Fiery amber glowing EXIT sign
    );
    rootNode->addChild(exitPortal);

    // Set correct PIN in InteractionManager
    interactionMgr.correctSafePin = cluePin;
}
