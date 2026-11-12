#include "rooms/MirrorRoom.hpp"
#include "renderer/PrimitiveGenerator.hpp"
#include "renderer/Texture.hpp"
#include "renderer/MeshBatcher.hpp"
#include "scene/EscapeRoomBuilder.hpp"

MirrorRoom::MirrorRoom() {
    theme = RoomTheme::MirrorRoom;
    name = "Haunted Mirror Grand Gallery";
    description = "A long Victorian gallery with a towering spectral mirror at the far end. Ghostly anomalies dwell only inside the reflection.";
    jsonFileName = "mirror_room.json";
    cluePin = "8820";

    // New 8m x 16m spatial dimensions
    roomBounds = glm::vec4(-4.0f, 4.0f, -8.0f, 8.0f);
    spawnPosition = glm::vec3(0.0f, 1.7f, 6.5f);
    spawnYaw = -90.0f; // Facing North (-Z) down the grand gallery toward the Great Mirror
    spawnPitch = 0.0f;

    // Ghostly pale lavender / spectral blue chandelier flicker retuned for HDR
    pointLight.position = glm::vec3(0.0f, 3.8f, -2.0f);
    pointLight.color = glm::vec3(0.75f, 0.65f, 1.0f);
    pointLight.baseIntensity = 12.0f;
    pointLight.intensity = 12.0f;
    pointLight.constant = 1.0f;
    pointLight.linear = 0.10f;
    pointLight.quadratic = 0.04f;
    pointLight.flickerEnabled = true;
    pointLight.flickerSpeed = 5.0f;
    pointLight.flickerNoiseDepth = 0.30f;

    ambientLight.color = glm::vec3(0.12f, 0.08f, 0.20f);
    ambientLight.intensity = 0.25f;
    iblIntensity = 1.35f;
    ambientAO = 1.0f;
    fogColor = glm::vec3(0.08f, 0.06f, 0.14f);
    fogDensity = 0.040f;
    fogHeightFalloff = 0.32f;
    inscatterIntensity = 0.9f;

    fearSourcePosition = glm::vec3(0.0f, 2.0f, -7.0f);
    fearDistanceThreshold = 4.5f;
    baseVignette = 0.50f;
    baseChromatic = 0.005f;
}

std::string MirrorRoom::getObjectiveHint(const InteractionManager& im) const {
    if (im.gameWon) {
        return "Ác mộng của Dr. Vance đã bị phá vỡ! Chúc mừng bạn đã giải thoát toàn bộ linh hồn!";
    }
    if (im.mirrorPuzzleSolved) {
        return "Phong ấn phản chiếu đã giải trừ! Tiến về Cánh Cửa Thánh Địa Cuối Cùng để thoát khỏi ác mộng Vance.";
    }
    return "Quan sát hình ảnh phản chiếu trong Gương Quỷ. Xoay 3 bức tượng ngoài đời thực [E] cho khớp với gợi ý trong gương.";
}

