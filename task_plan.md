# Master Modernization Roadmap: Phase 2 through Phase 6

## Project Overview
- **Project**: 3D Escape Room Simulation (Modern OpenGL 3.3 Core Profile / C++17 / Dear ImGui).
- **Branch**: `modernization-pipeline` (local commits only, zero remote push).
- **Target Hardware**: Intel UHD Graphics, 1280x720, $\ge 60$ FPS on Medium preset.
- **Execution Strategy**: Strict sequential gates (Phase 2 -> Phase 3 -> Phase 4 -> Phase 5 -> Phase 6).
- **Gate Rule**: After each Phase implementation, compile with `cmake --build build`, run regression diagnostics. 0 compiler errors, 0 GLSL errors, 100% test pass required to advance.

---

## Roadmap & Status Tracking

### [ ] GATE 2 — PHASE 2: Real-Time Shadows, Atmosphere & SSAO
- [ ] **2.1 Shadow Pipeline (`ShadowPipeline.hpp/cpp`)**:
  - [ ] SpotLight 2D Depth FBO (`GL_DEPTH_COMPONENT24`, 512–2048) + Rotated Poisson Disk PCF + Normal-offset & slope-scaled bias.
  - [ ] PointLight Omnidirectional Depth Cubemap (`GL_TEXTURE_CUBE_MAP`, 512x512 per face) via 6-pass FBO attachment without Geometry Shader.
  - [ ] Static Shadow Caching: Bake point shadow once, re-render only when `isHierarchyMoving()` is true.
  - [ ] Direct Lighting Decoupling: Shadow strictly modulates `Lo_point` and `Lo_spot`, preserving IBL ambient and emissive.
- [ ] **2.2 Atmosphere & HDR Post-FX**:
  - [ ] 5-mip Dual-filter / Karis HDR Bloom pyramid before ACES tonemapping.
  - [ ] Exponential Height Fog + Flashlight in-scatter analytic phase function in `pbr.frag`.
  - [ ] Floating Dust Particles (`ParticleSystem.hpp/cpp`, `GL_POINTS`, single draw call, brownian drift).
  - [ ] Smooth multi-harmonic sine noise for candle & bulb flicker.
  - [ ] Post-tonemap 35mm film grain, vignette, and chromatic aberration.
- [ ] **2.3 SSAO Pipeline (`SsaoPipeline.hpp/cpp`)**:
  - [ ] Half-res depth/normal buffer (640x360), 16 hemisphere samples, 4x4 noise, bilateral blur.
  - [ ] Applied to ambient/IBL term only; skipped in mirror pass.
- [ ] **Gate 2 Verification**: Build 0 errors, tests pass, shadows & atmosphere operational in all 3 rooms.

---

### [ ] GATE 3 — PHASE 3: Geometry Bevels, Planar Mirror & Draw-Call Batching
- [ ] **3.1 Chamfered/Beveled Primitives & Wear**:
  - [ ] Beveled box generator (`PrimitiveGenerator::createBeveledCube`) for desks, cabinets, frames, safe.
  - [ ] Procedural wear/scratch roughness modulation.
- [ ] **3.2 Planar Mirror Reflection Pass**:
  - [ ] Virtual reflected camera with oblique clipping plane for grand mirrors in `MirrorRoom`.
  - [ ] Reuses pre-rendered shadow maps without re-rendering shadows.
- [ ] **3.3 Static Geometry Batching**:
  - [ ] Batch static room architecture sharing materials into single VAOs.
  - [ ] Reduce draw calls from >300 to <80 per frame.
- [ ] **Gate 3 Verification**: Build 0 errors, tests pass, mirror reflections and batching verified.

---

### [ ] GATE 4 — PHASE 4: Physical Controller, Inspection & Interaction
- [ ] **4.1 Physical Player Controller (`PlayerController.hpp/cpp`)**:
  - [ ] Swept cylinder/capsule-vs-AABB collision with smooth wall sliding.
  - [ ] Sub-stepped physics ($\Delta t = 1/60$s) to eliminate tunneling.
  - [ ] Smooth acceleration/deceleration, sprint (Shift), toggleable head-bobbing.
- [ ] **4.2 3D Inspection & Easing (`InspectSystem.hpp/cpp`)**:
  - [ ] 3D Item Inspection mode with free mouse turntable rotation and styled note modal.
  - [ ] Cubic/smoothstep easing for doors, drawers, safe door with `sceneRevision` dirtying.
- [ ] **Gate 4 Verification**: Build 0 errors, collision navigation verified in all 3 rooms, inspect functional.

---

### [ ] GATE 5 — PHASE 5: Spatial 3D Audio & Diegetic HUD
- [ ] **5.1 Spatial 3D Audio Engine (`AudioEngine.hpp/cpp`)**:
  - [ ] Single-header `miniaudio.h` integration (zero DLL dependencies).
  - [ ] Positional 3D audio listener & sound emitters with distance attenuation.
  - [ ] Procedural synthesized audio (footsteps, electrical buzz, safe dial clicks, door creaks, ambient drone).
- [ ] **5.2 Diegetic HUD & Settings Menu**:
  - [ ] Dynamic contextual crosshair, clean prompts, comprehensive settings menu.
- [ ] **Gate 5 Verification**: Build 0 errors, 3D audio & HUD verified.

---

### [ ] GATE 6 — PHASE 6: Scalable Presets & Real-Time GPU Profiler
- [ ] **6.1 Quality Presets (Low / Medium / High / Ultra)**:
  - [ ] Dynamic runtime switching of shadow resolution, PCF taps, SSAO samples, Bloom, and Anisotropy.
- [ ] **6.2 Real-Time GPU Profiler**:
  - [ ] `GL_TIME_ELAPSED` hardware query timers for Shadow Pass, Geometry Pass, SSAO, Post-Process.
  - [ ] FPS, frame time graph, draw call & triangle counters.
- [ ] **Gate 6 Verification**: Build 0 errors, benchmark report, all 6 phases validated.
