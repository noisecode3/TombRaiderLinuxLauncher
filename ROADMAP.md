# Roadmap

**TombRaiderLinuxLauncher** is currently in a **prototype** stage and not yet intended for public release.
This roadmap outlines the steps toward the **0.0.1 initial release**.

---

## ✅ Core Features (Pre-0.0.1)

### Basic Functionality

- Allow deletion of a level (without removing save files), or just its ZIP archive.
- Update button states appropriately based on context.
- Implement basic keyboard navigation and shortcuts.
- Add support for launch flags (e.g. filters and sorting options).
- Implement core launching functionality for TRLE levels.

### Code Style & Linting

- Add clear, concise comments (including hover tooltips and Doxygen HTML docs).
- Ensure code passes Codacy and MISRA C++ checks without warnings.
- Maintain a consistent coding style throughout the codebase.
- Simplify and clean up complex or redundant code.

### Testing & Debugging

- Add unit tests for:
  - `GameFileTree`
  - `Path`
  - `PyRunner`
- Remove or clean up debug output/logging.
- Perform thorough testing and bug fixing.

---

## 🧪 After Initial Release

### Integration with trcustoms.org

- Fetch metadata using JSON and `libcurl`.
- Use perceptual image hashing (e.g. via `CImg`) and fuzzy string matching
  ([rapidfuzz-cpp](https://github.com/rapidfuzz/rapidfuzz-cpp))
