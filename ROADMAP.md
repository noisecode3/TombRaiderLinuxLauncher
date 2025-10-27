# Roadmap

**TombRaiderLinuxLauncher** is currently in a **prototype** stage and not yet intended for public release.
This roadmap outlines the steps toward the **0.0.1 initial release**.

---

## ✅ Core Features (Pre-0.0.1)

### Basic Functionality

- Allow deletion of a level (without removing save files), or just its ZIP archive.
- ✅Update button states appropriately based on context.
- Implement basic keyboard navigation and shortcuts.
- ✅Add support for launch flags (e.g. filters and sorting options).
- ✅Implement core launching functionality for TRLE levels.
- 🎮 PS Controller Script – Angular (Direction + Magnitude) State Context-Aware Left Stick
   - Make movement flowing and safer from mistakes, while staying modern and adaptable.
   - From the dead zone, use no overlapping between movement states.
   - When running or stepping back, use extreme overlapping locked in Y, but flippable within a narrow angle.
   - When running, Lara shouldn’t step back unless the stick is pulled fully downward within a narrow angle.
   - To jump forward-right, the player must press jump + pull forward + pull right — it cannot be mixed up with a right-side jump.
   - To stop, the player must return to dead zone + pull right — it cannot be mixed up with a running forward-right input.
   - With Look key activated, controls revert to classic 8-way equal-angle overlapping (traditional tank-style grid).
   - Consider using time.perf_counter() for cooldown checks or pulse-style transition turns, if it doesn’t cause Lara to stutter.
   - This design may not be final, and adjustments will be made as needed to improve left stick feel and responsiveness.
- run and manage all the original TR core design games.

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
