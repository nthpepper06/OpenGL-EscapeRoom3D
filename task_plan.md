# Phase 4 Plan: Gameplay, Interaction & Puzzle System (Full)

## Project Context
- **Target Platform**: Intel UHD Graphics, OpenGL 3.3 Core Profile (no compute shaders).
- **Target Performance**: 1280x720, $\ge 60$ FPS on "Medium" preset.
- **Git Branch**: `phase4-gameplay` (branched from tag `phase1-done`). Local commits only, no git push.
- **Narrative**: Dr. Elias Vance occult asylum sacrifice lore. Rooms: OccultRitualRoom -> AbandonedAsylumRoom -> MirrorRoom. 3 Sigil Fragments + 3 Golden Keys needed for final escape. Target play time ~40 min.

---

## Technical Audit of Existing System

| Subsystem | Current State | Deficiencies & Gaps |
| :--- | :--- | :--- |
| **Input & Camera** | Direct WASD keyboard polling modifying `camera.position` directly by `speed * dt`. | No acceleration/deceleration, no inertia, no head-bob, no sprint, fixed speed. |
| **Collision** | Hardcoded box clamp: `camera.clampToBounds(min, max)` | Zero physical collision with furniture, props, or interior walls. Player walks straight through desks, altar, safe. |
| **Interaction** | Center raycast against `obj->getWorldAABB()` with 3.5m limit. | Lacks 3D inspection turntable, no item pickup, text-only notes in status line. |
| **Inventory** | Ad-hoc single boolean `bool hasGoldenKey` in `InteractionManager`. | No inventory data structure, no item combination, no UI hotbar/slots, no "use on". |
| **Room Switching** | `Application::switchRoom` resets booleans and rebuilds room nodes. | Items and puzzle state do not carry over across rooms. Keys and sigils cannot be combined. |
| **Puzzle State** | Hardcoded PIN comparisons: `6661`, `4108`, `8820` directly in code. | Rigid, linear, no data-driven JSON, no hint system, no journal, no difficulty presets, no softlock detection. |

---

## Phase 4 Roadmap & Gate Specifications

### Gate 4A: Player Controller, Collision & Kinematic Props
- [ ] **1. First-Person Player Controller**:
  - `PlayerController` class wrapping `Camera`.
  - Capsule-vs-AABB collision (capsule radius = 0.35m, height = 1.8m, eye height = 1.65m) with smooth wall sliding.
  - Sub-stepping (fixed $\Delta t = 1/60$s) to prevent tunneling at low framerates.
  - Smooth acceleration/deceleration, optional sprint (Shift), subtle head-bob toggle.
  - User settings: FOV ($50^\circ-90^\circ$), mouse sensitivity, head-bob on/off.
- [ ] **2. Interaction & Collider Synchronization**:
  - Center raycast against colliders synchronized with animated mesh world matrices.
  - Contextual interaction prompt, highlight outline, range limit (2.5m).
  - Regression test asserting `collider == mesh.getWorldMatrix()[3]`.
- [ ] **3. 3D Item Pickup & Inspection System**:
  - Inspect mode freezing player movement.
  - Inspected 3D mesh turntable rotation via mouse drag in front of camera.
  - High-resolution readable note overlay via ImGui with custom serif typography. Close with `[E]` or `[Esc]`.
- [ ] **4. Kinematic Props & Easing**:
  - Animated props (drawers, doors, safe doors, lids) with cubic easing.
  - All transform mutations invoke `setPosition/setRotation`, marking dirty and bumping `sceneRevision`.
- [ ] **5. Kinematic Drop Physics**:
  - Simple gravity raycast resolving dropped items to nearest support surface AABB.
- **Verification Gate 4A**: Build 0 errors, tests pass, 3-room walk play-test report with 0 clipping, interact with 5 objects per room. STOP.

---

### Gate 4B: Data-Driven Puzzle Framework
- [ ] **1. Core Types & Event Bus**:
  - `PuzzleState` enum (`Locked`, `Available`, `InProgress`, `Solved`).
  - Decoupled `EventBus` (`OnItemPickedUp`, `OnObjectUsed`, `OnCodeEntered`, `OnPuzzleSolved`, `OnRoomChanged`).
- [ ] **2. Data-Driven JSON Parser**:
  - Per-room puzzle JSON files in `assets/puzzles/`.
  - Schema: interactables (`id`, `name`, `type`, `requires[]`, `grants[]`, `hints[3]`, `onSolve`), puzzles (`id`, `type`, `solution`, `dependsOn[]`).
  - Robust error handling with `try/catch` and safe fallback without crashes.
