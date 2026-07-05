# Roadmap

**TombRaiderLinuxLauncher** is currently in a **prototype** stage and not yet
intended for public release.

This roadmap outlines the steps toward the **0.0.1
initial release**.

---

## Core Features (Pre-0.0.1)

### Basic Functionality

- [x] Update button states appropriately based on context.
- [x] Add support for launch flags (e.g. filters and sorting options).
- [x] Implement core launching functionality for TRLE levels.
- [x] Implement basic keyboard navigation and shortcuts.
- [ ] Allow deletion of a level without removing save files.
- [ ] 🎮 PS Controller Script:
  - [x] Classic tank
  - [x] Generic Controller mapping
  - [ ] Angular (Direction + Magnitude) State Context-Aware Left Stick
- [ ] run all the original TR core design games.

### Code Style & Linting

- [ ] Add clear, concise comments (including hover tooltips and Doxygen HTML docs).
- [ ] Ensure code passes Codacy and MISRA C++ checks without warnings.
- [ ] Maintain a consistent coding style throughout the codebase.
- [ ] Simplify and clean up complex or redundant code.

### Testing & Debugging

- [ ] Add unit tests for:
  - [ ] `GameFileTree`
  - [ ] `Path`
  - [ ] `PyRunner`
- [ ] Remove or clean up debug output/logging.
- [ ] Perform thorough testing and bug fixing.

---

## 🧪 After Initial Release

### Integration with trcustoms.org

- [ ] Fetch metadata using JSON and `libcurl`.
- [ ] Use perceptual image hashing (e.g. via `CImg`) and fuzzy string matching e.g
  ([rapidfuzz-cpp](https://github.com/rapidfuzz/rapidfuzz-cpp))

### TombDraw – DX7/DDraw → OpenGL Roadmap

Future development includes custom DirectX replacement (ddraw.dll)
designed specifically for Tomb Raider 4 TRLE, aiming for better performance
and compatibility on modern systems.

Goal: Game-specific DX7 → OpenGL backend for TRLE/Tomb4

#### Base DX7 → OpenGL

- [ ]  Implement IDirectDrawSurface7:
  - [ ] Create surface
  - [ ]  Flip / Blt / Lock / Unlock
  - [ ]  Handle lost surfaces (IsLost)
- [ ]  Implement IDirect3DDevice7:
  - [ ]  BeginScene / EndScene
  - [ ]  DrawPrimitive (store parameters in cache)
  - [ ]  SetTransform / SetTexture / SetRenderState / SetLight

#### Frame Caching

- [ ] Implement CPU-side vertex buffer cache:
  - [ ] Append vertices from DrawPrimitive calls
  - [ ] Store per-primitive state (texture, shader, material, lights)
- [ ] Implement batch lists:
  - [ ] Start batching when any state changes
  - [ ] Test: All primitives are cached correctly between BeginScene → EndScene
  - [ ] Goal: Gather all frame geometry in memory before GPU submission

#### Fixed-function → Shader

- [ ] Write vertex shader:
  - [ ] Transform: World → View → Projection
  - [ ] Lighting: Ambient + Diffuse (per vertex)
  - [ ] Fog
- [ ] Write fragment shader:
  - [ ] Texture sampling
  - [ ] Color modulation / alpha blending
  - [ ] Map DX7 FVF → OpenGL VAO / VBO inputs
