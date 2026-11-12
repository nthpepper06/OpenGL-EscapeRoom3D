# Phase 1: Physically Based Rendering (PBR) & Linear HDR Pipeline

## Work Plan & Status Tracking

### Sub-Tasks
- [x] **1. Task Planning & Environment Verification**
  - [x] Create `task.md` work plan
  - [x] Confirm baseline build and running condition
- [x] **2. Linear HDR Pipeline & ACES Tone Mapping**
  - [x] Upgrade `PostProcessor` FBO color attachment to `GL_RGBA16F` (Float16 HDR buffer)
  - [x] Add `exposure` parameter in `PostProcessor` (default 1.0f)
  - [x] Upgrade `post_process.frag`: ACES Filmic Tone Mapping curve
  - [x] Implement linear to sRGB output gamma correction in `post_process.frag`
  - [x] Enable `GL_TEXTURE_MIN_FILTER`, `GL_LINEAR` with proper clamping
  - [x] Expose `exposure` and `toneMappingEnabled` controls in `GuiLayer` (Dear ImGui)
- [x] **3. Cook-Torrance PBR Shaders (`assets/shaders/pbr.vert`, `assets/shaders/pbr.frag`)**
  - [x] Create `pbr.vert` with world position, TBN frame (tangent, bitangent, normal), and UVs
  - [x] Create `pbr.frag`:
    - [x] Cook-Torrance Specular BRDF: D (Trowbridge-Reitz GGX), G (Smith Schlick-GGX), F (Fresnel-Schlick)
    - [x] Diffuse term: Lambertian diffuse with energy conservation $(1 - F) \times (1 - \text{metallic})$
    - [x] Point Light calculation with physical quadratic falloff + room flicker
    - [x] SpotLight calculation with angular penumbra + physical attenuation
    - [x] Ambient / IBL integration (diffuse irradiance + prefiltered specular reflection)
    - [x] Emissive contribution with color and factor
    - [x] Selection and Hover highlights
    - [x] Strict Material Fallbacks (prevent black screen / invisible objects if maps are missing)
- [x] **4. PBR Texture Sets & Procedural Material Upgrade**
  - [x] Upgrade `Material` struct:
    - `albedoMap`, `normalMap`, `metallicMap`, `roughnessMap`, `aoMap`, `emissiveMap`
    - Scalar parameters: `albedoColor`, `metallic`, `roughness`, `ao`, `emissiveColor`, `emissiveIntensity`
    - Feature flags: `useAlbedoMap`, `useNormalMap`, `useRoughnessMap`, `useMetallicMap`, `useAoMap`, `useEmissiveMap`
  - [x] Upgrade `Texture` class:
    - Anisotropic filtering setup (`GL_TEXTURE_MAX_ANISOTROPY_EXT` / mipmaps)
    - Safe default 1x1 fallback textures: `getWhiteTexture()`, `getFlatNormalTexture()`, `getDefaultRoughnessTexture()`, `getDefaultMetallicTexture()`, `getDefaultAoTexture()`, `getBlackTexture()`
    - Procedural PBR map generators (Albedo, Normal, Roughness, Metalness, AO, Emissive) for Stone, Wood, Metal, Gold, Asylum Tile, Ritual Pentagram, and Mirror
- [x] **5. Geometry Tangents & Optimized Normal Matrix**
  - [x] Add `tangent` and `bitangent` to `Vertex` struct in `Mesh.hpp`
  - [x] Automatic Tangent and Bitangent calculation with Gram-Schmidt orthogonalization in `Mesh::setupMesh()`
  - [x] Optimize `normalMatrix` caching in `SceneObject` with dirty flag tracking (avoids re-inverting unchanged matrices)
- [x] **6. Basic Image-Based Lighting (IBL) Support**
  - [x] Cook-Torrance Fresnel-Schlick with Roughness ambient approximation
  - [x] Directional room bounce specular reflection mapped across ceiling and light fill
- [x] **7. Room Integration & Verification**
  - [x] Connect PBR shader into `Application` (replacing legacy `blinn_phong`)
  - [x] Update `EscapeRoomBuilder`: Golden Key with `metallic = 1.0f`, `roughness = 0.16f`; Heavy Safe with `metallic = 0.90f - 0.98f`, `roughness = 0.18f - 0.32f`; Illuminated Exit Signs & Downlights with emissive radiance
  - [x] Update `OccultRitualRoom`, `AbandonedAsylumRoom`, `MirrorRoom` to bind high-resolution procedural PBR textures and material properties
  - [x] Compile with CMake + Ninja (`cmake --build build`), 0 warnings / 0 errors
  - [x] Smoke test `EscapeRoom.exe` with `RUN_OK` verification