- [ ] **3. Puzzle Mechanics Handlers**:
  - `CodeLock` (multi-digit keypad).
  - `DialLock` (multi-turn combination dial).
  - `OrderedSequence` (candles, buttons).
  - `ItemCombine` (assemble broken artifacts).
  - `RiddleChoice` (multiple-choice Latin inscriptions).
  - `MirrorBeam` (CPU 3D light reflection).
  - `GridPath` (safe pressure tiles).
- [ ] **4. Inventory & Journal Systems**:
  - Inventory manager + ImGui hotbar/dock with item select, "use on", and combination.
  - Journal (key `J`): auto-logs discovered clues, notes, codes, and mirrored text.
- [ ] **5. Hints & Difficulty Presets**:
  - 3 hint tiers (direction, object, near-answer) with cooldown timer.
  - Difficulty presets (`Easy`, `Normal`, `Hard`).
- [ ] **6. Save/Load & No-Softlock Validator**:
  - Automated JSON save after each solve and room change (`savegame.json`).
  - Startup graph simulator verifying reachability of all puzzles from initial state.
- [ ] **7. Game Flow Screens**:
  - Start Menu, Pause Menu (`Esc`), Game HUD, Win Screen with stats (time, hints, score).
- **Verification Gate 4B**: Unit tests for validator and manager, demo JSON puzzle solve, `docs/PUZZLE_SCHEMA.md`. STOP.

---

### Gate 4C: Room 1 — Occult Ritual Chamber (~10-12 min)
- [ ] **1. Three Hidden Diary Fragments**:
  - Fragment 1 under altar rug, Fragment 2 behind framed portrait, Fragment 3 in secret desk drawer.
  - Forms 5-line elemental poem.
- [ ] **2. Pentagram Candle Lighting Sequence**:
  - Light 5 candles in elemental order (Earth -> Water -> Fire -> Wind -> Void).
  - Candle bases have grazing-angle symbols. Wrong order extinguishes candles and flashes red.
- [ ] **3. Caesar Cipher Altar Drawer**:
  - Latin cipher decoded with shift = (lit candles = 5) + (missing petals = 2) = 7.
- [ ] **4. Roman Numeral Dial Safe**:
  - 3-turn dial with numbers derived from wall inscriptions.
  - Grants **Golden Key #1** + **Sigil Fragment #1**.
  - Red herring: Bloody "666" glyph.
- **Verification Gate 4C**: Full walkthrough test, wrong-input recovery, regression check. STOP.

---

### Gate 4D: Room 2 — Abandoned Psychiatric Ward (~12-15 min)
- [ ] **1. Emergency Power & Fuse Box Puzzle**:
  - Pitch black room; find replacement fuse and wire in color sequence from nurse duty calendar.
- [ ] **2. Patient File Logic Grid**:
  - 4 patients, 4 cells, 4 medicines. Cross-referencing records reveals target cell and dosage.
- [ ] **3. Safe-Tile Pressure Floor (GridPath)**:
  - Hallway pressure tiles; correct path encoded in nurse shift table. Wrong step triggers steam pushback and flashlight flicker.
- [ ] **4. Medicine Cabinet Dispenser**:
  - Place 3 correct pill bottles in dispenser trays -> unlocks compartment with **Golden Key #2** + **Sigil Fragment #2**.
  - Red herring: Damaged patient X-ray.
- **Verification Gate 4D**: Full walkthrough test, wrong-input recovery, regression check. STOP.

---

### Gate 4E: Room 3 — Haunted Mirror Sanctuary (~15-20 min)
- [ ] **1. Mirrored Inscriptions**:
  - Reversible notes readable only in mirror reflection. Clue indicates the "Mirror of Truth".
- [ ] **2. Spot-the-Difference Grand Mirror**:
  - Reflection exhibits 5 altered occult details, each providing a numerical clue.
- [ ] **3. MirrorBeam Light Reflection Puzzle**:
  - Orient 3 pivoting vanity mirrors to reflect wall beam around obstacles onto altar sensor.
- [ ] **4. Sigil Altar & Exit Portal**:
  - Assemble 3 Sigil Fragments in triangular altar socket.
  - Final portal requires 3 Golden Keys + assembled Sigil.
  - Two distinct endings: Shatter Mirror (banishment) vs Soul Liberation (redemption code).
- **Verification Gate 4E**: Full 3-room playthrough from clean save, recorded step list, regression check. STOP.

---

### Gate 4F: Polish, Benchmarking & Documentation
- [ ] Playtest checklist (rapid input spam, mid-game save/load, Esc inspect recovery).
- [ ] GPU benchmark (`GL_TIME_ELAPSED` mean/p95/p99) confirming $\ge 60$ FPS on Intel UHD.
- [ ] README section "Gameplay & Puzzle System" + dependency graph diagram.
- [ ] Tag `phase4-done`.