void MirrorRoom::build(InteractionManager& interactionMgr) {
    auto cubeMesh = PrimitiveGenerator::createCube();
    auto cylinderMesh = PrimitiveGenerator::createCylinder(32);
    auto planeMesh = PrimitiveGenerator::createPlane();

    // Procedural PBR Texture Sets
    PbrTextureSet mirrorPbr = Texture::createMirrorPbr(512, 512);
    PbrTextureSet woodPbr = Texture::createWoodPbr(512, 512);
    PbrTextureSet metalPbr = Texture::createMetalPbr(512, 512);

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
    // 1. EXPANDED ROOM SHELL (8m x 4.8m x 16m)
    // ==========================================
    // Polished Dark Oak Floor
    auto floor = std::make_shared<SceneObject>("floor_mirror", "Polished Oak Parquet Floor", PrimitiveType::Plane);
    floor->mesh = planeMesh;
    floor->position = glm::vec3(0.0f, 0.0f, 0.0f);
    floor->scale = glm::vec3(8.0f, 1.0f, 16.0f);
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

    // Victorian Ceiling
    auto ceiling = std::make_shared<SceneObject>("ceiling_mirror", "Victorian Ornate Ceiling", PrimitiveType::Cube);
    ceiling->mesh = cubeMesh;
    ceiling->position = glm::vec3(0.0f, 4.8f, 0.0f);
    ceiling->scale = glm::vec3(8.0f, 0.2f, 16.0f);
    ceiling->material.albedoColor = glm::vec3(0.20f, 0.16f, 0.22f);
    rootNode->addChild(ceiling);

    // 2 Ghostly Chandeliers hanging down the long gallery
    for (int i = 0; i < 2; ++i) {
        float posZ = (i == 0) ? -3.0f : 3.0f;
        auto chandelier = std::make_shared<SceneObject>("chandelier_" + std::to_string(i+1), "Ghost Chandelier", PrimitiveType::Cylinder);
        chandelier->mesh = cylinderMesh;
        chandelier->position = glm::vec3(0.0f, 4.0f, posZ);
        chandelier->scale = glm::vec3(0.85f, 0.65f, 0.85f);
        chandelier->material.albedoColor = glm::vec3(0.85f, 0.75f, 1.0f);
        chandelier->material.metallic = 0.92f;
        chandelier->material.roughness = 0.18f;
        chandelier->material.emissiveColor = glm::vec3(0.85f, 0.70f, 1.0f);
        chandelier->material.emissiveIntensity = 2.5f;
        rootNode->addChild(chandelier);
    }

    // North Wall (-Z, behind the great mirror)
    auto backWall = std::make_shared<SceneObject>("back_wall_mirror", "Gallery North Wall", PrimitiveType::Cube);
    backWall->mesh = cubeMesh;
    backWall->position = glm::vec3(0.0f, 2.4f, -8.0f);
    backWall->scale = glm::vec3(8.0f, 4.8f, 0.2f);
    backWall->material.albedoColor = glm::vec3(0.25f, 0.18f, 0.26f);

    // South Wall (+Z, with Grand Sanctuary Exit Portal)
    auto frontWall = std::make_shared<SceneObject>("front_wall_mirror", "Gallery South Wall", PrimitiveType::Cube);
    frontWall->mesh = cubeMesh;
    frontWall->position = glm::vec3(0.0f, 2.4f, 8.0f);
    frontWall->scale = glm::vec3(8.0f, 4.8f, 0.2f);
    frontWall->material.albedoColor = glm::vec3(0.25f, 0.18f, 0.26f);

    // West Wall (-X)
    auto leftWall = std::make_shared<SceneObject>("left_wall_mirror", "Gallery West Wall", PrimitiveType::Cube);
    leftWall->mesh = cubeMesh;
    leftWall->position = glm::vec3(-4.0f, 2.4f, 0.0f);
    leftWall->scale = glm::vec3(0.2f, 4.8f, 16.0f);
    leftWall->material.albedoColor = glm::vec3(0.25f, 0.18f, 0.26f);

    // East Wall (+X)
    auto rightWall = std::make_shared<SceneObject>("right_wall_mirror", "Gallery East Wall", PrimitiveType::Cube);
    rightWall->mesh = cubeMesh;
    rightWall->position = glm::vec3(4.0f, 2.4f, 0.0f);
    rightWall->scale = glm::vec3(0.2f, 4.8f, 16.0f);
    rightWall->material.albedoColor = glm::vec3(0.25f, 0.18f, 0.26f);

    std::vector<std::shared_ptr<SceneObject>> staticWalls = { backWall, frontWall, leftWall, rightWall };
    auto batchedWalls = MeshBatcher::batchObjects("walls_mirror_batched", "Salon Victorian Walls", staticWalls, backWall->material);
    if (batchedWalls) {
        rootNode->addChild(batchedWalls);
    } else {
        for (auto& w : staticWalls) rootNode->addChild(w);
    }

    // ==========================================
    // 2. COLOSSAL HAUNTED GRAND MIRROR ON NORTH WALL
    // ==========================================
    auto centerMirror = std::make_shared<SceneObject>("center_mirror", "Haunted Grand Mirror", PrimitiveType::Cube);
    centerMirror->mesh = cubeMesh;
    centerMirror->position = glm::vec3(0.0f, 2.4f, -7.88f);
    centerMirror->scale = glm::vec3(6.4f, 3.8f, 0.08f);
    applyMirrorPbr(centerMirror);
    rootNode->addChild(centerMirror);

    // Ornate Golden Mirror Frame
    auto mirrorFrame = std::make_shared<SceneObject>("mirror_frame", "Ornate Gilded Frame", PrimitiveType::Cube);
    mirrorFrame->mesh = PrimitiveGenerator::createBeveledCube(6.7f, 4.1f, 0.10f, 0.04f, 2);
    mirrorFrame->position = glm::vec3(0.0f, 2.4f, -7.89f);
    mirrorFrame->material.albedoColor = glm::vec3(0.92f, 0.80f, 0.35f); // Burnished gold
    mirrorFrame->material.metallic = 0.95f;
    mirrorFrame->material.roughness = 0.25f;
    rootNode->addChild(mirrorFrame);

    // ==========================================
    // 3. 3 REAL-WORLD ROTATABLE BUSTS & PEDESTALS
    // ==========================================
    glm::vec3 pedestalPositions[3] = {
        {-1.8f, 0.0f, -3.5f},
        { 0.0f, 0.0f, -1.0f},
        { 1.8f, 0.0f, -3.5f}
    };

    for (int i = 0; i < 3; ++i) {
        auto pedestal = std::make_shared<SceneObject>("pedestal_" + std::to_string(i+1), "Victorian Marble Pedestal", PrimitiveType::Cylinder);
        pedestal->mesh = cylinderMesh;
        pedestal->position = pedestalPositions[i];
        pedestal->scale = glm::vec3(0.40f, 0.9f, 0.40f);
        pedestal->material.albedoColor = glm::vec3(0.85f, 0.85f, 0.88f); // White marble
        pedestal->material.roughness = 0.30f;
        rootNode->addChild(pedestal);

        // Real-world rotatable bust (visible in BOTH real and mirror)
        auto bust = std::make_shared<SceneObject>("bust_" + std::to_string(i+1), "Antique Sculptured Bust #" + std::to_string(i+1), PrimitiveType::Cube);
        bust->mesh = PrimitiveGenerator::createBeveledCube(0.38f, 0.65f, 0.32f, 0.03f, 2);
        bust->position = glm::vec3(0.0f, 0.78f, 0.0f);
        bust->scale = glm::vec3(1.0f);
        bust->rotation = glm::vec3(0.0f, interactionMgr.bustAngles[i], 0.0f);
        bust->material.albedoColor = glm::vec3(0.78f, 0.72f, 0.65f); // Antique ivory bronze
        bust->material.metallic = 0.40f;
        bust->material.roughness = 0.45f;
        bust->interactType = InteractionType::MirrorBust;
        bust->renderLayer = RenderLayer::Both;
        pedestal->addChild(bust);

        // Bust nose/pointer feature so orientation is visually unmistakable
        auto nosePointer = std::make_shared<SceneObject>("bust_face_" + std::to_string(i+1), "Bust Face Feature", PrimitiveType::Cube);
        nosePointer->mesh = cubeMesh;
        nosePointer->position = glm::vec3(0.0f, 0.12f, 0.20f);
        nosePointer->scale = glm::vec3(0.12f, 0.18f, 0.15f);
        nosePointer->material.albedoColor = glm::vec3(0.9f, 0.8f, 0.5f);
        bust->addChild(nosePointer);
    }

    // ==========================================
    // 4. MIRROR-ONLY GHOSTLY SPECTRAL GUIDES (RenderLayer::MirrorOnly)
    // ==========================================
    // These ghostly markers ONLY render inside the reflection FBO!
    // Target angles: Bust 1 = 180 deg (South), Bust 2 = 90 deg (East), Bust 3 = 270 deg (West)
    float ghostAngles[3] = { 180.0f, 90.0f, 270.0f };

    for (int i = 0; i < 3; ++i) {
        auto ghostGuide = std::make_shared<SceneObject>("mirror_ghost_guide_" + std::to_string(i+1), "Spectral Reflection Guide", PrimitiveType::Cube);
        ghostGuide->mesh = PrimitiveGenerator::createBeveledCube(0.20f, 0.50f, 0.45f, 0.02f, 2);
        ghostGuide->position = pedestalPositions[i] + glm::vec3(0.0f, 1.75f, 0.0f); // Hovering above pedestal in the mirror
        ghostGuide->rotation = glm::vec3(0.0f, ghostAngles[i], 0.0f);
        ghostGuide->scale = glm::vec3(1.0f);
        ghostGuide->material.albedoColor = glm::vec3(0.85f, 0.45f, 1.0f);
        ghostGuide->material.emissiveColor = glm::vec3(0.85f, 0.45f, 1.0f);
        ghostGuide->material.emissiveIntensity = 4.5f; // Radiant spectral violet beacon
        // KEY INNOVATION: ONLY visible inside the mirror reflection!
        ghostGuide->renderLayer = RenderLayer::MirrorOnly;
        rootNode->addChild(ghostGuide);
    }

    // ==========================================
    // 5. FINAL SANCTUARY EXIT DOOR (ROOM 3 EXIT)
    // ==========================================
    auto exitPortal = EscapeRoomBuilder::buildExitDoor(
        interactionMgr, "mirror_door",
        glm::vec3(0.0f, 0.0f, 7.88f), 180.0f,
        metalPbr.albedo, woodPbr.albedo,
        glm::vec3(0.38f, 0.35f, 0.42f),
        glm::vec3(0.34f, 0.20f, 0.14f), // Polished Victorian mahogany door
        glm::vec3(0.85f, 0.65f, 1.0f)   // Spectral violet glowing EXIT sign
    );
    rootNode->addChild(exitPortal);

    // Final Golden Key on pedestal near exit door
    auto keyStand = std::make_shared<SceneObject>("final_key_stand", "Sanctuary Pedestal", PrimitiveType::Cylinder);
    keyStand->mesh = cylinderMesh;
    keyStand->position = glm::vec3(2.5f, 0.5f, 6.0f);
    keyStand->scale = glm::vec3(0.35f, 1.0f, 0.35f);
    keyStand->material.albedoColor = glm::vec3(0.6f, 0.55f, 0.65f);
    rootNode->addChild(keyStand);

    auto goldenKey = EscapeRoomBuilder::buildOrnateGoldenKey(interactionMgr);
    goldenKey->position = glm::vec3(2.5f, 1.15f, 6.0f);
    goldenKey->scale = glm::vec3(0.24f);
    goldenKey->interactType = InteractionType::GoldenKey;
    rootNode->addChild(goldenKey);
    interactionMgr.keyObject = goldenKey;

    interactionMgr.correctSafePin = cluePin; // "8820"
}
