#include "scene/EscapeRoomBuilder.hpp"
#include "renderer/PrimitiveGenerator.hpp"
#include "renderer/Texture.hpp"

std::shared_ptr<SceneObject> EscapeRoomBuilder::buildDefaultRoom(InteractionManager& interactionMgr) {
    auto root = std::make_shared<SceneObject>("room_root", "Escape Room", PrimitiveType::None);

    auto cubeMesh = PrimitiveGenerator::createCube();
    auto cylinderMesh = PrimitiveGenerator::createCylinder(32);
    auto planeMesh = PrimitiveGenerator::createPlane();

    // Textures
    unsigned int floorTex = Texture::createCheckerboard(256, 256, 32, {180, 170, 160}, {110, 105, 100});
    unsigned int woodTex = Texture::createWoodPattern(256, 256);
    unsigned int metalTex = Texture::createMetalPattern(256, 256);

    // ==========================================
    // 1. ROOM STRUCTURE (Floor, Ceiling, Walls)
    // ==========================================
    auto roomStructure = std::make_shared<SceneObject>("room_structure", "Architecture", PrimitiveType::None);
    root->addChild(roomStructure);

    // Floor (10x10)
    auto floor = std::make_shared<SceneObject>("obj_floor", "Floor", PrimitiveType::Cube);
    floor->mesh = cubeMesh;
    floor->position = glm::vec3(0.0f, -0.05f, 0.0f);
    floor->scale = glm::vec3(10.0f, 0.1f, 10.0f);
    floor->material.diffuseColor = glm::vec3(0.7f, 0.65f, 0.6f);
    floor->material.specularColor = glm::vec3(0.3f);
    floor->material.shininess = 32.0f;
    floor->material.useTexture = true;
    floor->material.textureId = floorTex;
    roomStructure->addChild(floor);

    // Ceiling
    auto ceiling = std::make_shared<SceneObject>("obj_ceiling", "Ceiling", PrimitiveType::Cube);
    ceiling->mesh = cubeMesh;
    ceiling->position = glm::vec3(0.0f, 4.05f, 0.0f);
    ceiling->scale = glm::vec3(10.0f, 0.1f, 10.0f);
    ceiling->material.diffuseColor = glm::vec3(0.85f, 0.85f, 0.88f);
    ceiling->material.specularColor = glm::vec3(0.1f);
    roomStructure->addChild(ceiling);

    // Back Wall (North: Z = -5)
    auto backWall = std::make_shared<SceneObject>("wall_north", "Wall North", PrimitiveType::Cube);
    backWall->mesh = cubeMesh;
    backWall->position = glm::vec3(0.0f, 2.0f, -5.05f);
    backWall->scale = glm::vec3(10.0f, 4.0f, 0.1f);
    backWall->material.diffuseColor = glm::vec3(0.75f, 0.72f, 0.68f);
    backWall->material.specularColor = glm::vec3(0.1f);
    roomStructure->addChild(backWall);

    // Left Wall (West: X = -5)
    auto leftWall = std::make_shared<SceneObject>("wall_west", "Wall West", PrimitiveType::Cube);
    leftWall->mesh = cubeMesh;
    leftWall->position = glm::vec3(-5.05f, 2.0f, 0.0f);
    leftWall->scale = glm::vec3(0.1f, 4.0f, 10.0f);
    leftWall->material.diffuseColor = glm::vec3(0.68f, 0.72f, 0.75f);
    leftWall->material.specularColor = glm::vec3(0.1f);
    roomStructure->addChild(leftWall);

    // Right Wall (East: X = 5)
    auto rightWall = std::make_shared<SceneObject>("wall_east", "Wall East", PrimitiveType::Cube);
    rightWall->mesh = cubeMesh;
    rightWall->position = glm::vec3(5.05f, 2.0f, 0.0f);
    rightWall->scale = glm::vec3(0.1f, 4.0f, 10.0f);
    rightWall->material.diffuseColor = glm::vec3(0.68f, 0.72f, 0.75f);
    rightWall->material.specularColor = glm::vec3(0.1f);
    roomStructure->addChild(rightWall);

    // Front Wall (South: Z = 5) - Left part, Right part, Header (leaving center for door)
    auto frontWallLeft = std::make_shared<SceneObject>("wall_south_left", "Wall South Left", PrimitiveType::Cube);
    frontWallLeft->mesh = cubeMesh;
    frontWallLeft->position = glm::vec3(-3.0f, 2.0f, 5.05f);
    frontWallLeft->scale = glm::vec3(4.0f, 4.0f, 0.1f);
    frontWallLeft->material.diffuseColor = glm::vec3(0.75f, 0.72f, 0.68f);
    roomStructure->addChild(frontWallLeft);

    auto frontWallRight = std::make_shared<SceneObject>("wall_south_right", "Wall South Right", PrimitiveType::Cube);
    frontWallRight->mesh = cubeMesh;
    frontWallRight->position = glm::vec3(3.0f, 2.0f, 5.05f);
    frontWallRight->scale = glm::vec3(4.0f, 4.0f, 0.1f);
    frontWallRight->material.diffuseColor = glm::vec3(0.75f, 0.72f, 0.68f);
    roomStructure->addChild(frontWallRight);

    auto frontWallTop = std::make_shared<SceneObject>("wall_south_top", "Wall South Top", PrimitiveType::Cube);
    frontWallTop->mesh = cubeMesh;
    frontWallTop->position = glm::vec3(0.0f, 3.25f, 5.05f);
    frontWallTop->scale = glm::vec3(2.0f, 1.5f, 0.1f);
    frontWallTop->material.diffuseColor = glm::vec3(0.75f, 0.72f, 0.68f);
    roomStructure->addChild(frontWallTop);

    // ==========================================
    // 2. CEILING LAMP & LIGHT SWITCH
    // ==========================================
    auto lamp = std::make_shared<SceneObject>("obj_ceiling_lamp", "Ceiling Lamp", PrimitiveType::None);
    lamp->position = glm::vec3(0.0f, 3.8f, 0.0f);
    root->addChild(lamp);

    auto lampCord = std::make_shared<SceneObject>("lamp_cord", "Lamp Cord", PrimitiveType::Cylinder);
    lampCord->mesh = cylinderMesh;
    lampCord->position = glm::vec3(0.0f, 0.15f, 0.0f);
    lampCord->scale = glm::vec3(0.04f, 0.4f, 0.04f);
    lampCord->material.diffuseColor = glm::vec3(0.1f);
    lamp->addChild(lampCord);

    auto lampShade = std::make_shared<SceneObject>("lamp_shade", "Lamp Shade", PrimitiveType::Cylinder);
    lampShade->mesh = cylinderMesh;
    lampShade->position = glm::vec3(0.0f, -0.05f, 0.0f);
    lampShade->scale = glm::vec3(0.6f, 0.2f, 0.6f);
    lampShade->material.diffuseColor = glm::vec3(0.2f, 0.2f, 0.22f);
    lamp->addChild(lampShade);

    auto lampBulb = std::make_shared<SceneObject>("lamp_bulb", "Light Bulb", PrimitiveType::Cube);
    lampBulb->mesh = cubeMesh;
    lampBulb->position = glm::vec3(0.0f, -0.15f, 0.0f);
    lampBulb->scale = glm::vec3(0.18f, 0.18f, 0.18f);
    lampBulb->material.diffuseColor = glm::vec3(1.0f, 0.95f, 0.6f);
    lampBulb->material.specularColor = glm::vec3(1.0f);
    lamp->addChild(lampBulb);

    // Wall Light Switch
    auto lightSwitch = std::make_shared<SceneObject>("light_switch", "Light Switch", PrimitiveType::Cube);
    lightSwitch->mesh = cubeMesh;
    lightSwitch->position = glm::vec3(-1.4f, 1.4f, 4.98f);
    lightSwitch->scale = glm::vec3(0.15f, 0.25f, 0.04f);
    lightSwitch->material.diffuseColor = glm::vec3(0.9f, 0.9f, 0.85f);
    lightSwitch->interactType = InteractionType::LightSwitch;
    root->addChild(lightSwitch);

    auto switchLever = std::make_shared<SceneObject>("switch_lever", "Switch Toggle", PrimitiveType::Cube);
    switchLever->mesh = cubeMesh;
    switchLever->position = glm::vec3(0.0f, 0.0f, 0.03f);
    switchLever->scale = glm::vec3(0.04f, 0.08f, 0.05f);
    switchLever->material.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
    lightSwitch->addChild(switchLever);

    // ==========================================
    // 3. WORK DESK WITH SLIDING DRAWER
    // ==========================================
    auto desk = std::make_shared<SceneObject>("obj_desk", "Work Desk", PrimitiveType::None);
    desk->position = glm::vec3(-2.2f, 0.0f, -3.8f);
    root->addChild(desk);

    // Desk Tabletop
    auto deskTop = std::make_shared<SceneObject>("desk_top", "Desk Surface", PrimitiveType::Cube);
    deskTop->mesh = cubeMesh;
    deskTop->position = glm::vec3(0.0f, 0.85f, 0.0f);
    deskTop->scale = glm::vec3(2.4f, 0.1f, 1.2f);
    deskTop->material.diffuseColor = glm::vec3(0.48f, 0.26f, 0.12f);
    deskTop->material.useTexture = true;
    deskTop->material.textureId = woodTex;
    desk->addChild(deskTop);

    // 4 Desk Legs
    glm::vec3 legPositions[4] = {
        {-1.1f, 0.4f, -0.5f},
        { 1.1f, 0.4f, -0.5f},
        {-1.1f, 0.4f,  0.5f},
        { 1.1f, 0.4f,  0.5f}
    };
    for (int i = 0; i < 4; ++i) {
        auto leg = std::make_shared<SceneObject>("desk_leg_" + std::to_string(i+1), "Desk Leg", PrimitiveType::Cylinder);
        leg->mesh = cylinderMesh;
        leg->position = legPositions[i];
        leg->scale = glm::vec3(0.08f, 0.8f, 0.08f);
        leg->material.diffuseColor = glm::vec3(0.2f, 0.2f, 0.22f);
        leg->material.useTexture = true;
        leg->material.textureId = metalTex;
        desk->addChild(leg);
    }

    // Drawer Housing (Cabinet on the right)
    auto drawerHousing = std::make_shared<SceneObject>("drawer_housing", "Drawer Housing", PrimitiveType::Cube);
    drawerHousing->mesh = cubeMesh;
    drawerHousing->position = glm::vec3(0.6f, 0.55f, 0.0f);
    drawerHousing->scale = glm::vec3(0.85f, 0.45f, 1.05f);
    drawerHousing->material.diffuseColor = glm::vec3(0.38f, 0.2f, 0.08f);
    drawerHousing->material.useTexture = true;
    drawerHousing->material.textureId = woodTex;
    desk->addChild(drawerHousing);

    // Sliding Drawer Container (Interactive, Open Tray Cavity)
    auto drawer = std::make_shared<SceneObject>("desk_drawer", "Desk Drawer", PrimitiveType::None);
    drawer->position = glm::vec3(0.6f, 0.55f, 0.05f);
    drawer->basePosition = drawer->position;
    drawer->openPositionOffset = glm::vec3(0.0f, 0.0f, 0.75f); // Slides forward along +Z
    drawer->scale = glm::vec3(0.85f, 0.35f, 0.95f);
    drawer->interactType = InteractionType::DeskDrawer;
    desk->addChild(drawer);
    interactionMgr.drawerObject = drawer;

    // Drawer Bottom Base
    auto drawerBottom = std::make_shared<SceneObject>("drawer_bottom", "Drawer Base", PrimitiveType::Cube);
    drawerBottom->mesh = cubeMesh;
    drawerBottom->position = glm::vec3(0.0f, -0.14f, 0.0f);
    drawerBottom->scale = glm::vec3(0.76f, 0.04f, 0.92f);
    drawerBottom->material.diffuseColor = glm::vec3(0.45f, 0.25f, 0.12f);
    drawerBottom->material.useTexture = true;
    drawerBottom->material.textureId = woodTex;
    drawer->addChild(drawerBottom);

    // Drawer Front Panel
    auto drawerFront = std::make_shared<SceneObject>("drawer_front", "Drawer Front", PrimitiveType::Cube);
    drawerFront->mesh = cubeMesh;
    drawerFront->position = glm::vec3(0.0f, 0.0f, 0.44f);
    drawerFront->scale = glm::vec3(0.78f, 0.32f, 0.04f);
    drawerFront->material.diffuseColor = glm::vec3(0.52f, 0.28f, 0.14f);
    drawerFront->material.useTexture = true;
    drawerFront->material.textureId = woodTex;
    drawer->addChild(drawerFront);

    // Drawer Back Panel
    auto drawerBack = std::make_shared<SceneObject>("drawer_back", "Drawer Back", PrimitiveType::Cube);
    drawerBack->mesh = cubeMesh;
    drawerBack->position = glm::vec3(0.0f, 0.0f, -0.44f);
    drawerBack->scale = glm::vec3(0.76f, 0.28f, 0.04f);
    drawerBack->material.diffuseColor = glm::vec3(0.42f, 0.22f, 0.10f);
    drawerBack->material.useTexture = true;
    drawerBack->material.textureId = woodTex;
    drawer->addChild(drawerBack);

    // Drawer Left Side
    auto drawerLeft = std::make_shared<SceneObject>("drawer_left", "Drawer Left Side", PrimitiveType::Cube);
    drawerLeft->mesh = cubeMesh;
    drawerLeft->position = glm::vec3(-0.36f, 0.0f, 0.0f);
    drawerLeft->scale = glm::vec3(0.04f, 0.28f, 0.92f);
    drawerLeft->material.diffuseColor = glm::vec3(0.42f, 0.22f, 0.10f);
    drawerLeft->material.useTexture = true;
    drawerLeft->material.textureId = woodTex;
    drawer->addChild(drawerLeft);

    // Drawer Right Side
    auto drawerRight = std::make_shared<SceneObject>("drawer_right", "Drawer Right Side", PrimitiveType::Cube);
    drawerRight->mesh = cubeMesh;
    drawerRight->position = glm::vec3(0.36f, 0.0f, 0.0f);
    drawerRight->scale = glm::vec3(0.04f, 0.28f, 0.92f);
    drawerRight->material.diffuseColor = glm::vec3(0.42f, 0.22f, 0.10f);
    drawerRight->material.useTexture = true;
    drawerRight->material.textureId = woodTex;
    drawer->addChild(drawerRight);

    // Drawer Handle
    auto drawerHandle = std::make_shared<SceneObject>("drawer_handle", "Drawer Handle", PrimitiveType::Cube);
    drawerHandle->mesh = cubeMesh;
    drawerHandle->position = glm::vec3(0.0f, 0.0f, 0.05f);
    drawerHandle->scale = glm::vec3(0.25f, 0.05f, 0.06f);
    drawerHandle->material.diffuseColor = glm::vec3(0.9f, 0.8f, 0.25f);
    drawerFront->addChild(drawerHandle);

    // Clue Note resting in open cavity
    unsigned int noteTex = Texture::createNoteTexture(256, 256);
    auto clueNote = std::make_shared<SceneObject>("clue_note", "Secret Clue Note", PrimitiveType::Cube);
    clueNote->mesh = cubeMesh;
    clueNote->position = glm::vec3(0.0f, -0.11f, 0.05f);
    clueNote->scale = glm::vec3(0.52f, 0.02f, 0.38f);
    clueNote->material.diffuseColor = glm::vec3(1.0f);
    clueNote->material.specularColor = glm::vec3(0.2f);
    clueNote->material.shininess = 16.0f;
    clueNote->material.useTexture = true;
    clueNote->material.textureId = noteTex;
    clueNote->interactType = InteractionType::SecretNote;
    drawer->addChild(clueNote);

    // ==========================================
    // 4. HEAVY STEEL SAFE (KÉT SẮT)
    // ==========================================
    auto safeRoot = std::make_shared<SceneObject>("obj_safe", "Electronic Safe", PrimitiveType::None);
    safeRoot->position = glm::vec3(2.5f, 0.0f, -3.8f);
    root->addChild(safeRoot);

    // Safe Pedestal Stand
    auto safeStand = std::make_shared<SceneObject>("safe_stand", "Safe Stand", PrimitiveType::Cube);
    safeStand->mesh = cubeMesh;
    safeStand->position = glm::vec3(0.0f, 0.45f, 0.0f);
    safeStand->scale = glm::vec3(1.3f, 0.9f, 1.3f);
    safeStand->material.diffuseColor = glm::vec3(0.2f, 0.2f, 0.22f);
    safeStand->material.useTexture = true;
    safeStand->material.textureId = metalTex;
    safeRoot->addChild(safeStand);

    // Safe Body (Hollow metal chassis)
    auto safeBody = std::make_shared<SceneObject>("safe_body", "Safe Chassis", PrimitiveType::None);
    safeBody->position = glm::vec3(0.0f, 1.45f, 0.0f);
    safeBody->scale = glm::vec3(1.1f, 1.1f, 1.1f);
    safeRoot->addChild(safeBody);

    // Safe Bottom
    auto safeBottom = std::make_shared<SceneObject>("safe_bottom", "Safe Floor", PrimitiveType::Cube);
    safeBottom->mesh = cubeMesh;
    safeBottom->position = glm::vec3(0.0f, -0.48f, 0.0f);
    safeBottom->scale = glm::vec3(1.0f, 0.08f, 1.0f);
    safeBottom->material.diffuseColor = glm::vec3(0.14f, 0.15f, 0.17f);
    safeBottom->material.useTexture = true;
    safeBottom->material.textureId = metalTex;
    safeBody->addChild(safeBottom);

    // Safe Top
    auto safeTop = std::make_shared<SceneObject>("safe_top", "Safe Ceiling", PrimitiveType::Cube);
    safeTop->mesh = cubeMesh;
    safeTop->position = glm::vec3(0.0f, 0.48f, 0.0f);
    safeTop->scale = glm::vec3(1.0f, 0.08f, 1.0f);
    safeTop->material.diffuseColor = glm::vec3(0.14f, 0.15f, 0.17f);
    safeTop->material.useTexture = true;
    safeTop->material.textureId = metalTex;
    safeBody->addChild(safeTop);

    // Safe Back
    auto safeBack = std::make_shared<SceneObject>("safe_back", "Safe Back Wall", PrimitiveType::Cube);
    safeBack->mesh = cubeMesh;
    safeBack->position = glm::vec3(0.0f, 0.0f, -0.48f);
    safeBack->scale = glm::vec3(1.0f, 0.96f, 0.08f);
    safeBack->material.diffuseColor = glm::vec3(0.10f, 0.11f, 0.13f);
    safeBack->material.useTexture = true;
    safeBack->material.textureId = metalTex;
    safeBody->addChild(safeBack);

    // Safe Left Wall
    auto safeLeft = std::make_shared<SceneObject>("safe_left", "Safe Left Wall", PrimitiveType::Cube);
    safeLeft->mesh = cubeMesh;
    safeLeft->position = glm::vec3(-0.48f, 0.0f, 0.0f);
    safeLeft->scale = glm::vec3(0.08f, 0.96f, 1.0f);
    safeLeft->material.diffuseColor = glm::vec3(0.12f, 0.13f, 0.15f);
    safeLeft->material.useTexture = true;
    safeLeft->material.textureId = metalTex;
    safeBody->addChild(safeLeft);

    // Safe Right Wall
    auto safeRight = std::make_shared<SceneObject>("safe_right", "Safe Right Wall", PrimitiveType::Cube);
    safeRight->mesh = cubeMesh;
    safeRight->position = glm::vec3(0.48f, 0.0f, 0.0f);
    safeRight->scale = glm::vec3(0.08f, 0.96f, 1.0f);
    safeRight->material.diffuseColor = glm::vec3(0.12f, 0.13f, 0.15f);
    safeRight->material.useTexture = true;
    safeRight->material.textureId = metalTex;
    safeBody->addChild(safeRight);

    // Safe Interior Shelf
    auto safeShelf = std::make_shared<SceneObject>("safe_shelf", "Safe Interior Shelf", PrimitiveType::Cube);
    safeShelf->mesh = cubeMesh;
    safeShelf->position = glm::vec3(0.0f, -0.1f, 0.0f);
    safeShelf->scale = glm::vec3(0.88f, 0.04f, 0.88f);
    safeShelf->material.diffuseColor = glm::vec3(0.2f, 0.22f, 0.25f);
    safeShelf->material.useTexture = true;
    safeShelf->material.textureId = metalTex;
    safeBody->addChild(safeShelf);

    // Safe Door Hinge Node (Pivot at front-left edge of safe)
    auto safeHinge = std::make_shared<SceneObject>("safe_hinge", "Safe Door Hinge", PrimitiveType::None);
    safeHinge->position = glm::vec3(-0.48f, 1.45f, 0.56f);
    safeHinge->basePosition = safeHinge->position;
    safeHinge->openRotationOffset = glm::vec3(0.0f, 110.0f, 0.0f); // Swings open 110 degrees!
    safeHinge->interactType = InteractionType::SafeDoor;
    safeHinge->isLocked = true;
    safeRoot->addChild(safeHinge);
    interactionMgr.safeDoorObject = safeHinge;

    // Safe Door Panel (Child of Hinge Node, offset to the right so hinge is at left edge)
    auto safeDoor = std::make_shared<SceneObject>("safe_door_panel", "Safe Door", PrimitiveType::Cube);
    safeDoor->mesh = cubeMesh;
    safeDoor->position = glm::vec3(0.48f, 0.0f, 0.0f);
    safeDoor->scale = glm::vec3(0.96f, 0.96f, 0.08f);
    safeDoor->material.diffuseColor = glm::vec3(0.16f, 0.18f, 0.2f);
    safeDoor->material.specularColor = glm::vec3(0.6f);
    safeDoor->material.shininess = 64.0f;
    safeDoor->material.useTexture = true;
    safeDoor->material.textureId = metalTex;
    safeHinge->addChild(safeDoor);

    // Digital Keypad on Safe Door
    auto keypad = std::make_shared<SceneObject>("safe_keypad", "Electronic Keypad", PrimitiveType::Cube);
    keypad->mesh = cubeMesh;
    keypad->position = glm::vec3(0.1f, 0.12f, 0.06f);
    keypad->scale = glm::vec3(0.24f, 0.34f, 0.04f);
    keypad->material.diffuseColor = glm::vec3(0.04f, 0.04f, 0.04f);
    keypad->interactType = InteractionType::SafeKeypad;
    safeDoor->addChild(keypad);

    // Safe Turning Wheel / Dial
    auto safeDial = std::make_shared<SceneObject>("safe_dial", "Safe Combination Wheel", PrimitiveType::Cylinder);
    safeDial->mesh = cylinderMesh;
    safeDial->position = glm::vec3(0.1f, -0.16f, 0.06f);
    safeDial->rotation = glm::vec3(90.0f, 0.0f, 0.0f);
    safeDial->scale = glm::vec3(0.22f, 0.05f, 0.22f);
    safeDial->material.diffuseColor = glm::vec3(0.85f, 0.85f, 0.9f);
    safeDial->material.specularColor = glm::vec3(0.9f);
    safeDial->material.shininess = 128.0f;
    safeDoor->addChild(safeDial);

    // Golden Key inside the Safe
    auto keyRoot = std::make_shared<SceneObject>("golden_key", "Golden Exit Key", PrimitiveType::None);
    keyRoot->position = glm::vec3(2.5f, 1.25f, -3.75f);
    keyRoot->interactType = InteractionType::GoldenKey;
    root->addChild(keyRoot);
    interactionMgr.keyObject = keyRoot;

    // Key Shaft
    auto keyShaft = std::make_shared<SceneObject>("key_shaft", "Key Shaft", PrimitiveType::Cylinder);
    keyShaft->mesh = cylinderMesh;
    keyShaft->position = glm::vec3(0.0f, 0.0f, 0.0f);
    keyShaft->rotation = glm::vec3(0.0f, 0.0f, 90.0f);
    keyShaft->scale = glm::vec3(0.03f, 0.28f, 0.03f);
    keyShaft->material.diffuseColor = glm::vec3(1.0f, 0.84f, 0.0f);
    keyShaft->material.specularColor = glm::vec3(1.0f, 0.95f, 0.5f);
    keyShaft->material.shininess = 128.0f;
    keyRoot->addChild(keyShaft);

    // Key Ring
    auto keyRing = std::make_shared<SceneObject>("key_ring", "Key Bow Ring", PrimitiveType::Cylinder);
    keyRing->mesh = cylinderMesh;
    keyRing->position = glm::vec3(-0.16f, 0.0f, 0.0f);
    keyRing->rotation = glm::vec3(90.0f, 0.0f, 0.0f);
    keyRing->scale = glm::vec3(0.1f, 0.02f, 0.1f);
    keyRing->material.diffuseColor = glm::vec3(1.0f, 0.84f, 0.0f);
    keyRing->material.specularColor = glm::vec3(1.0f, 0.95f, 0.5f);
    keyRing->material.shininess = 128.0f;
    keyRoot->addChild(keyRing);

    // Key Bit Teeth
    auto keyTeeth = std::make_shared<SceneObject>("key_teeth", "Key Teeth", PrimitiveType::Cube);
    keyTeeth->mesh = cubeMesh;
    keyTeeth->position = glm::vec3(0.12f, -0.04f, 0.0f);
    keyTeeth->scale = glm::vec3(0.06f, 0.06f, 0.02f);
    keyTeeth->material.diffuseColor = glm::vec3(1.0f, 0.84f, 0.0f);
    keyTeeth->material.specularColor = glm::vec3(1.0f, 0.95f, 0.5f);
    keyTeeth->material.shininess = 128.0f;
    keyRoot->addChild(keyTeeth);

    // ==========================================
    // 5. EXIT DOOR (CỬA CHÍNH THOÁT HIỂM)
    // ==========================================
    auto doorRoot = std::make_shared<SceneObject>("obj_exit_door", "Exit Door Assembly", PrimitiveType::None);
    doorRoot->position = glm::vec3(0.0f, 0.0f, 4.95f);
    root->addChild(doorRoot);

    // Door Frame Left Post
    auto frameLeft = std::make_shared<SceneObject>("door_frame_left", "Door Frame Left", PrimitiveType::Cube);
    frameLeft->mesh = cubeMesh;
    frameLeft->position = glm::vec3(-0.75f, 1.25f, 0.0f);
    frameLeft->scale = glm::vec3(0.12f, 2.5f, 0.18f);
    frameLeft->material.diffuseColor = glm::vec3(0.25f, 0.15f, 0.08f);
    doorRoot->addChild(frameLeft);

    // Door Frame Right Post
    auto frameRight = std::make_shared<SceneObject>("door_frame_right", "Door Frame Right", PrimitiveType::Cube);
    frameRight->mesh = cubeMesh;
    frameRight->position = glm::vec3(0.75f, 1.25f, 0.0f);
    frameRight->scale = glm::vec3(0.12f, 2.5f, 0.18f);
    frameRight->material.diffuseColor = glm::vec3(0.25f, 0.15f, 0.08f);
    doorRoot->addChild(frameRight);

    // Door Frame Top Header
    auto frameTop = std::make_shared<SceneObject>("door_frame_top", "Door Frame Top", PrimitiveType::Cube);
    frameTop->mesh = cubeMesh;
    frameTop->position = glm::vec3(0.0f, 2.52f, 0.0f);
    frameTop->scale = glm::vec3(1.62f, 0.12f, 0.18f);
    frameTop->material.diffuseColor = glm::vec3(0.25f, 0.15f, 0.08f);
    doorRoot->addChild(frameTop);

    // Illuminated EXIT Sign above door
    auto exitSign = std::make_shared<SceneObject>("exit_sign", "EXIT Sign", PrimitiveType::Cube);
    exitSign->mesh = cubeMesh;
    exitSign->position = glm::vec3(0.0f, 2.75f, -0.04f);
    exitSign->scale = glm::vec3(0.65f, 0.22f, 0.05f);
    exitSign->material.diffuseColor = glm::vec3(0.1f, 0.95f, 0.2f); // Glowing emergency green
    exitSign->material.specularColor = glm::vec3(0.8f);
    doorRoot->addChild(exitSign);

    // Door Hinge Pivot Node (at left post x = -0.68)
    auto doorHinge = std::make_shared<SceneObject>("door_hinge", "Exit Door Hinge", PrimitiveType::None);
    doorHinge->position = glm::vec3(-0.68f, 0.0f, 0.0f);
    doorHinge->basePosition = doorHinge->position;
    doorHinge->openRotationOffset = glm::vec3(0.0f, -95.0f, 0.0f); // Swings open
    doorHinge->interactType = InteractionType::ExitDoor;
    doorHinge->isLocked = true;
    doorRoot->addChild(doorHinge);
    interactionMgr.exitDoorObject = doorHinge;

    // Door Leaf Panel (Child of Hinge Node)
    auto doorLeaf = std::make_shared<SceneObject>("door_leaf", "Door Leaf", PrimitiveType::Cube);
    doorLeaf->mesh = cubeMesh;
    doorLeaf->position = glm::vec3(0.68f, 1.25f, 0.0f);
    doorLeaf->scale = glm::vec3(1.36f, 2.45f, 0.1f);
    doorLeaf->material.diffuseColor = glm::vec3(0.42f, 0.22f, 0.1f); // Stained oak
    doorLeaf->material.specularColor = glm::vec3(0.3f);
    doorLeaf->material.useTexture = true;
    doorLeaf->material.textureId = woodTex;
    doorHinge->addChild(doorLeaf);

    // Door Handle / Lock
    auto doorHandle = std::make_shared<SceneObject>("door_handle", "Door Knob Lock", PrimitiveType::Cylinder);
    doorHandle->mesh = cylinderMesh;
    doorHandle->position = glm::vec3(0.55f, 0.0f, -0.08f);
    doorHandle->rotation = glm::vec3(90.0f, 0.0f, 0.0f);
    doorHandle->scale = glm::vec3(0.08f, 0.1f, 0.08f);
    doorHandle->material.diffuseColor = glm::vec3(0.9f, 0.8f, 0.2f); // Brass knob
    doorHandle->material.specularColor = glm::vec3(0.9f);
    doorHandle->material.shininess = 64.0f;
    doorLeaf->addChild(doorHandle);

    return root;
}

