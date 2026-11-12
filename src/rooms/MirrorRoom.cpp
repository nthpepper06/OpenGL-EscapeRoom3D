#include "rooms/MirrorRoom.hpp"
#include "renderer/PrimitiveGenerator.hpp"
#include "renderer/Texture.hpp"
#include "renderer/MeshBatcher.hpp"
#include "scene/EscapeRoomBuilder.hpp"

MirrorRoom::MirrorRoom() {
    theme = RoomTheme::MirrorRoom;
    name = "Haunted Mirror Sanctuary";
    description = "An eerie Victorian salon lined with antique mirrors and ghostly spectral reflections.";
    jsonFileName = "mirror_room.json";
    cluePin = "8820";

    spawnPosition = glm::vec3(0.0f, 1.7f, 2.8f);
    spawnYaw = -90.0f;
    spawnPitch = 0.0f;

    // Ghostly pale lavender / spectral blue chandelier flicker retuned for HDR
    pointLight.position = glm::vec3(0.0f, 3.2f, 0.0f);
    pointLight.color = glm::vec3(0.75f, 0.65f, 1.0f);
    pointLight.baseIntensity = 10.0f;
    pointLight.intensity = 10.0f;
    pointLight.constant = 1.0f;
    pointLight.linear = 0.14f;
    pointLight.quadratic = 0.07f;
    pointLight.flickerEnabled = true;
    pointLight.flickerSpeed = 5.0f;
    pointLight.flickerNoiseDepth = 0.30f;

    ambientLight.color = glm::vec3(0.12f, 0.08f, 0.20f);
    ambientLight.intensity = 0.25f;
    iblIntensity = 1.35f;
    ambientAO = 1.0f;
    fogColor = glm::vec3(0.08f, 0.06f, 0.14f);
    fogDensity = 0.045f;
    fogHeightFalloff = 0.32f;
    inscatterIntensity = 0.9f;

    fearSourcePosition = glm::vec3(0.0f, 2.0f, -4.8f);
    fearDistanceThreshold = 3.2f;
    baseVignette = 0.50f;
    baseChromatic = 0.005f;
}

