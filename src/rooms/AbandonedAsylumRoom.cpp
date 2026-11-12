#include "rooms/AbandonedAsylumRoom.hpp"
#include "renderer/PrimitiveGenerator.hpp"
#include "renderer/Texture.hpp"
#include "scene/EscapeRoomBuilder.hpp"

AbandonedAsylumRoom::AbandonedAsylumRoom() {
    theme = RoomTheme::AbandonedAsylum;
    name = "Abandoned Psychiatric Ward";
    description = "A forsaken asylum hall with peeling tiles, rusty gurneys, and buzzing fluorescent tubes.";
    jsonFileName = "asylum_room.json";
    cluePin = "4108";

    spawnPosition = glm::vec3(0.0f, 1.7f, 2.8f);
    spawnYaw = -90.0f;
    spawnPitch = 0.0f;

    // Cold fluorescent flickering light (sickly greenish-cyan with abrupt dips retuned for HDR)
    pointLight.position = glm::vec3(0.0f, 3.2f, 0.0f);
    pointLight.color = glm::vec3(0.70f, 0.95f, 0.92f);
    pointLight.baseIntensity = 11.0f;
    pointLight.intensity = 11.0f;
    pointLight.constant = 1.0f;
    pointLight.linear = 0.14f;
    pointLight.quadratic = 0.07f;
    pointLight.flickerEnabled = true;
    pointLight.flickerSpeed = 9.0f;
    pointLight.flickerNoiseDepth = 0.50f;

    ambientLight.color = glm::vec3(0.08f, 0.15f, 0.14f);
    ambientLight.intensity = 0.20f;
    iblIntensity = 1.25f;
    ambientAO = 1.0f;
    fogColor = glm::vec3(0.06f, 0.12f, 0.10f);
    fogDensity = 0.060f;
    fogHeightFalloff = 0.38f;
    inscatterIntensity = 1.0f;

    fearSourcePosition = glm::vec3(-1.6f, 0.6f, -1.0f);
    fearDistanceThreshold = 3.0f;
    baseVignette = 0.55f;
    baseChromatic = 0.005f;
}

