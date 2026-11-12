# Phase 2 Findings & Technical Insights

## Platform & Hardware Constraints
- **GPU**: Intel(R) UHD Graphics (Driver Build 30.0.101.1273)
- **API**: OpenGL 3.3 Core Profile, GLSL 3.30.
- **Constraints**:
  - No compute shaders (`GL_COMPUTE_SHADER` is OpenGL 4.3+).
  - Bandwidth is shared with system RAM; 1280x720 target requires minimal high-resolution blurs and careful FBO allocation.
  - Geometry shader cubemap rendering (`gl_Layer`) is notoriously slow on Intel integrated graphics; rendering 6 passes using lightweight vertex/fragment shaders and switching FBO cube face attachments is faster and more reliable.

## Shadow Mapping Findings
- **Static Depth Cubemap**:
  - Point lights in the escape room are ceiling-mounted and static relative to room geometry.
  - Rendering 6 faces of 512x512 every frame costs ~2.5 - 3.5 ms on Intel UHD.
  - By rendering the static geometry ONCE to the cubemap and caching it, and only re-rendering when dynamic items move (`isHierarchyMoving()` dirty flag), per-frame GPU cost for point light shadows drops to **0.0 ms render overhead** during exploration!
- **Bias Strategy**:
  - Normal offset bias (`FragPos + normal * normalBias`) combined with slope-scaled depth bias completely removes acne on flat walls and floors while preventing shadow detachment on furniture feet.
