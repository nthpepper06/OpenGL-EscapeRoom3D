#include "rooms/AbandonedAsylumRoom.hpp"
#include "renderer/PrimitiveGenerator.hpp"
#include "renderer/Texture.hpp"
#include "renderer/MeshBatcher.hpp"
#include "scene/EscapeRoomBuilder.hpp"

AbandonedAsylumRoom::AbandonedAsylumRoom() {
    theme = RoomTheme::AbandonedAsylum;
    name = "Abandoned Psychiatric Ward & Isolation Cell";
    description = "An L-corridor partitioned by reinforced iron bars. High-voltage power is out, and secret patient codes are written in UV-reactive paint.";
    jsonFileName = "asylum_room.json";
    cluePin = "4108";

    // New 14m x 8m spatial dimensions
    roomBounds = glm::vec4(-7.0f, 7.0f, -4.0f, 4.0f);
    spawnPosition = glm::vec3(-5.5f, 1.7f, 0.0f);
    spawnYaw = 0.0f; // Facing East (+X) towards the iron bars gate
    spawnPitch = 0.0f;

    // Atmospheric horror lighting: Emergency red flashing strobe until breaker is restored
    pointLight.position = glm::vec3(-3.0f, 3.6f, 0.0f);
    pointLight.color = glm::vec3(1.0f, 0.12f, 0.06f); // Red emergency strobe
    pointLight.baseIntensity = 4.5f;
    pointLight.intensity = 4.5f;
    pointLight.constant = 1.0f;
    pointLight.linear = 0.12f;
    pointLight.quadratic = 0.06f;
    pointLight.flickerEnabled = true;
    pointLight.flickerSpeed = 12.0f; // Fast alarm pulse
    pointLight.flickerNoiseDepth = 0.60f;

    ambientLight.color = glm::vec3(0.06f, 0.02f, 0.02f);
    ambientLight.intensity = 0.12f;
    iblIntensity = 0.95f;
    ambientAO = 1.0f;
    fogColor = glm::vec3(0.08f, 0.03f, 0.03f);
    fogDensity = 0.055f;
    fogHeightFalloff = 0.38f;
    inscatterIntensity = 1.0f;

    fearSourcePosition = glm::vec3(0.0f, 1.5f, 0.0f);
    fearDistanceThreshold = 3.5f;
    baseVignette = 0.55f;
    baseChromatic = 0.005f;
}

std::string AbandonedAsylumRoom::getObjectiveHint(const InteractionManager& im) const {
    if (im.exitUnlocked) {
        return "Isolation exit door is open! Step through to advance to the next chamber.";
    }
    if (im.safeUnlocked) {
        return "Security Keypad unlocked (PIN 4108)! Open the exit door to proceed.";
    }
    if (im.powerRestored) {
        return "Power restored! Toggle UV Blacklight [Right-Click / V] to reveal 4 hidden cipher digits in the cell corners.";
    }
    if (im.fusesCollected < 2) {
        return "Power failure! Follow the faint UV trail to find 2 missing electrical fuses [Acquired: " + std::to_string(im.fusesCollected) + "/2].";
    }
    return "Both fuses acquired! Insert them into the Circuit Breaker box [E] to restore 220V power & open security gate.";
}