std::shared_ptr<SceneObject> EscapeRoomBuilder::buildOrnateGoldenKey(InteractionManager& interactionMgr) {
    auto cubeMesh = PrimitiveGenerator::createCube();
    auto cylinderMesh = PrimitiveGenerator::createCylinder(32);

    // Root key object (invisible interaction bounding volume, clean 3D mesh only on children)
    auto keyRoot = std::make_shared<SceneObject>("golden_key", "Golden Key of Escape", PrimitiveType::None);
    keyRoot->position = glm::vec3(0.0f, 0.01f, 0.0f);
    keyRoot->scale = glm::vec3(0.50f, 0.20f, 0.35f);
    keyRoot->interactType = InteractionType::GoldenKey;
    interactionMgr.keyObject = keyRoot;

    // Key Bow (Circular Ornate Handle at the back)
    auto keyBow = std::make_shared<SceneObject>("key_bow", "Key Handle Ring", PrimitiveType::Cylinder);
    keyBow->mesh = cylinderMesh;
    keyBow->position = glm::vec3(-0.16f, 0.0f, 0.0f);
    keyBow->rotation = glm::vec3(90.0f, 0.0f, 0.0f);
    keyBow->scale = glm::vec3(0.18f, 0.04f, 0.18f);
    keyBow->material.albedoColor = glm::vec3(1.0f, 0.78f, 0.22f); // Radiant PBR Gold
    keyBow->material.metallic = 1.0f;
    keyBow->material.roughness = 0.16f;
    keyBow->material.diffuseColor = glm::vec3(1.0f, 0.85f, 0.2f);
    keyBow->material.specularColor = glm::vec3(1.0f, 0.95f, 0.7f);
    keyBow->material.shininess = 128.0f;
    keyRoot->addChild(keyBow);

    // Inner cutout ring of key handle
    auto keyHole = std::make_shared<SceneObject>("key_hole", "Key Ring Hole", PrimitiveType::Cylinder);
    keyHole->mesh = cylinderMesh;
    keyHole->position = glm::vec3(-0.16f, 0.005f, 0.0f);
    keyHole->rotation = glm::vec3(90.0f, 0.0f, 0.0f);
    keyHole->scale = glm::vec3(0.09f, 0.05f, 0.09f);
    keyHole->material.albedoColor = glm::vec3(0.35f, 0.15f, 0.18f);
    keyHole->material.metallic = 0.0f;
    keyHole->material.roughness = 0.9f;
    keyHole->material.diffuseColor = glm::vec3(0.35f, 0.15f, 0.18f); // Matches velvet pillow
    keyRoot->addChild(keyHole);

    // Key Stem (Cylindrical Shaft)
    auto keyStem = std::make_shared<SceneObject>("key_stem", "Key Shaft", PrimitiveType::Cylinder);
    keyStem->mesh = cylinderMesh;
    keyStem->position = glm::vec3(0.04f, 0.0f, 0.0f);
    keyStem->rotation = glm::vec3(0.0f, 0.0f, 90.0f);
    keyStem->scale = glm::vec3(0.04f, 0.28f, 0.04f);
    keyStem->material.albedoColor = glm::vec3(1.0f, 0.78f, 0.22f);
    keyStem->material.metallic = 1.0f;
    keyStem->material.roughness = 0.16f;
    keyStem->material.diffuseColor = glm::vec3(1.0f, 0.85f, 0.2f);
    keyStem->material.specularColor = glm::vec3(1.0f, 0.95f, 0.7f);
    keyStem->material.shininess = 128.0f;
    keyRoot->addChild(keyStem);

    // Key Teeth (Bit)
    auto keyBit = std::make_shared<SceneObject>("key_bit", "Key Bit Teeth", PrimitiveType::Cube);
    keyBit->mesh = cubeMesh;
    keyBit->position = glm::vec3(0.14f, 0.0f, 0.06f);
    keyBit->scale = glm::vec3(0.08f, 0.03f, 0.08f);
    keyBit->material.albedoColor = glm::vec3(1.0f, 0.78f, 0.22f);
    keyBit->material.metallic = 1.0f;
    keyBit->material.roughness = 0.16f;
    keyBit->material.diffuseColor = glm::vec3(1.0f, 0.85f, 0.2f);
    keyBit->material.specularColor = glm::vec3(1.0f, 0.95f, 0.7f);
    keyBit->material.shininess = 128.0f;
    keyRoot->addChild(keyBit);

    auto keyBit2 = std::make_shared<SceneObject>("key_bit2", "Key Notch", PrimitiveType::Cube);
    keyBit2->mesh = cubeMesh;
    keyBit2->position = glm::vec3(0.08f, 0.0f, 0.05f);
    keyBit2->scale = glm::vec3(0.04f, 0.03f, 0.06f);
    keyBit2->material.albedoColor = glm::vec3(1.0f, 0.78f, 0.22f);
    keyBit2->material.metallic = 1.0f;
    keyBit2->material.roughness = 0.16f;
    keyBit2->material.diffuseColor = glm::vec3(1.0f, 0.85f, 0.2f);
    keyBit2->material.specularColor = glm::vec3(1.0f, 0.95f, 0.7f);
    keyBit2->material.shininess = 128.0f;
    keyRoot->addChild(keyBit2);

    return keyRoot;
}

