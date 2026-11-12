# Task Plan: Unique Spatial Layouts & 3 Distinct Puzzle Mechanics

## Goal
Redesign all 3 escape rooms with unique architectural dimensions and distinct, non-repetitive puzzle mechanics utilizing modern OpenGL features (PBR, Dynamic Shadows, UV Blacklight/Bloom, Planar Reflection layering), while updating player capsule collision, diegetic HUD objective hints, and ensuring 100% CTest pass.

## Current Status
- [x] Phase 1: Architecture Design & Core Interfaces (`RenderLayer`, UV Light uniform, Dynamic Room Bounds)
- [x] Phase 2: Room 1 (`OccultRitualRoom`) — $12\text{m} \times 12\text{m}$, 4 Pillars, Shadow Alignment Totem Puzzle
- [x] Phase 3: Room 2 (`AbandonedAsylumRoom`) — $14\text{m} \times 8\text{m}$ L-Corridor, Iron Bars Partition, Fuses & Breaker, UV Blacklight Shader
- [x] Phase 4: Room 3 (`MirrorRoom`) — $8\text{m} \times 16\text{m}$ Gallery, Planar Mirror-Only Clues & Symmetry Puzzle
- [x] Phase 5: Player Controller Capsule Collision & Dynamic HUD Objective Hints
- [x] Phase 6: Test Suite Updates (`RegressionTest`, `SmokeTest`), Build & Verification (0 errors, 0 warnings, 100% CTest)

## Key Technical Decisions
1. **SceneObject RenderLayer**: `RenderLayer::Both`, `RenderLayer::RealOnly`, `RenderLayer::MirrorOnly` for mirror-only clues.
2. **UV Blacklight**: Add `uniform bool isUvLightActive;` and `uniform bool isFluorescent;` to `pbr.frag`. When active, fluorescent painted runes glow with high emissive radiance ($5.0\times$) feeding directly into HDR bloom.
3. **Dynamic Room Bounds**: Add `glm::vec4 roomBounds` (`minX, maxX, minZ, maxZ`) to `BaseRoom`, queried by `PlayerController`.
4. **Collision**: Collect all partition walls, iron bars, pillars, totems, and pedestals in `PlayerController::collectStaticColliders`.
5. **Puzzle State Machines**: Implement specific handlers in `InteractionManager` for Totems, Fuses, Circuit Breaker, UV light toggle, and Mirror Busts.