void AbandonedAsylumRoom::build(InteractionManager& interactionMgr) {
    auto cubeMesh = PrimitiveGenerator::createCube();
    auto cylinderMesh = PrimitiveGenerator::createCylinder(32);
    auto planeMesh = PrimitiveGenerator::createPlane();

    // Procedural PBR Texture Sets
    PbrTextureSet asylumPbr = Texture::createAsylumTilePbr(512, 512);
    PbrTextureSet metalPbr = Texture::createMetalPbr(512, 512);
    PbrTextureSet woodPbr = Texture::createWoodPbr(512, 512);
    unsigned int noteTex = Texture::createNoteTexture("PATIENT RECORD", "PIN: " + cluePin, 256, 256);

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
    // 1. ASYLUM ROOM SHELL (10m x 4.0m x 10m)
    // ==========================================
    // Floor (Dirty Grimy Linoleum Tile)
    auto floor = std::make_shared<SceneObject>("floor_asylum", "Asylum Floor", PrimitiveType::Plane);
    floor->mesh = planeMesh;
    floor->position = glm::vec3(0.0f, 0.0f, 0.0f);
    floor->scale = glm::vec3(10.0f, 1.0f, 10.0f);
    applyAsylumTilePbr(floor, glm::vec3(1.0f));
    rootNode->addChild(floor);

    // Ceiling with broken tiles
    auto ceiling = std::make_shared<SceneObject>("ceiling_asylum", "Asylum Ceiling", PrimitiveType::Cube);
    ceiling->mesh = cubeMesh;
    ceiling->position = glm::vec3(0.0f, 4.0f, 0.0f);
    ceiling->scale = glm::vec3(10.0f, 0.2f, 10.0f);
    applyAsylumTilePbr(ceiling, glm::vec3(0.35f, 0.40f, 0.38f));
    rootNode->addChild(ceiling);

    // Fluorescent Light Fixture on ceiling
    auto lampFixture = std::make_shared<SceneObject>("lamp_fixture", "Fluorescent Fixture", PrimitiveType::Cube);
    lampFixture->mesh = cubeMesh;
    lampFixture->position = glm::vec3(0.0f, 3.85f, 0.0f);
    lampFixture->scale = glm::vec3(0.4f, 0.1f, 2.5f);
    lampFixture->material.albedoColor = glm::vec3(0.9f, 1.0f, 0.98f);
    lampFixture->material.emissiveColor = glm::vec3(0.85f, 1.0f, 0.95f);
    lampFixture->material.emissiveIntensity = 3.5f; // Brilliant flickering fluorescent tube
    lampFixture->material.diffuseColor = glm::vec3(0.9f, 1.0f, 0.98f);
    rootNode->addChild(lampFixture);

    // Back Wall (-Z)
    auto backWall = std::make_shared<SceneObject>("back_wall_asylum", "Asylum Back Wall", PrimitiveType::Cube);
    backWall->mesh = cubeMesh;
    backWall->position = glm::vec3(0.0f, 2.0f, -5.0f);
    backWall->scale = glm::vec3(10.0f, 4.0f, 0.2f);
    applyAsylumTilePbr(backWall, glm::vec3(0.85f));
    rootNode->addChild(backWall);

    // Front Wall (+Z)
    auto frontWall = std::make_shared<SceneObject>("front_wall_asylum", "Asylum Front Wall", PrimitiveType::Cube);
    frontWall->mesh = cubeMesh;
    frontWall->position = glm::vec3(0.0f, 2.0f, 5.0f);
    frontWall->scale = glm::vec3(10.0f, 4.0f, 0.2f);
    applyAsylumTilePbr(frontWall, glm::vec3(0.85f));
    rootNode->addChild(frontWall);

    // Left Wall (-X)
    auto leftWall = std::make_shared<SceneObject>("left_wall_asylum", "Asylum Left Wall", PrimitiveType::Cube);
    leftWall->mesh = cubeMesh;
    leftWall->position = glm::vec3(-5.0f, 2.0f, 0.0f);
    leftWall->scale = glm::vec3(0.2f, 4.0f, 10.0f);
    applyAsylumTilePbr(leftWall, glm::vec3(0.85f));
    rootNode->addChild(leftWall);

    // Right Wall (+X)
    auto rightWall = std::make_shared<SceneObject>("right_wall_asylum", "Asylum Right Wall", PrimitiveType::Cube);
    rightWall->mesh = cubeMesh;
    rightWall->position = glm::vec3(5.0f, 2.0f, 0.0f);
    rightWall->scale = glm::vec3(0.2f, 4.0f, 10.0f);
    applyAsylumTilePbr(rightWall, glm::vec3(0.85f));
    rootNode->addChild(rightWall);

    // ==========================================
    // 2. MEDICAL EXAMINATION GURNEY (STRETCHER)
    // ==========================================
    auto gurneyRoot = std::make_shared<SceneObject>("gurney_root", "Medical Gurney Root", PrimitiveType::None);
    gurneyRoot->position = glm::vec3(-1.6f, 0.0f, -1.0f);
    gurneyRoot->rotation = glm::vec3(0.0f, 25.0f, 0.0f); // Tilted askew
    rootNode->addChild(gurneyRoot);

    // Gurney Bed Slab & Pad
    auto gurneyPad = std::make_shared<SceneObject>("gurney_pad", "Stretcher Pad", PrimitiveType::Cube);
    gurneyPad->mesh = cubeMesh;
    gurneyPad->position = glm::vec3(0.0f, 0.85f, 0.0f);
    gurneyPad->scale = glm::vec3(1.1f, 0.12f, 2.6f);
    gurneyPad->material.diffuseColor = glm::vec3(0.6f, 0.2f, 0.2f); // Stained vinyl
    gurneyRoot->addChild(gurneyPad);

    // Gurney Metal Legs (4 legs)
    glm::vec3 gurneyLegs[4] = {
        {-0.45f, 0.42f, -1.1f},
        { 0.45f, 0.42f, -1.1f},
        {-0.45f, 0.42f,  1.1f},
        { 0.45f, 0.42f,  1.1f}
    };
    for (int i = 0; i < 4; ++i) {
        auto leg = std::make_shared<SceneObject>("gurney_leg_" + std::to_string(i+1), "Gurney Leg", PrimitiveType::Cylinder);
        leg->mesh = cylinderMesh;
        leg->position = gurneyLegs[i];
        leg->scale = glm::vec3(0.05f, 0.85f, 0.05f);
        leg->material.diffuseColor = glm::vec3(0.3f, 0.3f, 0.32f);
        leg->material.useTexture = true;
        leg->material.textureId = metalPbr.albedo;
        gurneyRoot->addChild(leg);

        // Wheel caster
        auto wheel = std::make_shared<SceneObject>("gurney_wheel_" + std::to_string(i+1), "Wheel", PrimitiveType::Cylinder);
        wheel->mesh = cylinderMesh;
        wheel->position = glm::vec3(gurneyLegs[i].x, 0.06f, gurneyLegs[i].z);
        wheel->rotation = glm::vec3(0.0f, 0.0f, 90.0f);
        wheel->scale = glm::vec3(0.08f, 0.04f, 0.08f);
        wheel->material.diffuseColor = glm::vec3(0.15f, 0.15f, 0.15f);
        gurneyRoot->addChild(wheel);
    }

    // Restraint Straps
    auto strap = std::make_shared<SceneObject>("gurney_strap", "Restraint Strap", PrimitiveType::Cube);
    strap->mesh = cubeMesh;
    strap->position = glm::vec3(0.0f, 0.94f, 0.2f);
    strap->scale = glm::vec3(1.15f, 0.04f, 0.1f);
    strap->material.diffuseColor = glm::vec3(0.1f, 0.1f, 0.1f);
    gurneyRoot->addChild(strap);

    // ==========================================
    // 3. RUSTY FILE CABINET & SLIDING DRAWER
    // ==========================================
    auto cabinetRoot = std::make_shared<SceneObject>("cabinet_root", "Medical File Cabinet", PrimitiveType::Cube);
    cabinetRoot->mesh = cubeMesh;
    cabinetRoot->position = glm::vec3(3.2f, 0.85f, -2.5f);
    cabinetRoot->scale = glm::vec3(1.1f, 1.7f, 0.9f);
    cabinetRoot->material.diffuseColor = glm::vec3(0.25f, 0.32f, 0.28f); // Rusty olive
    cabinetRoot->material.useTexture = true;
    cabinetRoot->material.textureId = metalPbr.albedo;
    cabinetRoot->interactType = InteractionType::DeskDrawer;
    rootNode->addChild(cabinetRoot);

    // Sliding Drawer Container (Interactive)
    auto drawer = std::make_shared<SceneObject>("desk_drawer", "File Drawer", PrimitiveType::None);
    drawer->position = glm::vec3(0.0f, 0.1f, 0.05f);
    drawer->basePosition = drawer->position;
    drawer->openPositionOffset = glm::vec3(0.0f, 0.0f, 0.85f); // Slides forward
    drawer->scale = glm::vec3(0.9f, 0.4f, 0.85f);
    drawer->interactType = InteractionType::DeskDrawer;
    cabinetRoot->addChild(drawer);
    interactionMgr.drawerObject = drawer;

    // Drawer Base
    auto drawerBottom = std::make_shared<SceneObject>("drawer_bottom", "Drawer Base", PrimitiveType::Cube);
    drawerBottom->mesh = cubeMesh;
    drawerBottom->position = glm::vec3(0.0f, -0.15f, 0.0f);
    drawerBottom->scale = glm::vec3(0.80f, 0.04f, 0.80f);
    drawerBottom->material.diffuseColor = glm::vec3(0.22f, 0.28f, 0.25f);
    drawerBottom->material.useTexture = true;
    drawerBottom->material.textureId = metalPbr.albedo;
    drawer->addChild(drawerBottom);

    // Drawer Front Panel
    auto drawerFront = std::make_shared<SceneObject>("drawer_front", "Drawer Front", PrimitiveType::Cube);
    drawerFront->mesh = cubeMesh;
    drawerFront->position = glm::vec3(0.0f, 0.0f, 0.42f);
    drawerFront->scale = glm::vec3(0.85f, 0.35f, 0.04f);
    drawerFront->material.diffuseColor = glm::vec3(0.28f, 0.35f, 0.30f);
    drawerFront->material.useTexture = true;
    drawerFront->material.textureId = metalPbr.albedo;
    drawerFront->interactType = InteractionType::DeskDrawer;
    drawer->addChild(drawerFront);

    // Drawer Handle
    auto drawerHandle = std::make_shared<SceneObject>("drawer_handle", "Cabinet Handle", PrimitiveType::Cube);
    drawerHandle->mesh = cubeMesh;
    drawerHandle->position = glm::vec3(0.0f, 0.0f, 0.05f);
    drawerHandle->scale = glm::vec3(0.22f, 0.05f, 0.04f);
    drawerHandle->material.diffuseColor = glm::vec3(0.7f, 0.7f, 0.75f);
    drawerFront->addChild(drawerHandle);

    // Patient Report Clue Note inside Drawer
    auto clueNote = std::make_shared<SceneObject>("clue_note", "Patient Record Note", PrimitiveType::Cube);
    clueNote->mesh = cubeMesh;
    clueNote->position = glm::vec3(0.0f, -0.11f, 0.05f);
    clueNote->scale = glm::vec3(0.52f, 0.02f, 0.38f);
    clueNote->material.diffuseColor = glm::vec3(1.0f);
    clueNote->material.useTexture = true;
    clueNote->material.textureId = noteTex;
    clueNote->interactType = InteractionType::SecretNote;
    drawer->addChild(clueNote);

    // ==========================================
    // 4. HEAVY MEDICAL VAULT SAFE & ORNATE GOLDEN KEY (PUZZLE)
    // ==========================================
    auto safe = EscapeRoomBuilder::buildHeavyVaultSafe(interactionMgr, "asylum_safe",
        glm::vec3(3.2f, 0.0f, 1.2f), 0.0f, metalPbr.albedo, asylumPbr.albedo,
        glm::vec3(0.24f, 0.28f, 0.27f), glm::vec3(0.75f, 0.85f, 0.85f));
    rootNode->addChild(safe);

    // ==========================================
    // 5. HIGH-VISIBILITY ILLUMINATED EXIT DOOR
    // ==========================================
    auto exitPortal = EscapeRoomBuilder::buildExitDoor(
        interactionMgr, "asylum_door",
        glm::vec3(0.0f, 0.0f, 4.88f), 0.0f,
        metalPbr.albedo, asylumPbr.albedo,
        glm::vec3(0.32f, 0.36f, 0.35f),
        glm::vec3(0.38f, 0.48f, 0.46f), // Institutional teal/green door
        glm::vec3(0.10f, 1.0f, 0.35f)   // Neon radioactive green EXIT sign
    );
    rootNode->addChild(exitPortal);

    interactionMgr.correctSafePin = cluePin;
}
