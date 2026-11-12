# Changelog

All notable changes to the 3D Escape Room project will be documented in this file.

## [Regression Fix & Verification] - 2026-11-12

### Bug Fix: Hierarchical Transform Invalidation for Drawer & Safe / Locked Cabinet
- **Bug Description**:
  - Interacting with drawers in all rooms (`desk_drawer`) and unlocking the safe/locked cabinet (`safe_hinge` / `safe_door_panel`) triggered game logic and state updates, but the visible 3D meshes (`drawer_front`, `drawer_bottom`, `clue_note`, `safe_door_panel`, dials, keypads) remained frozen at delta 0.0 in the closed configuration.
  - In `AbandonedAsylumRoom`, aiming at the outer frame of the Medical File Cabinet (`cabinet_root`) did not interact because `interactType` was set to `None`.
- **Root Cause Analysis (Regression)**:
  - During Phase 1 PBR and CPU performance optimization, `cachedWorldMatrix`, `cachedNormalMatrix`, and `isTransformDirty` caching were introduced into `SceneObject` to avoid redundant CPU-side matrix inverses and transpositions every frame.
  - However, when a parent articulated node (such as `desk_drawer` or `safe_hinge`) updated its local transform during animation, `markTransformDirty()` was never cascaded down to child `SceneObject`s.
  - Because child objects had static local transforms relative to their parent, their local `position == lastPosition` check evaluated to false, leaving child `isTransformDirty = false`.
  - Child meshes returned the stale cached world matrix from frame 0, preventing visible geometry from moving or rotating.
- **Resolution**:
  - Added encapsulation setters in `SceneObject`: `setPosition(vec3)`, `setRotation(vec3)`, `setScale(vec3)` that automatically invoke recursive `markTransformDirty()`.
  - Updated `SceneObject::getWorldMatrix()` to detect direct transform field modifications and cascade `markTransformDirty()` to all descendant nodes.
  - Refactored `SceneObject::update(float deltaTime)` to utilize `setPosition` and `setRotation` rather than directly writing to public fields.
  - Updated `InteractionManager::getPromptText` and `InteractionManager::interact` for `DeskDrawer` to support hierarchy target resolution (`auto target = drawerObject ? drawerObject : obj;`).
  - Added `InteractionType::DeskDrawer` to `cabinet_root` and `drawer_front` across all rooms (`OccultRitualRoom`, `AbandonedAsylumRoom`, `MirrorRoom`) for seamless raycast detection.
- **Verification Evidence**:
  - Automated diagnostic trace confirmed visible mesh world displacement deltas across all rooms:
    - Room 0: `drawer_front Delta = 1.105` m, `safe_door_panel Delta = 0.911` m.
    - Room 1: `drawer_front Delta = 0.765` m, `safe_door_panel Delta = 0.911` m.
    - Room 2: `drawer_front Delta = 0.850` m, `safe_door_panel Delta = 0.911` m.
