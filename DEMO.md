# 3D Escape Room Simulation — Live Presentation & Demo Guide
**Course:** Computer Graphics (OpenGL 3.3 Core Profile / C++17)  
**Target Platform:** Intel UHD Graphics / Windows 10/11  
**Build Status:** Demo-Ready (Tag: `demo-ready`)

---

## 1. Quick Start & Execution

```powershell
# Build Release executable (Ninja / CMake)
cmake --build build --config Release

# Run game directly
.\build\EscapeRoom.exe

# Run automated tests
ctest --test-dir build --output-on-failure

# Run headless smoke test
.\build\EscapeRoom.exe --smoke-test
```

---

## 2. Controls Reference

| Input | Action | Description |
| :--- | :--- | :--- |
| **W, A, S, D** | First-Person Movement | Smooth acceleration and capsule sliding collision |
| **Mouse Look** | Camera Pitch & Yaw | First-person camera look with vertical pitch clamping ($\pm 89^\circ$) |
| **[E]** | Interact | Pickup items, rotate totems/busts, open breaker, examine keypad |
| **[F]** | Toggle Flashlight | Calibrated HDR spotlight with smoothstep cone falloff |
| **[V] / Right-Click** | Toggle UV Blacklight | Switches flashlight to UV mode for fluorescent decal discovery |
| **[Tab]** | Play / Edit Mode | Toggles between gameplay FPS mode and ImGui debug inspector |
| **[Esc]** | Close Modal / Cancel | Closes keypad modal or 3D inspect view, re-captures mouse |
| **[F1]** | Jump to Room 0 | Occult Ritual Grand Chamber ($12\text{m} \times 12\text{m}$) |
| **[F2]** | Jump to Room 1 | Abandoned Psychiatric Ward ($14\text{m} \times 8\text{m}$) |
| **[F3]** | Jump to Room 2 | Haunted Mirror Grand Gallery ($8\text{m} \times 16\text{m}$) |
| **[F4]** | Auto-Solve Puzzle | Instantly solves current room puzzle for rapid demo progression |

---

## 3. Presenter's 5-Minute Live Walkthrough Script

### Introduction (0:00 - 0:45)
- **Goal:** Set the atmosphere and highlight the graphics pipeline.
- *"Welcome everyone. Today we are demonstrating our 3D Escape Room engine, built from scratch in C++17 and Modern OpenGL 3.3 Core Profile. The engine runs a fully physically based rendering (PBR) deferred pipeline with HDR tone mapping, SSAO, screen-space bloom, dynamic shadows, and real-time planar reflections on Intel UHD integrated graphics at over 160 FPS."*

### Room 1: Occult Ritual Grand Chamber (0:45 - 2:00)
- **Mechanic:** Shadow-Totem Alignment Puzzle.
- **Narrative:** Dr. Elias Vance used asylum patients for arcane rituals.
- **Action Steps:**
  1. Point out the central altar and the 3 ancient stone totems.
  2. Press **[E]** on Totem 1 once ($90^\circ$ - North).
  3. Press **[E]** on Totem 2 twice ($180^\circ$ - East).
  4. Press **[E]** on Totem 3 three times ($270^\circ$ - South).
  5. The stone altar lid slides open with sound. Press **[E]** on the altar to pick up **Golden Key #1** and **Sigil Fragment #1**.
  6. Walk to the south door, aim at the lock plate, and press **[E]** to unlock the door and advance to Room 2.

### Room 2: Abandoned Psychiatric Ward (2:00 - 3:30)
- **Mechanic:** Electrical Restoration & UV Fluorescent Decal Cipher.
- **Narrative:** The power is cut; an iron security gate bars the exit.
- **Action Steps:**
  1. Walk to the metal medical trolley on the west side. Press **[E]** to pick up **Fuse #1**.
  2. Walk to the dirty ceramic sink recess on the east side. Press **[E]** to pick up **Fuse #2**.
  3. Walk to the electrical breaker box next to the iron gate. Press **[E]** to install both fuses.
  4. The ceiling fluorescent lights flicker on with electrical audio, and the heavy iron gate slides upward.
  5. Right-click or press **[V]** to activate the **UV Blacklight**.
  6. Shine the beam across the walls to reveal the fluorescent cipher marks `[1]: 4`, `[2]: 1`, `[3]: 0`, `[4]: 8`.
  7. Walk through the opened gate to the exit door keypad. Press **[E]** to open the modal.
  8. Type `4108` on the physical keyboard (or click ImGui buttons). The keypad beeps green, the door unlocks, and **Sigil Fragment #2** is awarded.
  9. Press **[E]** on the door to transition to Room 3.

### Room 3: Haunted Mirror Grand Gallery (3:30 - 4:45)
- **Mechanic:** Spectral Reflection Anomalies & Bust Symmetry.
- **Narrative:** The Great Victorian Demon Mirror reveals spectral guides that only exist in the mirror dimension.
- **Action Steps:**
  1. Walk forward down the grand gallery parquet floor toward the Great Mirror.
  2. Explain the planar reflection pipeline: hardware user clip planes, zero specular blowout, antique soda-lime glass tint, and glancing Fresnel rim.
  3. Observe that inside the mirror reflection, 3 radiant violet spectral guides hover above the pedestals indicating target angles:
     - Left Bust #1 target: **$180^\circ$ (South)**.
     - Center Bust #2 target: **$90^\circ$ (East)**.
     - Right Bust #3 target: **$270^\circ$ (West)**.
  4. Press **[E]** on real Bust #1 twice ($180^\circ$).
  5. Press **[E]** on real Bust #2 once ($90^\circ$).
  6. Press **[E]** on real Bust #3 three times ($270^\circ$).
  7. The mirror chime rings, the grand exit portal unlocks, and the **VICTORY BANNER** appears!

### Conclusion & Q&A (4:45 - 5:00)
- Summarize graphics milestones: 100% Core Profile, 0 external engine dependencies, sub-6ms frame times.

---

## 4. Cheat Sheet & Emergency Solutions

| Room | Puzzle | Exact Solution | Emergency Bypass |
| :--- | :--- | :--- | :--- |
| **Room 0** | Shadow Totems | Totem 1: 1 click ($90^\circ$)<br>Totem 2: 2 clicks ($180^\circ$)<br>Totem 3: 3 clicks ($270^\circ$) | Press **[F4]** to auto-solve |
| **Room 1** | Fuses & UV PIN | Fuses on trolley & sink $\rightarrow$ Breaker Box.<br>Door PIN: **4108** | Press **[F4]** to auto-solve |
| **Room 2** | Mirror Busts | Bust 1: 2 clicks ($180^\circ$)<br>Bust 2: 1 click ($90^\circ$)<br>Bust 3: 3 clicks ($270^\circ$) | Press **[F4]** to auto-solve |

---

## 5. Live Troubleshooting Tips

- **If the presentation runs short on time:** Press **[F1]**, **[F2]**, or **[F3]** to instantly jump between rooms without walking through transitions.
- **If an interaction is misclicked:** Press **[F4]** to immediately align all puzzle states and unlock the exit.
- **If running on an ultra-low-power machine:** Open ImGui with **[Tab]**, navigate to Settings, and select the **Low** preset (disables SSAO and Bloom for 200+ FPS).