std::shared_ptr<SceneObject> EscapeRoomBuilder::buildHeavyVaultSafe(InteractionManager& interactionMgr,
                                                                  const std::string& prefix,
                                                                  const glm::vec3& position,
                                                                  float rotationY,
                                                                  unsigned int metalTex,
                                                                  unsigned int stoneTex,
                                                                  const glm::vec3& primaryColor,
                                                                  const glm::vec3& accentColor) {
    auto cubeMesh = PrimitiveGenerator::createCube();
    auto cylinderMesh = PrimitiveGenerator::createCylinder(32);

    auto safeRoot = std::make_shared<SceneObject>(prefix + "_root", "Industrial Vault Safe", PrimitiveType::None);
    safeRoot->position = position;
    safeRoot->rotation = glm::vec3(0.0f, rotationY, 0.0f);

    // 1. Heavy Stone / Industrial Pedestal
    auto stand = std::make_shared<SceneObject>(prefix + "_stand", "Heavy Base Stand", PrimitiveType::Cube);
    stand->mesh = cubeMesh;
    stand->position = glm::vec3(0.0f, 0.42f, 0.0f);
    stand->scale = glm::vec3(1.4f, 0.84f, 1.4f);
    stand->material.diffuseColor = glm::vec3(0.22f, 0.22f, 0.24f);
    stand->material.useTexture = true;
    stand->material.textureId = stoneTex ? stoneTex : metalTex;
    safeRoot->addChild(stand);

    // Pedestal Base Trim
    auto standTrim = std::make_shared<SceneObject>(prefix + "_stand_trim", "Pedestal Trim", PrimitiveType::Cube);
    standTrim->mesh = cubeMesh;
    standTrim->position = glm::vec3(0.0f, 0.06f, 0.0f);
    standTrim->scale = glm::vec3(1.48f, 0.12f, 1.48f);
    standTrim->material.diffuseColor = glm::vec3(0.18f, 0.18f, 0.20f);
    standTrim->material.useTexture = true;
    standTrim->material.textureId = metalTex;
    safeRoot->addChild(standTrim);

    // 2. Heavy Armored Safe Chassis
    auto safeBody = std::make_shared<SceneObject>(prefix + "_body", "Safe Chassis", PrimitiveType::None);
    safeBody->position = glm::vec3(0.0f, 1.46f, 0.0f);
    safeRoot->addChild(safeBody);

    // Heavy Outer Walls (thick 12cm steel plates)
    auto safeBottom = std::make_shared<SceneObject>(prefix + "_bottom", "Safe Bottom Plate", PrimitiveType::Cube);
    safeBottom->mesh = cubeMesh;
    safeBottom->position = glm::vec3(0.0f, -0.48f, 0.0f);
    safeBottom->scale = glm::vec3(1.24f, 0.12f, 1.24f);
    safeBottom->material.diffuseColor = primaryColor * 0.9f;
    safeBottom->material.useTexture = true;
    safeBottom->material.textureId = metalTex;
    safeBody->addChild(safeBottom);

    auto safeTop = std::make_shared<SceneObject>(prefix + "_top", "Safe Top Plate", PrimitiveType::Cube);
    safeTop->mesh = cubeMesh;
    safeTop->position = glm::vec3(0.0f, 0.48f, 0.0f);
    safeTop->scale = glm::vec3(1.24f, 0.12f, 1.24f);
    safeTop->material.diffuseColor = primaryColor * 0.9f;
    safeTop->material.useTexture = true;
    safeTop->material.textureId = metalTex;
    safeBody->addChild(safeTop);

    auto safeBack = std::make_shared<SceneObject>(prefix + "_back", "Safe Back Plate", PrimitiveType::Cube);
    safeBack->mesh = cubeMesh;
    safeBack->position = glm::vec3(0.0f, 0.0f, -0.52f);
    safeBack->scale = glm::vec3(1.24f, 0.96f, 0.14f);
    safeBack->material.diffuseColor = primaryColor * 0.85f;
    safeBack->material.useTexture = true;
    safeBack->material.textureId = metalTex;
    safeBody->addChild(safeBack);

    auto safeLeft = std::make_shared<SceneObject>(prefix + "_left", "Safe Left Wall", PrimitiveType::Cube);
    safeLeft->mesh = cubeMesh;
    safeLeft->position = glm::vec3(-0.52f, 0.0f, 0.0f);
    safeLeft->scale = glm::vec3(0.14f, 0.96f, 1.24f);
    safeLeft->material.diffuseColor = primaryColor;
    safeLeft->material.useTexture = true;
    safeLeft->material.textureId = metalTex;
    safeBody->addChild(safeLeft);

    auto safeRight = std::make_shared<SceneObject>(prefix + "_right", "Safe Right Wall", PrimitiveType::Cube);
    safeRight->mesh = cubeMesh;
    safeRight->position = glm::vec3(0.52f, 0.0f, 0.0f);
    safeRight->scale = glm::vec3(0.14f, 0.96f, 1.24f);
    safeRight->material.diffuseColor = primaryColor;
    safeRight->material.useTexture = true;
    safeRight->material.textureId = metalTex;
    safeBody->addChild(safeRight);

    // 4 Corner Reinforcement Pillars
    glm::vec3 corners[4] = {
        {-0.56f, 0.0f, -0.56f},
        { 0.56f, 0.0f, -0.56f},
        {-0.56f, 0.0f,  0.56f},
        { 0.56f, 0.0f,  0.56f}
    };
    for (int i = 0; i < 4; ++i) {
        auto pillar = std::make_shared<SceneObject>(prefix + "_corner_" + std::to_string(i), "Corner Pillar", PrimitiveType::Cube);
        pillar->mesh = cubeMesh;
        pillar->position = corners[i];
        pillar->scale = glm::vec3(0.10f, 1.08f, 0.10f);
        pillar->material.diffuseColor = accentColor * 0.6f;
        pillar->material.useTexture = true;
        pillar->material.textureId = metalTex;
        safeBody->addChild(pillar);
    }

    // Heavy Exterior Hinge Barrels on the outer left face (aligned with rotation axis)
    glm::vec3 hingeY[2] = { { -0.54f, 0.35f, 0.68f }, { -0.54f, -0.35f, 0.68f } };
    for (int i = 0; i < 2; ++i) {
        auto hBarrel = std::make_shared<SceneObject>(prefix + "_ext_hinge_" + std::to_string(i), "Heavy Hinge Barrel", PrimitiveType::Cylinder);
        hBarrel->mesh = cylinderMesh;
        hBarrel->position = hingeY[i];
        hBarrel->scale = glm::vec3(0.08f, 0.22f, 0.08f);
        hBarrel->material.diffuseColor = accentColor;
        hBarrel->material.useTexture = true;
        hBarrel->material.textureId = metalTex;
        safeBody->addChild(hBarrel);
    }

    // 3. Safe Interior (Shelf + Velvet Display Pillow)
    auto interiorShelf = std::make_shared<SceneObject>(prefix + "_shelf", "Steel Safe Shelf", PrimitiveType::Cube);
    interiorShelf->mesh = cubeMesh;
    interiorShelf->position = glm::vec3(0.0f, -0.12f, 0.0f);
    interiorShelf->scale = glm::vec3(0.92f, 0.05f, 0.92f);
    interiorShelf->material.diffuseColor = glm::vec3(0.24f, 0.25f, 0.28f);
    interiorShelf->material.useTexture = true;
    interiorShelf->material.textureId = metalTex;
    safeBody->addChild(interiorShelf);

    // Luxury Velvet Display Cushion under key
    auto velvetPillow = std::make_shared<SceneObject>(prefix + "_velvet_pillow", "Velvet Key Cushion", PrimitiveType::Cube);
    velvetPillow->mesh = cubeMesh;
    velvetPillow->position = glm::vec3(0.0f, -0.06f, 0.0f);
    velvetPillow->scale = glm::vec3(0.68f, 0.07f, 0.46f);
    velvetPillow->material.albedoColor = glm::vec3(0.55f, 0.06f, 0.10f);
    velvetPillow->material.metallic = 0.0f;
    velvetPillow->material.roughness = 0.90f;
    velvetPillow->material.diffuseColor = glm::vec3(0.55f, 0.08f, 0.12f); // Deep Crimson Velvet
    velvetPillow->material.specularColor = glm::vec3(0.2f);
    safeBody->addChild(velvetPillow);

    // Detailed Ornate Golden Key sitting on the cushion
    auto ornateKey = buildOrnateGoldenKey(interactionMgr);
    safeBody->addChild(ornateKey);

    // 4. Heavy Vault Articulated Door Assembly
    // Pivot located at front-left edge (-0.54, 0.0, 0.68)
    auto safeHinge = std::make_shared<SceneObject>("safe_hinge", "Safe Door Articulation", PrimitiveType::None);
    safeHinge->position = glm::vec3(-0.54f, 0.0f, 0.68f);
    safeHinge->basePosition = safeHinge->position;
    safeHinge->baseRotation = glm::vec3(0.0f);
    safeHinge->openRotationOffset = glm::vec3(0.0f, -115.0f, 0.0f); // Swings cleanly outward
    safeHinge->scale = glm::vec3(1.0f, 1.0f, 1.0f); // Full scale (1.0) so child door renders at true size!
    safeHinge->interactType = InteractionType::None; // Joint dummy node: never intercept raycasts
    safeHinge->isLocked = true;
    safeHinge->isOpen = false;
    safeBody->addChild(safeHinge);
    interactionMgr.safeDoorObject = safeHinge;

    // Heavy Armored Door Slab (Covers the full safe opening)
    auto safeDoor = std::make_shared<SceneObject>("safe_door_panel", "Reinforced Vault Door", PrimitiveType::Cube);
    safeDoor->mesh = cubeMesh;
    safeDoor->position = glm::vec3(0.54f, 0.0f, 0.0f);
    safeDoor->scale = glm::vec3(1.08f, 0.96f, 0.12f);
    safeDoor->material.albedoColor = primaryColor * 1.1f;
    safeDoor->material.metallic = 0.90f;
    safeDoor->material.roughness = 0.32f;
    safeDoor->material.diffuseColor = primaryColor * 1.1f;
    safeDoor->material.specularColor = glm::vec3(0.5f);
    safeDoor->material.shininess = 64.0f;
    safeDoor->material.useTexture = true;
    safeDoor->material.textureId = metalTex;
    safeDoor->interactType = InteractionType::SafeDoor; // Door panel itself is interactable
    safeHinge->addChild(safeDoor);

    // Beveled Inner Face Plate
    auto doorPlate = std::make_shared<SceneObject>(prefix + "_door_inner_plate", "Armored Faceplate", PrimitiveType::Cube);
    doorPlate->mesh = cubeMesh;
    doorPlate->position = glm::vec3(0.0f, 0.0f, 0.065f);
    doorPlate->scale = glm::vec3(0.92f, 0.84f, 0.03f);
    doorPlate->material.albedoColor = primaryColor * 0.8f;
    doorPlate->material.metallic = 0.92f;
    doorPlate->material.roughness = 0.28f;
    doorPlate->material.diffuseColor = primaryColor * 0.8f;
    doorPlate->material.useTexture = true;
    doorPlate->material.textureId = metalTex;
    safeDoor->addChild(doorPlate);

    // Chrome Rotary Vault Turn-Wheel (Spoke Wheel)
    auto vaultWheelHub = std::make_shared<SceneObject>(prefix + "_wheel_hub", "Vault Wheel Hub", PrimitiveType::Cylinder);
    vaultWheelHub->mesh = cylinderMesh;
    vaultWheelHub->position = glm::vec3(-0.12f, -0.06f, 0.09f);
    vaultWheelHub->rotation = glm::vec3(90.0f, 0.0f, 0.0f);
    vaultWheelHub->scale = glm::vec3(0.18f, 0.05f, 0.18f);
    vaultWheelHub->material.albedoColor = accentColor;
    vaultWheelHub->material.metallic = 0.98f;
    vaultWheelHub->material.roughness = 0.18f;
    vaultWheelHub->material.diffuseColor = accentColor;
    vaultWheelHub->material.specularColor = glm::vec3(1.0f);
    vaultWheelHub->material.shininess = 128.0f;
    safeDoor->addChild(vaultWheelHub);

    // Spoke Arms for the Vault Wheel
    auto spoke1 = std::make_shared<SceneObject>(prefix + "_spoke_1", "Wheel Spoke H", PrimitiveType::Cylinder);
    spoke1->mesh = cylinderMesh;
    spoke1->position = glm::vec3(-0.12f, -0.06f, 0.11f);
    spoke1->rotation = glm::vec3(0.0f, 0.0f, 90.0f);
    spoke1->scale = glm::vec3(0.03f, 0.38f, 0.03f);
    spoke1->material.albedoColor = accentColor;
    spoke1->material.metallic = 0.98f;
    spoke1->material.roughness = 0.18f;
    spoke1->material.diffuseColor = accentColor;
    safeDoor->addChild(spoke1);

    auto spoke2 = std::make_shared<SceneObject>(prefix + "_spoke_2", "Wheel Spoke V", PrimitiveType::Cylinder);
    spoke2->mesh = cylinderMesh;
    spoke2->position = glm::vec3(-0.12f, -0.06f, 0.11f);
    spoke2->scale = glm::vec3(0.03f, 0.38f, 0.03f);
    spoke2->material.albedoColor = accentColor;
    spoke2->material.metallic = 0.98f;
    spoke2->material.roughness = 0.18f;
    spoke2->material.diffuseColor = accentColor;
    safeDoor->addChild(spoke2);

    // Digital Keypad Console Unit
    auto keypadHousing = std::make_shared<SceneObject>("safe_keypad", "Electronic Security Keypad", PrimitiveType::Cube);
    keypadHousing->mesh = cubeMesh;
    keypadHousing->position = glm::vec3(0.24f, 0.18f, 0.085f);
    keypadHousing->scale = glm::vec3(0.26f, 0.34f, 0.04f);
    keypadHousing->material.albedoColor = glm::vec3(0.12f, 0.14f, 0.16f);
    keypadHousing->material.metallic = 0.5f;
    keypadHousing->material.roughness = 0.5f;
    keypadHousing->material.diffuseColor = glm::vec3(0.12f, 0.14f, 0.16f);
    keypadHousing->interactType = InteractionType::SafeKeypad;
    safeDoor->addChild(keypadHousing);

    // Keypad Glowing LED Display Screen
    auto keypadScreen = std::make_shared<SceneObject>(prefix + "_keypad_screen", "Keypad Screen", PrimitiveType::Cube);
    keypadScreen->mesh = cubeMesh;
    keypadScreen->position = glm::vec3(0.0f, 0.09f, 0.025f);
    keypadScreen->scale = glm::vec3(0.20f, 0.07f, 0.02f);
    keypadScreen->material.albedoColor = glm::vec3(0.1f, 0.95f, 0.4f);
    keypadScreen->material.emissiveColor = glm::vec3(0.1f, 0.95f, 0.4f);
    keypadScreen->material.emissiveIntensity = 2.5f; // Radiant green LED
    keypadScreen->material.diffuseColor = glm::vec3(0.1f, 0.85f, 0.4f);
    keypadScreen->material.specularColor = glm::vec3(0.8f);
    keypadHousing->addChild(keypadScreen);

    // 3 Chrome Locking Bolts sticking out from the side of the door
    for (int i = 0; i < 3; ++i) {
        float boltY = 0.26f - i * 0.26f;
        auto bolt = std::make_shared<SceneObject>(prefix + "_bolt_" + std::to_string(i), "Locking Bolt Pin", PrimitiveType::Cylinder);
        bolt->mesh = cylinderMesh;
        bolt->position = glm::vec3(0.54f, boltY, 0.0f);
        bolt->rotation = glm::vec3(0.0f, 0.0f, 90.0f);
        bolt->scale = glm::vec3(0.045f, 0.10f, 0.045f);
        bolt->material.albedoColor = accentColor;
        bolt->material.metallic = 1.0f;
        bolt->material.roughness = 0.14f;
        bolt->material.diffuseColor = accentColor;
        bolt->material.specularColor = glm::vec3(1.0f);
        bolt->material.shininess = 128.0f;
        safeDoor->addChild(bolt);
    }

    return safeRoot;
}

