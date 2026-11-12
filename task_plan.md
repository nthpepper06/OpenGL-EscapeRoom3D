# Task Plan: Bug Fixes, UI English Standardization, UV Blacklight Clues, and Demo Shortcuts (F1-F4)

## Goal
Implement 3 cohesive upgrades:
1. Fix 4 visual and UI display bugs (100% English ASCII strings for ImGui/HUD, fix mirror flickering/blinding specular blowout, fix pillar geometry/culling in OccultRitualRoom, completely disable firefly dust particle system).
2. Upgrade UV Blacklight flashlight into an essential puzzle mechanic in `AbandonedAsylumRoom` (UV shader discard/neon emission, 3 fluorescent trail arrows leading to Fuse 2, 4 indexed number clue plaques for PIN 4108 in isolation cell, HUD status).
3. Implement robust Demo Shortcuts `F1-F4` with edge-triggered key handling, clean room transition state resets, colliders reload, and instant puzzle solve in `F4`.

## Implementation Phases
- [ ] Phase 1: Planning, Audit & Disabling Dust Particles
  - Audit existing particle system rendering in `Application.cpp` and disable it.
  - Review all Vietnamese strings in UI, HUD, InteractionManager, Room headers, and GuiLayer; convert 100% to English ASCII.
- [ ] Phase 2: Fix Mirror Flickering & Blinding Specular Blowout, Fix Stone Pillars
  - In `PlanarReflection.cpp` / `Application.cpp`: add slight bias to `clipPlane` to prevent Z-fighting at mirror surface, configure `glFrontFace(GL_CW)` / disable face culling during reflection pass, clamp mirror specular / reflection luminance to avoid HDR bloom blowout.
  - In `OccultRitualRoom.cpp` and `PrimitiveGenerator.cpp`: inspect and fix cylinder/pillar winding order, normals, UVs, and boundary intersection with floor/ceiling.
- [ ] Phase 3: UV Blacklight Shader & Puzzle Mechanic in Room 2
  - In `assets/shaders/pbr.frag`: implement `uIsUVMode`, `uIsUVReactive`, discard if UV off or outside cone (`spotFactor <= 0.05`), neon emission `vec3(0.1, 1.0, 0.65) * 4.5 * spotFactor`.
  - In `src/renderer/Material.hpp` & `src/scene/SceneObject.cpp`: wire `isUVReactive`.
  - In `src/rooms/AbandonedAsylumRoom.cpp`: add 3 UV reactive arrows leading to Fuse 2; add 4 UV reactive number plates (`1: 4`, `2: 1`, `3: 0`, `4: 8`) in isolation cell corners.
  - In `src/interaction/InteractionManager.cpp` and `src/core/Application.cpp`: support toggle with Right-Click and `V` (and keep `F` if desired), update HUD status.
- [ ] Phase 4: Demo Shortcuts (`F1`, `F2`, `F3`, `F4`)
  - Edge-triggered keypress handling in `Application::processInput`.
  - Safe cleanup on room switch (reset collider list, reset inspect mode & keypad, lock cursor, trigger shadow/batching rebuild, notification message).
  - Implement `F4` solver logic per room.
- [ ] Phase 5: Verification & Regression Testing
  - Rebuild with `cmake --build` (0 errors, 0 warnings).
  - Run CTest `EscapeRoom_RegressionTest` and `EscapeRoom_SmokeTest` to ensure 100% pass.
  - Test F1-F4 paths, capture updated screenshots, generate report.
