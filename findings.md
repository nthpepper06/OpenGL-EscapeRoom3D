# Research & Architectural Findings

## Existing Systems Review
1. **PlayerController Collision**:
   - `PlayerController::resolveCollisions` hardcoded room bounds `[-4.5, 4.5]`.
   - `collectStaticColliders` checked node name substrings like `altar`, `stand`, `cabinet`, `gurney`, `desk`, `pillar`.
   - Sliding capsule with 3 sub-steps along X and Z works cleanly when AABBs are properly updated.
2. **PBR Shader & Shading Pipeline**:
   - `assets/shaders/pbr.frag` and `pbr.vert` handle all lighting, shadows, SSAO, planar reflections, and HDR tonemapping in post-processing.
   - Flashlight is `SpotLight spotLight`. Toggleable UV mode can set `spotLight.color = vec3(0.4, 0.1, 1.0)` and set a boolean `isUvLightActive = true`.
   - Materials with `isFluorescent = true` can activate intense emissive bloom when lit by the UV flashlight.
3. **Planar Reflection Pipeline**:
   - `PlanarReflection` renders the scene into a 512x512 FBO with an oblique clipping projection matrix.
   - `sceneRoot->draw(lightingShader, hoveredId, selectedId)` is called during both the reflection pass and the main scene pass.
   - Adding a `passLayer` parameter (`Both`, `RealOnly`, `MirrorOnly`) allows rendering mirror-only objects exclusively into `PlanarReflection`'s FBO!
4. **Interaction System**:
   - `InteractionType` enum can be expanded with `Totem`, `FuseItem`, `BreakerBox`, `MirrorBust`, `IsolationGate`.
   - `InteractionManager` can track dynamic objective hints, totem angles, fuse count, power state, and bust orientations.