std::shared_ptr<SceneObject> EscapeRoomBuilder::buildExitDoor(
    InteractionManager& interactionMgr,
    const std::string& prefix,
    const glm::vec3& position,
    float rotationY,
    unsigned int frameTex,
    unsigned int doorTex,
    const glm::vec3& frameColor,
    const glm::vec3& doorColor,
    const glm::vec3& signGlowColor) {

    auto cubeMesh = PrimitiveGenerator::createCube();
    auto cylinderMesh = PrimitiveGenerator::createCylinder(32);

    auto doorRoot = std::make_shared<SceneObject>(prefix + "_root", "Exit Door Assembly", PrimitiveType::None);
    doorRoot->position = position;
    doorRoot->rotation = glm::vec3(0.0f, rotationY, 0.0f);
    doorRoot->interactType = InteractionType::ExitDoor; // Root itself is interactable

    // 1. Heavy Outer Architectural Door Frame
    auto frameLeft = std::make_shared<SceneObject>(prefix + "_frame_left", "Door Frame Left", PrimitiveType::Cube);
    frameLeft->mesh = cubeMesh;
    frameLeft->position = glm::vec3(-1.15f, 1.70f, 0.0f);
    frameLeft->scale = glm::vec3(0.30f, 3.40f, 0.30f);
    frameLeft->material.diffuseColor = frameColor;
    frameLeft->material.useTexture = true;
    frameLeft->material.textureId = frameTex;
    frameLeft->interactType = InteractionType::ExitDoor;
    doorRoot->addChild(frameLeft);

    auto frameRight = std::make_shared<SceneObject>(prefix + "_frame_right", "Door Frame Right", PrimitiveType::Cube);
    frameRight->mesh = cubeMesh;
    frameRight->position = glm::vec3(1.15f, 1.70f, 0.0f);
    frameRight->scale = glm::vec3(0.30f, 3.40f, 0.30f);
    frameRight->material.diffuseColor = frameColor;
    frameRight->material.useTexture = true;
    frameRight->material.textureId = frameTex;
    frameRight->interactType = InteractionType::ExitDoor;
    doorRoot->addChild(frameRight);

    auto frameTop = std::make_shared<SceneObject>(prefix + "_frame_top", "Door Frame Lintel", PrimitiveType::Cube);
    frameTop->mesh = cubeMesh;
    frameTop->position = glm::vec3(0.0f, 3.45f, 0.0f);
    frameTop->scale = glm::vec3(2.60f, 0.30f, 0.32f);
    frameTop->material.diffuseColor = frameColor;
    frameTop->material.useTexture = true;
    frameTop->material.textureId = frameTex;
    frameTop->interactType = InteractionType::ExitDoor;
    doorRoot->addChild(frameTop);

    auto frameStep = std::make_shared<SceneObject>(prefix + "_frame_step", "Door Threshold Step", PrimitiveType::Cube);
    frameStep->mesh = cubeMesh;
    frameStep->position = glm::vec3(0.0f, 0.05f, 0.0f);
    frameStep->scale = glm::vec3(2.50f, 0.10f, 0.40f);
    frameStep->material.diffuseColor = frameColor * 0.8f;
    frameStep->material.useTexture = true;
    frameStep->material.textureId = frameTex;
    doorRoot->addChild(frameStep);

    // 2. High-Visibility Illuminated EXIT Sign Box (Glowing beacon in the dark)
    auto signHousing = std::make_shared<SceneObject>(prefix + "_sign_box", "Exit Sign Housing", PrimitiveType::Cube);
    signHousing->mesh = cubeMesh;
    signHousing->position = glm::vec3(0.0f, 3.75f, 0.08f);
    signHousing->scale = glm::vec3(1.30f, 0.34f, 0.16f);
    signHousing->material.diffuseColor = glm::vec3(0.10f, 0.12f, 0.14f);
    signHousing->interactType = InteractionType::ExitDoor;
    doorRoot->addChild(signHousing);

    auto signPlate = std::make_shared<SceneObject>(prefix + "_sign_plate", "Glowing Exit Plate", PrimitiveType::Cube);
    signPlate->mesh = cubeMesh;
    signPlate->position = glm::vec3(0.0f, 0.0f, 0.09f);
    signPlate->scale = glm::vec3(1.18f, 0.24f, 0.03f);
    signPlate->material.albedoColor = signGlowColor;
    signPlate->material.emissiveColor = signGlowColor;
    signPlate->material.emissiveIntensity = 2.5f; // Radiant exit sign glow
    signPlate->material.diffuseColor = signGlowColor; // Neon Green or Flaming Amber
    signPlate->material.specularColor = glm::vec3(1.0f);
    signPlate->material.shininess = 128.0f;
    signPlate->interactType = InteractionType::ExitDoor;
    signHousing->addChild(signPlate);

    auto beaconL = std::make_shared<SceneObject>(prefix + "_beacon_l", "Exit Beacon Left", PrimitiveType::Cylinder);
    beaconL->mesh = cylinderMesh;
    beaconL->position = glm::vec3(-0.58f, 0.0f, 0.06f);
    beaconL->scale = glm::vec3(0.06f, 0.18f, 0.06f);
    beaconL->material.albedoColor = signGlowColor;
    beaconL->material.emissiveColor = signGlowColor;
    beaconL->material.emissiveIntensity = 1.8f;
    beaconL->material.diffuseColor = signGlowColor;
    signHousing->addChild(beaconL);

    auto beaconR = std::make_shared<SceneObject>(prefix + "_beacon_r", "Exit Beacon Right", PrimitiveType::Cylinder);
    beaconR->mesh = cylinderMesh;
    beaconR->position = glm::vec3(0.58f, 0.0f, 0.06f);
    beaconR->scale = glm::vec3(0.06f, 0.18f, 0.06f);
    beaconR->material.albedoColor = signGlowColor;
    beaconR->material.emissiveColor = signGlowColor;
    beaconR->material.emissiveIntensity = 1.8f;
    beaconR->material.diffuseColor = signGlowColor;
    signHousing->addChild(beaconR);

    // 3. Overhead Emergency Wall Sconce Lamp
    auto sconceArm = std::make_shared<SceneObject>(prefix + "_sconce_arm", "Sconce Arm", PrimitiveType::Cylinder);
    sconceArm->mesh = cylinderMesh;
    sconceArm->position = glm::vec3(0.0f, 4.02f, 0.18f);
    sconceArm->rotation = glm::vec3(90.0f, 0.0f, 0.0f);
    sconceArm->scale = glm::vec3(0.04f, 0.32f, 0.04f);
    sconceArm->material.albedoColor = glm::vec3(0.2f);
    sconceArm->material.metallic = 0.8f;
    sconceArm->material.roughness = 0.4f;
    sconceArm->material.diffuseColor = glm::vec3(0.2f);
    doorRoot->addChild(sconceArm);

    auto sconceBulb = std::make_shared<SceneObject>(prefix + "_sconce_bulb", "Emergency Downlight", PrimitiveType::Cube);
    sconceBulb->mesh = cubeMesh;
    sconceBulb->position = glm::vec3(0.0f, 3.92f, 0.34f);
    sconceBulb->scale = glm::vec3(0.20f, 0.12f, 0.20f);
    sconceBulb->material.albedoColor = glm::vec3(1.0f, 0.98f, 0.85f);
    sconceBulb->material.emissiveColor = glm::vec3(1.0f, 0.98f, 0.85f);
    sconceBulb->material.emissiveIntensity = 3.0f; // Radiant lamp beacon
    sconceBulb->material.diffuseColor = glm::vec3(1.0f, 0.98f, 0.85f); // Bright illuminated bulb
    sconceBulb->material.specularColor = glm::vec3(1.0f);
    sconceBulb->material.shininess = 128.0f;
    doorRoot->addChild(sconceBulb);

    // 4. Articulated Exit Door Joint & Slab
    auto doorHinge = std::make_shared<SceneObject>("door_hinge", "Exit Door Articulation", PrimitiveType::None);
    doorHinge->position = glm::vec3(-1.0f, 1.70f, 0.0f);
    doorHinge->basePosition = doorHinge->position;
    doorHinge->baseRotation = glm::vec3(0.0f);
    doorHinge->openRotationOffset = glm::vec3(0.0f, -105.0f, 0.0f); // Swings open cleanly into room
    doorHinge->scale = glm::vec3(1.0f, 1.0f, 1.0f);
    doorHinge->interactType = InteractionType::ExitDoor;
    doorHinge->isLocked = true;
    doorHinge->isOpen = false;
    doorRoot->addChild(doorHinge);
    interactionMgr.exitDoorObject = doorHinge;

    // Detailed Reinforced Door Slab
    auto exitDoor = std::make_shared<SceneObject>("exit_door_panel", "Reinforced Escape Door", PrimitiveType::Cube);
    exitDoor->mesh = cubeMesh;
    exitDoor->position = glm::vec3(1.0f, 0.0f, 0.0f);
    exitDoor->scale = glm::vec3(2.0f, 3.20f, 0.12f);
    exitDoor->material.diffuseColor = doorColor;
    exitDoor->material.specularColor = glm::vec3(0.4f);
    exitDoor->material.shininess = 32.0f;
    exitDoor->material.useTexture = true;
    exitDoor->material.textureId = doorTex;
    exitDoor->interactType = InteractionType::ExitDoor; // Main door panel is directly interactable!
    doorHinge->addChild(exitDoor);

    // Relief Panels on the Door Face
    auto upperPanel = std::make_shared<SceneObject>(prefix + "_panel_upper", "Door Upper Inset", PrimitiveType::Cube);
    upperPanel->mesh = cubeMesh;
    upperPanel->position = glm::vec3(0.0f, 0.72f, 0.025f);
    upperPanel->scale = glm::vec3(1.70f, 1.15f, 0.03f);
    upperPanel->material.diffuseColor = doorColor * 0.82f;
    upperPanel->material.useTexture = true;
    upperPanel->material.textureId = doorTex;
    upperPanel->interactType = InteractionType::ExitDoor;
    exitDoor->addChild(upperPanel);

    auto lowerPanel = std::make_shared<SceneObject>(prefix + "_panel_lower", "Door Lower Inset", PrimitiveType::Cube);
    lowerPanel->mesh = cubeMesh;
    lowerPanel->position = glm::vec3(0.0f, -0.72f, 0.025f);
    lowerPanel->scale = glm::vec3(1.70f, 1.15f, 0.03f);
    lowerPanel->material.diffuseColor = doorColor * 0.82f;
    lowerPanel->material.useTexture = true;
    lowerPanel->material.textureId = doorTex;
    lowerPanel->interactType = InteractionType::ExitDoor;
    exitDoor->addChild(lowerPanel);

    auto midBeam = std::make_shared<SceneObject>(prefix + "_panel_mid", "Door Reinforcement Beam", PrimitiveType::Cube);
    midBeam->mesh = cubeMesh;
    midBeam->position = glm::vec3(0.0f, 0.0f, 0.035f);
    midBeam->scale = glm::vec3(1.90f, 0.20f, 0.04f);
    midBeam->material.diffuseColor = doorColor * 1.25f;
    midBeam->material.useTexture = true;
    midBeam->material.textureId = frameTex;
    midBeam->interactType = InteractionType::ExitDoor;
    exitDoor->addChild(midBeam);

    // 5. Heavy Brass Lockplate & Golden Keyhole Fitting (Waist Height)
    auto lockPlate = std::make_shared<SceneObject>(prefix + "_lock_plate", "Brass Lock Assembly", PrimitiveType::Cube);
    lockPlate->mesh = cubeMesh;
    lockPlate->position = glm::vec3(0.72f, -0.08f, 0.065f);
    lockPlate->scale = glm::vec3(0.24f, 0.48f, 0.03f);
    lockPlate->material.diffuseColor = glm::vec3(1.0f, 0.84f, 0.25f); // Radiant Brass / Gold
    lockPlate->material.specularColor = glm::vec3(1.0f, 0.95f, 0.80f);
    lockPlate->material.shininess = 128.0f;
    lockPlate->interactType = InteractionType::ExitDoor;
    exitDoor->addChild(lockPlate);

    // Black Keyhole Cutout slot
    auto keyholeSlot = std::make_shared<SceneObject>(prefix + "_keyhole_slot", "Keyhole Slot", PrimitiveType::Cube);
    keyholeSlot->mesh = cubeMesh;
    keyholeSlot->position = glm::vec3(0.0f, 0.04f, 0.02f);
    keyholeSlot->scale = glm::vec3(0.04f, 0.10f, 0.02f);
    keyholeSlot->material.diffuseColor = glm::vec3(0.05f);
    keyholeSlot->interactType = InteractionType::ExitDoor;
    lockPlate->addChild(keyholeSlot);

    auto keyholeRing = std::make_shared<SceneObject>(prefix + "_keyhole_ring", "Keyhole Ring", PrimitiveType::Cylinder);
    keyholeRing->mesh = cylinderMesh;
    keyholeRing->position = glm::vec3(0.0f, 0.09f, 0.025f);
    keyholeRing->scale = glm::vec3(0.08f, 0.02f, 0.08f);
    keyholeRing->material.diffuseColor = glm::vec3(0.85f, 0.70f, 0.15f);
    keyholeRing->interactType = InteractionType::ExitDoor;
    lockPlate->addChild(keyholeRing);

    // Heavy Brass Lever Handle
    auto handleStem = std::make_shared<SceneObject>(prefix + "_handle_stem", "Handle Spindle", PrimitiveType::Cylinder);
    handleStem->mesh = cylinderMesh;
    handleStem->position = glm::vec3(0.0f, -0.10f, 0.05f);
    handleStem->rotation = glm::vec3(90.0f, 0.0f, 0.0f);
    handleStem->scale = glm::vec3(0.04f, 0.08f, 0.04f);
    handleStem->material.diffuseColor = glm::vec3(1.0f, 0.84f, 0.25f);
    handleStem->material.specularColor = glm::vec3(1.0f);
    handleStem->material.shininess = 128.0f;
    handleStem->interactType = InteractionType::ExitDoor;
    lockPlate->addChild(handleStem);

    auto handleGrip = std::make_shared<SceneObject>(prefix + "_handle_grip", "Heavy Brass Lever Handle", PrimitiveType::Cube);
    handleGrip->mesh = cubeMesh;
    handleGrip->position = glm::vec3(-0.12f, -0.10f, 0.08f);
    handleGrip->scale = glm::vec3(0.24f, 0.04f, 0.04f);
    handleGrip->material.diffuseColor = glm::vec3(1.0f, 0.84f, 0.25f);
    handleGrip->material.specularColor = glm::vec3(1.0f);
    handleGrip->material.shininess = 128.0f;
    handleGrip->interactType = InteractionType::ExitDoor;
    lockPlate->addChild(handleGrip);

    return doorRoot;
}