void AbandonedAsylumRoom::build(InteractionManager& interactionMgr) {
    auto cubeMesh = PrimitiveGenerator::createCube();
    auto cylinderMesh = PrimitiveGenerator::createCylinder(32);
    auto planeMesh = PrimitiveGenerator::createPlane();

    // Procedural PBR Texture Sets
    PbrTextureSet asylumPbr = Texture::createAsylumTilePbr(512, 512);
    PbrTextureSet metalPbr = Texture::createMetalPbr(512, 512);
    unsigned int uvCodeTex = Texture::createNoteTexture("WARD #4108", "CODE: 4108", 256, 256);

    auto applyAsylumTilePbr = [&](const std::shared_ptr<SceneObject>& obj, const glm::vec3& tint = glm::vec3(1.0f)) {
        obj->material.albedoColor = tint;
        obj->material.albedoMap = asylumPbr.albedo;
        obj->material.normalMap = asylumPbr.normal;
        obj->material.roughnessMap = asylumPbr.roughness;
        obj->material.aoMap = asylumPbr.ao;
        obj->material.useAlbedoMap = true;
        obj->material.useNormalMap = true;
        obj->material.useRoughnessMap = true;
        obj->material.useAoMap = true;
        obj->material.useTexture = true;
        obj->material.textureId = asylumPbr.albedo;
    };

    rootNode = std::make_shared<SceneObject>("room_asylum_root", "Asylum Room Root", PrimitiveType::None);

    // ==========================================
    // 1. ASYLUM ROOM SHELL (14m x 4.5m x 8m)
    // ==========================================
    // Floor (Dirty Grimy Linoleum Tile)
    auto floor = std::make_shared<SceneObject>("floor_asylum", "Asylum Floor", PrimitiveType::Plane);
    floor->mesh = planeMesh;
    floor->position = glm::vec3(0.0f, 0.0f, 0.0f);
    floor->scale = glm::vec3(14.0f, 1.0f, 8.0f);
    applyAsylumTilePbr(floor, glm::vec3(1.0f));
    rootNode->addChild(floor);

    // Ceiling
    auto ceiling = std::make_shared<SceneObject>("ceiling_asylum", "Asylum Ceiling", PrimitiveType::Cube);
    ceiling->mesh = cubeMesh;
    ceiling->position = glm::vec3(0.0f, 4.5f, 0.0f);
    ceiling->scale = glm::vec3(14.0f, 0.2f, 8.0f);
    applyAsylumTilePbr(ceiling, glm::vec3(0.35f, 0.40f, 0.38f));
    rootNode->addChild(ceiling);

    // Fluorescent Fixtures on Ceiling (2 fixtures along the hallway)
    for (int i = 0; i < 2; ++i) {
        float posX = (i == 0) ? -3.5f : 3.5f;
        auto lampFixture = std::make_shared<SceneObject>("lamp_fixture_" + std::to_string(i+1), "Fluorescent Fixture", PrimitiveType::Cube);
        lampFixture->mesh = cubeMesh;
        lampFixture->position = glm::vec3(posX, 4.35f, 0.0f);
        lampFixture->scale = glm::vec3(3.0f, 0.1f, 0.4f);
        lampFixture->material.albedoColor = glm::vec3(0.9f, 1.0f, 0.98f);
        lampFixture->material.emissiveColor = glm::vec3(0.85f, 1.0f, 0.95f);
        lampFixture->material.emissiveIntensity = 3.5f;
        rootNode->addChild(lampFixture);
    }

    // North Wall (-Z)
    auto backWall = std::make_shared<SceneObject>("back_wall_asylum", "Asylum North Wall", PrimitiveType::Cube);
    backWall->mesh = cubeMesh;
    backWall->position = glm::vec3(0.0f, 2.25f, -4.0f);
    backWall->scale = glm::vec3(14.0f, 4.5f, 0.2f);
    applyAsylumTilePbr(backWall, glm::vec3(0.85f));

    // South Wall (+Z)
    auto frontWall = std::make_shared<SceneObject>("front_wall_asylum", "Asylum South Wall", PrimitiveType::Cube);
    frontWall->mesh = cubeMesh;
    frontWall->position = glm::vec3(0.0f, 2.25f, 4.0f);
    frontWall->scale = glm::vec3(14.0f, 4.5f, 0.2f);
    applyAsylumTilePbr(frontWall, glm::vec3(0.85f));

    // West Wall (-X, Entrance wall)
    auto leftWall = std::make_shared<SceneObject>("left_wall_asylum", "Asylum West Wall", PrimitiveType::Cube);
    leftWall->mesh = cubeMesh;
    leftWall->position = glm::vec3(-7.0f, 2.25f, 0.0f);
    leftWall->scale = glm::vec3(0.2f, 4.5f, 8.0f);
    applyAsylumTilePbr(leftWall, glm::vec3(0.85f));

    // East Wall (+X, Isolation back wall)
    auto rightWall = std::make_shared<SceneObject>("right_wall_asylum", "Asylum East Wall", PrimitiveType::Cube);
    rightWall->mesh = cubeMesh;
    rightWall->position = glm::vec3(7.0f, 2.25f, 0.0f);
    rightWall->scale = glm::vec3(0.2f, 4.5f, 8.0f);
    applyAsylumTilePbr(rightWall, glm::vec3(0.85f));

    std::vector<std::shared_ptr<SceneObject>> staticWalls = { backWall, frontWall, leftWall, rightWall };
    auto batchedWalls = MeshBatcher::batchObjects("walls_asylum_batched", "Asylum Tile Walls", staticWalls, backWall->material);
    if (batchedWalls) {
        rootNode->addChild(batchedWalls);
    } else {
        for (auto& w : staticWalls) rootNode->addChild(w);
    }

    // ==========================================
    // 2. REINFORCED IRON BARS PARTITION & ELECTRONIC GATE
    // ==========================================
    auto partitionRoot = std::make_shared<SceneObject>("partition_root", "Iron Bars Partition Root", PrimitiveType::None);
    partitionRoot->position = glm::vec3(0.0f, 0.0f, 0.0f);
    rootNode->addChild(partitionRoot);

    // Left Partition Frame & Bars (Z in [-4.0, -1.0])
    auto leftBarsFrame = std::make_shared<SceneObject>("partition_left", "Left Iron Partition", PrimitiveType::Cube);
    leftBarsFrame->mesh = cubeMesh;
    leftBarsFrame->position = glm::vec3(0.0f, 2.25f, -2.5f);
    leftBarsFrame->scale = glm::vec3(0.18f, 4.5f, 3.0f);
    leftBarsFrame->material.albedoColor = glm::vec3(0.25f, 0.28f, 0.30f);
    leftBarsFrame->material.metallic = 0.90f;
    leftBarsFrame->material.roughness = 0.35f;
    partitionRoot->addChild(leftBarsFrame);

    // Right Partition Frame & Bars (Z in [1.0, 4.0])
    auto rightBarsFrame = std::make_shared<SceneObject>("partition_right", "Right Iron Partition", PrimitiveType::Cube);
    rightBarsFrame->mesh = cubeMesh;
    rightBarsFrame->position = glm::vec3(0.0f, 2.25f, 2.5f);
    rightBarsFrame->scale = glm::vec3(0.18f, 4.5f, 3.0f);
    rightBarsFrame->material.albedoColor = glm::vec3(0.25f, 0.28f, 0.30f);
    rightBarsFrame->material.metallic = 0.90f;
    rightBarsFrame->material.roughness = 0.35f;
    partitionRoot->addChild(rightBarsFrame);

    // Center Electronic Sliding Security Gate (Z in [-1.0, 1.0])
    auto ironGate = std::make_shared<SceneObject>("iron_bars_gate", "Electronic Security Gate", PrimitiveType::Cube);
    ironGate->mesh = PrimitiveGenerator::createBeveledCube(0.14f, 4.2f, 2.0f, 0.02f, 2);
    ironGate->position = glm::vec3(0.0f, 2.1f, 0.0f);
    ironGate->basePosition = ironGate->position;
    ironGate->openPositionOffset = glm::vec3(0.0f, 0.0f, 2.1f); // Slides open when power restored
    ironGate->interactType = InteractionType::IsolationGate;
    ironGate->material.albedoColor = glm::vec3(0.32f, 0.35f, 0.38f);
    ironGate->material.metallic = 0.92f;
    ironGate->material.roughness = 0.28f;
    partitionRoot->addChild(ironGate);
    interactionMgr.isolationGateObject = ironGate;

    // Gate Lock Indicator Light (Red = locked, Green = unlocked)
    auto lockLight = std::make_shared<SceneObject>("lock_indicator", "Gate Solenoid Lock", PrimitiveType::Cube);
    lockLight->mesh = cubeMesh;
    lockLight->position = glm::vec3(-0.12f, 1.8f, 0.95f);
    lockLight->scale = glm::vec3(0.08f, 0.12f, 0.08f);
    lockLight->material.albedoColor = glm::vec3(1.0f, 0.1f, 0.1f);
    lockLight->material.emissiveColor = glm::vec3(1.0f, 0.1f, 0.1f);
    lockLight->material.emissiveIntensity = 3.0f;
    ironGate->addChild(lockLight);

    // ==========================================
    // 3. CIRCUIT BREAKER & FUSE PUZZLE ITEMS
    // ==========================================
    // Breaker Box mounted on wall in the ante-room
    auto breakerBox = std::make_shared<SceneObject>("breaker_box", "Main High-Voltage Breaker Box", PrimitiveType::Cube);
    breakerBox->mesh = PrimitiveGenerator::createBeveledCube(0.25f, 0.85f, 0.65f, 0.02f, 2);
    breakerBox->position = glm::vec3(-6.85f, 1.8f, -2.5f);
    breakerBox->scale = glm::vec3(1.0f);
    breakerBox->material.albedoColor = glm::vec3(0.40f, 0.42f, 0.44f);
    breakerBox->material.metallic = 0.85f;
    breakerBox->material.roughness = 0.30f;
    breakerBox->interactType = InteractionType::BreakerBox;
    rootNode->addChild(breakerBox);

    // Switch Handle on Breaker Box
    auto breakerHandle = std::make_shared<SceneObject>("breaker_switch", "Main Breaker Lever", PrimitiveType::Cube);
    breakerHandle->mesh = cubeMesh;
    breakerHandle->position = glm::vec3(0.15f, 0.0f, 0.0f);
    breakerHandle->scale = glm::vec3(0.10f, 0.35f, 0.08f);
    breakerHandle->material.albedoColor = glm::vec3(0.9f, 0.2f, 0.1f);
    breakerHandle->material.roughness = 0.25f;
    breakerBox->addChild(breakerHandle);

    // Medical Trolley in ante-room (holding Fuse 1)
    auto trolley = std::make_shared<SceneObject>("medical_trolley", "Medical Equipment Trolley", PrimitiveType::Cube);
    trolley->mesh = PrimitiveGenerator::createBeveledCube(0.9f, 0.95f, 1.4f, 0.03f, 2);
    trolley->position = glm::vec3(-3.5f, 0.47f, -2.0f);
    trolley->material.albedoColor = glm::vec3(0.70f, 0.72f, 0.75f);
    trolley->material.metallic = 0.88f;
    trolley->material.roughness = 0.22f;
    rootNode->addChild(trolley);

    // Fuse 1 on top of Medical Trolley
    auto fuse1 = std::make_shared<SceneObject>("fuse_1", "High-Voltage Fuse 20A", PrimitiveType::Cylinder);
    fuse1->mesh = cylinderMesh;
    fuse1->position = glm::vec3(-3.5f, 1.05f, -2.0f);
    fuse1->scale = glm::vec3(0.08f, 0.22f, 0.08f);
    fuse1->rotation = glm::vec3(0.0f, 0.0f, 90.0f); // Lying flat
    fuse1->material.albedoColor = glm::vec3(0.95f, 0.85f, 0.25f); // Brass contact
    fuse1->material.metallic = 0.95f;
    fuse1->material.roughness = 0.15f;
    fuse1->material.emissiveColor = glm::vec3(0.95f, 0.85f, 0.25f);
    fuse1->material.emissiveIntensity = 1.2f; // Subtle glow so player spots it in the dark
    fuse1->interactType = InteractionType::FuseItem;
    rootNode->addChild(fuse1);

    // Washbasin / Sink in ante-room (holding Fuse 2)
    auto washbasin = std::make_shared<SceneObject>("washbasin", "Asylum Washbasin", PrimitiveType::Cube);
    washbasin->mesh = PrimitiveGenerator::createBeveledCube(0.75f, 0.9f, 0.9f, 0.03f, 2);
    washbasin->position = glm::vec3(-3.5f, 0.45f, 2.6f);
    washbasin->material.albedoColor = glm::vec3(0.85f, 0.88f, 0.86f);
    washbasin->material.roughness = 0.35f;
    rootNode->addChild(washbasin);

    // Fuse 2 inside Washbasin
    auto fuse2 = std::make_shared<SceneObject>("fuse_2", "Ceramic Main Fuse", PrimitiveType::Cylinder);
    fuse2->mesh = cylinderMesh;
    fuse2->position = glm::vec3(-3.5f, 0.98f, 2.6f);
    fuse2->scale = glm::vec3(0.08f, 0.22f, 0.08f);
    fuse2->rotation = glm::vec3(0.0f, 0.0f, 90.0f);
    fuse2->material.albedoColor = glm::vec3(0.95f, 0.85f, 0.25f);
    fuse2->material.metallic = 0.95f;
    fuse2->material.roughness = 0.15f;
    fuse2->material.emissiveColor = glm::vec3(0.95f, 0.85f, 0.25f);
    fuse2->material.emissiveIntensity = 1.2f;
    fuse2->interactType = InteractionType::FuseItem;
    rootNode->addChild(fuse2);

    // 3 UV Reactive Fluorescent Chevrons leading to Fuse 2 (washbasin)
    glm::vec3 arrowPositions[3] = {
        {-1.2f, 0.015f,  0.0f},  // Corridor floor near partition
        {-3.5f, 0.015f,  1.0f},  // Corridor turn floor
        {-4.88f, 1.60f,  2.6f}   // Wall directly above washbasin
    };
    glm::vec3 arrowRotations[3] = {
        {0.0f,  90.0f, 0.0f},
        {0.0f, 135.0f, 0.0f},
        {0.0f,   0.0f, -90.0f}
    };
    for (int i = 0; i < 3; ++i) {
        auto arrow = std::make_shared<SceneObject>("uv_arrow_fuse_" + std::to_string(i + 1), "Fluorescent Trail Marker #" + std::to_string(i + 1), PrimitiveType::Cube);
        arrow->mesh = cubeMesh;
        arrow->position = arrowPositions[i];
        arrow->rotation = arrowRotations[i];
        arrow->scale = (i == 2) ? glm::vec3(0.04f, 0.50f, 0.18f) : glm::vec3(0.40f, 0.01f, 0.16f);
        arrow->material.uIsUVReactive = true;
        arrow->material.albedoColor = glm::vec3(0.10f, 1.0f, 0.65f);
        arrow->material.emissiveColor = glm::vec3(0.10f, 1.0f, 0.65f);
        arrow->material.emissiveIntensity = 2.0f;
        rootNode->addChild(arrow);
    }

    // ==========================================
    // 4. INNER ISOLATION CELL & UV FLUORESCENT CIPHER WALL
    // ==========================================
    // Examination Gurney inside isolation cell
    auto gurneyRoot = std::make_shared<SceneObject>("gurney_root", "Isolation Gurney", PrimitiveType::Cube);
    gurneyRoot->mesh = PrimitiveGenerator::createBeveledCube(1.2f, 0.85f, 2.6f, 0.03f, 2);
    gurneyRoot->position = glm::vec3(3.5f, 0.42f, 1.2f);
    gurneyRoot->rotation = glm::vec3(0.0f, 15.0f, 0.0f);
    gurneyRoot->material.albedoColor = glm::vec3(0.40f, 0.45f, 0.42f);
    gurneyRoot->material.roughness = 0.50f;
    rootNode->addChild(gurneyRoot);

    // UV Fluorescent Wall Panel (Reveals "4108" under UV Blacklight!)
    auto uvPanel = std::make_shared<SceneObject>("uv_wall_runes", "UV Fluorescent Patient Record", PrimitiveType::Cube);
    uvPanel->mesh = cubeMesh;
    uvPanel->position = glm::vec3(3.5f, 2.2f, -3.88f); // On the isolation cell North Wall
    uvPanel->scale = glm::vec3(2.4f, 1.5f, 0.04f);
    uvPanel->material.albedoColor = glm::vec3(0.20f, 0.22f, 0.22f); // Barely noticeable in white light
    uvPanel->material.roughness = 0.80f;
    uvPanel->material.useTexture = true;
    uvPanel->material.textureId = uvCodeTex;
    // Mark as UV Fluorescent: reacts to UV Blacklight with radiant cyan emission!
    uvPanel->material.isFluorescent = true;
    uvPanel->material.uIsUVReactive = true;
    uvPanel->material.fluorescentColor = glm::vec3(0.25f, 0.95f, 1.0f); // Brilliant cyan-blue fluorescence
    rootNode->addChild(uvPanel);

    // 4 UV Reactive Cipher Plaques in 4 corners of Isolation Cell (Order 1->2->3->4 = PIN from cluePin)
    struct CipherPlaqueData {
        glm::vec3 pos;
        glm::vec3 rot;
        std::string label;
        char digit;
        int orderIndex;
    };
    CipherPlaqueData cipherData[4] = {
        {{ 4.5f, 2.0f, -3.88f}, {0.0f,   0.0f, 0.0f}, "Cipher Fragment #1", (cluePin.size() > 0 ? cluePin[0] : '4'), 1},
        {{ 6.88f, 2.0f, -1.8f}, {0.0f, -90.0f, 0.0f}, "Cipher Fragment #2", (cluePin.size() > 1 ? cluePin[1] : '1'), 2},
        {{ 4.5f, 2.0f,  3.88f}, {0.0f, 180.0f, 0.0f}, "Cipher Fragment #3", (cluePin.size() > 2 ? cluePin[2] : '0'), 3},
        {{ 1.68f, 2.0f,  2.0f}, {0.0f,  90.0f, 0.0f}, "Cipher Fragment #4", (cluePin.size() > 3 ? cluePin[3] : '8'), 4}
    };

    for (int i = 0; i < 4; ++i) {
        auto plaqueRoot = std::make_shared<SceneObject>("uv_cipher_" + std::to_string(i + 1), cipherData[i].label, PrimitiveType::None);
        plaqueRoot->position = cipherData[i].pos;
        plaqueRoot->rotation = cipherData[i].rot;
        rootNode->addChild(plaqueRoot);

        auto plate = std::make_shared<SceneObject>("uv_plate_" + std::to_string(i + 1), "Cipher Plate [" + std::to_string(cipherData[i].orderIndex) + "]", PrimitiveType::Cube);
        plate->mesh = PrimitiveGenerator::createBeveledCube(0.65f, 0.65f, 0.03f, 0.01f, 1);
        plate->position = glm::vec3(0.0f);
        plate->material.uIsUVReactive = true;
        plate->material.albedoColor = glm::vec3(0.12f, 0.14f, 0.14f);
        plate->interactType = InteractionType::SecretNote;
        plaqueRoot->addChild(plate);

        auto numTag = std::make_shared<SceneObject>("uv_tag_" + std::to_string(i + 1), "Digit " + std::string(1, cipherData[i].digit), PrimitiveType::Cube);
        numTag->mesh = cubeMesh;
        numTag->position = glm::vec3(0.0f, -0.04f, 0.025f);
        numTag->scale = glm::vec3(0.35f, 0.35f, 0.02f);
        numTag->material.uIsUVReactive = true;
        numTag->material.albedoColor = glm::vec3(0.10f, 1.0f, 0.65f);
        numTag->material.emissiveColor = glm::vec3(0.10f, 1.0f, 0.65f);
        numTag->material.emissiveIntensity = 4.0f;
        plate->addChild(numTag);

        // Small fluorescent order position mark [1], [2], [3], [4] directly above each digit
        auto orderMark = std::make_shared<SceneObject>("uv_order_" + std::to_string(i + 1), "Position #" + std::to_string(cipherData[i].orderIndex), PrimitiveType::Cube);
        orderMark->mesh = cubeMesh;
        orderMark->position = glm::vec3(0.0f, 0.22f, 0.025f);
        orderMark->scale = glm::vec3(0.20f, 0.06f, 0.02f);
        orderMark->material.uIsUVReactive = true;
        orderMark->material.albedoColor = glm::vec3(0.25f, 0.90f, 1.0f);
        orderMark->material.emissiveColor = glm::vec3(0.25f, 0.90f, 1.0f);
        orderMark->material.emissiveIntensity = 3.5f;
        plate->addChild(orderMark);
    }

    // Specimen Examination Table inside cell (Empty, no bypass key - player MUST solve UV puzzle!)
    auto specimenTable = std::make_shared<SceneObject>("specimen_table", "Specimen Examination Stand", PrimitiveType::Cylinder);
    specimenTable->mesh = cylinderMesh;
    specimenTable->position = glm::vec3(3.5f, 0.5f, -1.8f);
    specimenTable->scale = glm::vec3(0.4f, 1.0f, 0.4f);
    specimenTable->material.albedoColor = glm::vec3(0.35f, 0.38f, 0.36f);
    rootNode->addChild(specimenTable);

    // ==========================================
    // 5. ISOLATION EXIT DOOR & KEYPAD
    // ==========================================
    auto exitPortal = EscapeRoomBuilder::buildExitDoor(
        interactionMgr, "asylum_door",
        glm::vec3(6.88f, 0.0f, 0.0f), -90.0f,
        metalPbr.albedo, asylumPbr.albedo,
        glm::vec3(0.35f, 0.40f, 0.38f),
        glm::vec3(0.18f, 0.26f, 0.24f),
        glm::vec3(0.2f, 0.95f, 0.85f)
    );
    rootNode->addChild(exitPortal);

    interactionMgr.correctSafePin = cluePin; // "4108"
}
