# Known Issues & Technical Limitations

This document lists all known behaviors, design compromises, and non-critical limitations of the **3D Escape Room Simulation** build as of the final demo freeze (`demo-ready`).

---

### 1. Rendering & Graphics Pipeline

- **Single Planar Reflection Surface:**
  - *Behavior:* Only the North Wall mirror in Room 2 (`center_mirror`) computes a real-time planar reflection.
  - *Reasoning:* Running multiple reflection passes per frame would double or triple draw calls, which would drop frame rates below 60 FPS on Intel UHD integrated graphics.
  - *Impact:* None for intended gameplay; Room 0 and Room 1 rely on PBR ambient IBL split-sum cubemaps instead.

- **Point Light Static Cubemap Shadow Caching:**
  - *Behavior:* Point light depth cubemaps are cached and updated only when dynamic objects (doors, drawers, altar lids) animate or change state.
  - *Reasoning:* Re-rendering 6 cubemap faces every frame on Intel UHD costs ~8ms, dropping FPS below 60. Caching keeps frame times at ~5.4ms (180+ FPS).

- **Screen-Space Particles:**
  - *Behavior:* Atmospheric dust motes are rendered in the forward pass over the main view and do not appear as reflected geometry inside the mirror pass.
  - *Reasoning:* Avoids redundant particle sorting and blending overhead in the reflection framebuffer.

---

### 2. UI & Interaction System

- **ASCII English Font Constraint:**
  - *Behavior:* All HUD hints, room names, item names, and ImGui overlays use standard English ASCII characters.
  - *Reasoning:* Avoids unrendered unicode question mark glyphs (`?`) in Dear ImGui's default monospace font texture atlas without bundling heavy external TTF fonts.

- **Modal Mouse Capture:**
  - *Behavior:* When entering the electronic keypad or 3D inspect view, the mouse cursor is freed to `GLFW_CURSOR_NORMAL`. Closing the modal re-locks the cursor to `GLFW_CURSOR_DISABLED`.
  - *Workaround:* If the window loses focus via `Alt+Tab`, clicking back inside the window properly restores mouse capture.

---

### 3. Gameplay Mechanics

- **Fast Room Transitions:**
  - *Behavior:* Hotkeys `[F1]`, `[F2]`, `[F3]`, and `[F4]` instantly switch rooms and solve puzzles.
  - *Reasoning:* Designed specifically for presentation judges to allow quick inspection of any room without requiring full linear progression.