void MirrorRoom::build(InteractionManager& interactionMgr) {
    auto cubeMesh = PrimitiveGenerator::createCube();
    auto cylinderMesh = PrimitiveGenerator::createCylinder(32);
    auto planeMesh = PrimitiveGenerator::createPlane();

    // Procedural PBR Texture Sets
    PbrTextureSet mirrorPbr = Texture::createMirrorPbr(512, 512);
    PbrTextureSet woodPbr = Texture::createWoodPbr(512, 512);
    PbrTextureSet metalPbr = Texture::createMetalPbr(512, 512);
    unsigned int noteTex = Texture::createNoteTexture("MIRROR CURSE", "PIN: " + cluePin, 256, 256);

    auto applyMirrorPbr = [&](const std::shared_ptr<SceneObject>& obj) {
        obj->material.albedoColor = glm::vec3(1.0f);
        obj->material.albedoMap = mirrorPbr.albedo;
        obj->material.normalMap = mirrorPbr.normal;
        obj->material.roughnessMap = mirrorPbr.roughness;
        obj->material.metallicMap = mirrorPbr.metallic;
        obj->material.aoMap = mirrorPbr.ao;
        obj->material.useAlbedoMap = true;
        obj->material.useNormalMap = true;
        obj->material.useRoughnessMap = true;
        obj->material.useMetallicMap = true;
        obj->material.useAoMap = true;
        obj->material.useTexture = true;
        obj->material.textureId = mirrorPbr.albedo;
    };

    rootNode = std::make_shared<SceneObject>("room_mirror_root", "Mirror Room Root", PrimitiveType::None);

    // ==========================================
    // 1. MIRROR ROOM SHELL (10m x 4.2m x 10m)
    // ==========================================
    // Polished Dark Oak Floor
    auto floor = std::make_shared<SceneObject>("floor_mirror", "Polished Oak Floor", PrimitiveType::Plane);
    floor->mesh = planeMesh;
    floor->position = glm::vec3(0.0f, 0.0f, 0.0f);
    floor->scale = glm::vec3(10.0f, 1.0f, 10.0f);
    floor->material.albedoColor = glm::vec3(0.55f, 0.40f, 0.32f);
    floor->material.albedoMap = woodPbr.albedo;
    floor->material.normalMap = woodPbr.normal;
    floor->material.roughnessMap = woodPbr.roughness;
    floor->material.aoMap = woodPbr.ao;
    floor->material.useAlbedoMap = true;
    floor->material.useNormalMap = true;
    floor->material.useRoughnessMap = true;
    floor->material.useAoMap = true;
    floor->material.useTexture = true;
    floor->material.textureId = woodPbr.albedo;
    rootNode->addChild(floor);

    // Ceiling
    auto ceiling = std::make_shared<SceneObject>("ceiling_mirror", "Victorian Ceiling", PrimitiveType::Cube);
    ceiling->mesh = cubeMesh;
    ceiling->position = glm::vec3(0.0f, 4.2f, 0.0f);
    ceiling->scale = glm::vec3(10.0f, 0.2f, 10.0f);
    ceiling->material.albedoColor = glm::vec3(0.2f, 0.16f, 0.22f);
    ceiling->material.diffuseColor = glm::vec3(0.2f, 0.16f, 0.22f);
    rootNode->addChild(ceiling);

    // Chandelier hanging from ceiling
    auto chandelier = std::make_shared<SceneObject>("chandelier", "Ghost Chandelier", PrimitiveType::Cylinder);
    chandelier->mesh = cylinderMesh;
    chandelier->position = glm::vec3(0.0f, 3.6f, 0.0f);
    chandelier->scale = glm::vec3(0.8f, 0.6f, 0.8f);
    chandelier->material.albedoColor = glm::vec3(0.85f, 0.75f, 1.0f);
    chandelier->material.metallic = 0.92f;
    chandelier->material.roughness = 0.18f;
    chandelier->material.emissiveColor = glm::vec3(0.85f, 0.70f, 1.0f);
    chandelier->material.emissiveIntensity = 2.5f; // Eerie violet spectral glow
    chandelier->material.diffuseColor = glm::vec3(0.7f, 0.6f, 0.9f);
    rootNode->addChild(chandelier);

    // Back Wall (-Z)
    auto backWall = std::make_shared<SceneObject>("back_wall_mirror", "Salon Back Wall", PrimitiveType::Cube);
    backWall->mesh = cubeMesh;
    backWall->position = glm::vec3(0.0f, 2.1f, -5.0f);
    backWall->scale = glm::vec3(10.0f, 4.2f, 0.2f);
    backWall->material.albedoColor = glm::vec3(0.25f, 0.18f, 0.26f);
    backWall->material.diffuseColor = glm::vec3(0.25f, 0.18f, 0.26f);

    // Front Wall (+Z)
    auto frontWall = std::make_shared<SceneObject>("front_wall_mirror", "Salon Front Wall", PrimitiveType::Cube);
    frontWall->mesh = cubeMesh;
    frontWall->position = glm::vec3(0.0f, 2.1f, 5.0f);
    frontWall->scale = glm::vec3(10.0f, 4.2f, 0.2f);
    frontWall->material.albedoColor = glm::vec3(0.25f, 0.18f, 0.26f);
    frontWall->material.diffuseColor = glm::vec3(0.25f, 0.18f, 0.26f);

    // Left Wall (-X)
    auto leftWall = std::make_shared<SceneObject>("left_wall_mirror", "Salon Left Wall", PrimitiveType::Cube);
    leftWall->mesh = cubeMesh;
    leftWall->position = glm::vec3(-5.0f, 2.1f, 0.0f);
    leftWall->scale = glm::vec3(0.2f, 4.2f, 10.0f);
    leftWall->material.albedoColor = glm::vec3(0.25f, 0.18f, 0.26f);
    leftWall->material.diffuseColor = glm::vec3(0.25f, 0.18f, 0.26f);

    // Right Wall (+X)
    auto rightWall = std::make_shared<SceneObject>("right_wall_mirror", "Salon Right Wall", PrimitiveType::Cube);
    rightWall->mesh = cubeMesh;
    rightWall->position = glm::vec3(5.0f, 2.1f, 0.0f);
    rightWall->scale = glm::vec3(0.2f, 4.2f, 10.0f);
    rightWall->material.albedoColor = glm::vec3(0.25f, 0.18f, 0.26f);
    rightWall->material.diffuseColor = glm::vec3(0.25f, 0.18f, 0.26f);

    // Static Mesh Batching (Phase 3: 4 Victorian walls batched into 1 draw call)
    std::vector<std::shared_ptr<SceneObject>> staticWalls = { backWall, frontWall, leftWall, rightWall };
    auto batchedWalls = MeshBatcher::batchObjects("walls_mirror_batched", "Salon Victorian Walls", staticWalls, backWall->material);
    if (batchedWalls) {
        rootNode->addChild(batchedWalls);
    } else {
        for (auto& w : staticWalls) rootNode->addChild(w);
    }

    // ==========================================
    // 2. HAUNTED MIRRORS
    // ==========================================
    // Center Great Mirror on Back Wall
    auto centerMirror = std::make_shared<SceneObject>("center_mirror", "Haunted Grand Mirror", PrimitiveType::Cube);
    centerMirror->mesh = cubeMesh;
    centerMirror->position = glm::vec3(0.0f, 2.1f, -4.88f);
    centerMirror->scale = glm::vec3(3.2f, 2.6f, 0.08f);
    applyMirrorPbr(centerMirror);
    rootNode->addChild(centerMirror);

    // Left Wall Mirror
    auto leftMirror = std::make_shared<SceneObject>("left_mirror", "Left Silver Mirror", PrimitiveType::Cube);
    leftMirror->mesh = cubeMesh;
    leftMirror->position = glm::vec3(-4.88f, 2.1f, -1.0f);
    leftMirror->rotation = glm::vec3(0.0f, 90.0f, 0.0f);
    leftMirror->scale = glm::vec3(2.4f, 2.4f, 0.08f);
    applyMirrorPbr(leftMirror);
    rootNode->addChild(leftMirror);

    // Right Wall Mirror
    auto rightMirror = std::make_shared<SceneObject>("right_mirror", "Right Silver Mirror", PrimitiveType::Cube);
    rightMirror->mesh = cubeMesh;
    rightMirror->position = glm::vec3(4.88f, 2.1f, -1.0f);
    rightMirror->rotation = glm::vec3(0.0f, -90.0f, 0.0f);
    rightMirror->scale = glm::vec3(2.4f, 2.4f, 0.08f);
    applyMirrorPbr(rightMirror);
    rootNode->addChild(rightMirror);

    // ==========================================
    // 3. ANTIQUE VANITY DESK & SLIDING DRAWER
    // ==========================================
    auto deskRoot = std::make_shared<SceneObject>("vanity_desk", "Antique Vanity Desk", PrimitiveType::None);
    deskRoot->position = glm::vec3(-2.8f, 0.0f, -2.5f);
    rootNode->addChild(deskRoot);

    auto deskTop = std::make_shared<SceneObject>("desk_top", "Vanity Surface", PrimitiveType::Cube);
    deskTop->mesh = PrimitiveGenerator::createBeveledCube(2.2f, 0.08f, 1.2f, 0.015f, 2);
    deskTop->position = glm::vec3(0.0f, 0.8f, 0.0f);
    deskTop->scale = glm::vec3(1.0f, 1.0f, 1.0f);
    deskTop->material.diffuseColor = glm::vec3(0.38f, 0.18f, 0.10f);
    deskTop->material.useTexture = true;
    deskTop->material.textureId = woodPbr.albedo;
    deskRoot->addChild(deskTop);

    // 4 Desk Legs
    glm::vec3 legPos[4] = {
        {-0.95f, 0.4f, -0.45f},
        { 0.95f, 0.4f, -0.45f},
        {-0.95f, 0.4f,  0.45f},
        { 0.95f, 0.4f,  0.45f}
    };
    for (int i = 0; i < 4; ++i) {
        auto leg = std::make_shared<SceneObject>("vanity_leg_" + std::to_string(i+1), "Desk Leg", PrimitiveType::Cylinder);
        leg->mesh = cylinderMesh;
        leg->position = legPos[i];
        leg->scale = glm::vec3(0.06f, 0.8f, 0.06f);
        leg->material.diffuseColor = glm::vec3(0.3f, 0.15f, 0.08f);
        leg->material.useTexture = true;
        leg->material.textureId = woodPbr.albedo;
        deskRoot->addChild(leg);
    }

    // Desk Cabinet Housing
    auto drawerHousing = std::make_shared<SceneObject>("drawer_housing", "Drawer Housing", PrimitiveType::Cube);
    drawerHousing->mesh = cubeMesh;
    drawerHousing->position = glm::vec3(0.5f, 0.55f, 0.0f);
    drawerHousing->scale = glm::vec3(0.85f, 0.42f, 1.05f);
    drawerHousing->material.diffuseColor = glm::vec3(0.32f, 0.15f, 0.08f);
    drawerHousing->material.useTexture = true;
    drawerHousing->material.textureId = woodPbr.albedo;
    deskRoot->addChild(drawerHousing);

    // Sliding Drawer Container (Interactive)
    auto drawer = std::make_shared<SceneObject>("desk_drawer", "Vanity Drawer", PrimitiveType::None);
    drawer->position = glm::vec3(0.5f, 0.55f, 0.05f);
    drawer->basePosition = drawer->position;
    drawer->openPositionOffset = glm::vec3(0.0f, 0.0f, 0.85f);
    drawer->scale = glm::vec3(0.85f, 0.35f, 0.95f);
    drawer->interactType = InteractionType::DeskDrawer;
    deskRoot->addChild(drawer);
    interactionMgr.drawerObject = drawer;

    // Drawer Base
    auto drawerBottom = std::make_shared<SceneObject>("drawer_bottom", "Drawer Base", PrimitiveType::Cube);
    drawerBottom->mesh = cubeMesh;
    drawerBottom->position = glm::vec3(0.0f, -0.14f, 0.0f);
    drawerBottom->scale = glm::vec3(0.76f, 0.04f, 0.92f);
    drawerBottom->material.diffuseColor = glm::vec3(0.35f, 0.18f, 0.10f);
    drawerBottom->material.useTexture = true;
    drawerBottom->material.textureId = woodPbr.albedo;
    drawer->addChild(drawerBottom);

    // Drawer Front Panel
    auto drawerFront = std::make_shared<SceneObject>("drawer_front", "Drawer Front", PrimitiveType::Cube);
    drawerFront->mesh = cubeMesh;
    drawerFront->position = glm::vec3(0.0f, 0.0f, 0.44f);
    drawerFront->scale = glm::vec3(0.78f, 0.32f, 0.04f);
    drawerFront->material.diffuseColor = glm::vec3(0.42f, 0.22f, 0.12f);
    drawerFront->material.useTexture = true;
    drawerFront->material.textureId = woodPbr.albedo;
    drawerFront->interactType = InteractionType::DeskDrawer;
    drawer->addChild(drawerFront);

    // Brass Handle
    auto drawerHandle = std::make_shared<SceneObject>("drawer_handle", "Brass Handle", PrimitiveType::Cube);
    drawerHandle->mesh = cubeMesh;
    drawerHandle->position = glm::vec3(0.0f, 0.0f, 0.05f);
    drawerHandle->scale = glm::vec3(0.24f, 0.05f, 0.05f);
    drawerHandle->material.albedoColor = glm::vec3(0.9f, 0.8f, 0.3f);
    drawerHandle->material.metallic = 0.95f;
    drawerHandle->material.roughness = 0.20f;
    drawerHandle->material.diffuseColor = glm::vec3(0.9f, 0.8f, 0.3f);
    drawerFront->addChild(drawerHandle);

    // Cursed Mirror Inscription Clue Note inside Drawer
    auto clueNote = std::make_shared<SceneObject>("clue_note", "Mirror Curse Note", PrimitiveType::Cube);
    clueNote->mesh = cubeMesh;
    clueNote->position = glm::vec3(0.0f, -0.10f, 0.05f);
    clueNote->scale = glm::vec3(0.52f, 0.02f, 0.38f);
    clueNote->material.diffuseColor = glm::vec3(1.0f);
    clueNote->material.useTexture = true;
    clueNote->material.textureId = noteTex;
    clueNote->interactType = InteractionType::SecretNote;
    drawer->addChild(clueNote);

    // ==========================================
    // 4. HEAVY SILVER VAULT SAFE & ORNATE GOLDEN KEY (PUZZLE)
    // ==========================================
    auto safe = EscapeRoomBuilder::buildHeavyVaultSafe(interactionMgr, "mirror_safe",
        glm::vec3(2.8f, 0.0f, -2.5f), 0.0f, metalPbr.albedo, woodPbr.albedo,
        glm::vec3(0.32f, 0.30f, 0.38f), glm::vec3(0.95f, 0.95f, 1.0f));
    rootNode->addChild(safe);

    // ==========================================
    // 5. HIGH-VISIBILITY ILLUMINATED EXIT DOOR
    // ==========================================
    auto exitPortal = EscapeRoomBuilder::buildExitDoor(
        interactionMgr, "mirror_door",
        glm::vec3(0.0f, 0.0f, 4.88f), 0.0f,
        metalPbr.albedo, woodPbr.albedo,
        glm::vec3(0.38f, 0.35f, 0.42f),
        glm::vec3(0.34f, 0.20f, 0.14f), // Polished Victorian mahogany door
        glm::vec3(0.85f, 0.65f, 1.0f)   // Spectral violet glowing EXIT sign
    );
    rootNode->addChild(exitPortal);

    interactionMgr.correctSafePin = cluePin;
}
