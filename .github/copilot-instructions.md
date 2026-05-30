# Dune Legacy Copilot Instructions

## Build, test, and formatting

- This repository uses **CMake presets**. Prefer the documented release presets from `README.md` / `CMakePresets.json` over ad hoc build directories:
  - Windows: `cmake --preset=windows-x64-release && cmake --build --preset=windows-x64-release && ctest --preset=windows-x64-release`
  - Linux: `cmake --preset=linux-release && cmake --build --preset=linux-release && ctest --preset=linux-release`
  - macOS CI uses `macos-arm64-release`
- The test suite is exposed through **CTest** as a small set of named top-level tests. Useful single-test commands:
  - `ctest --preset=linux-release -R '^INIFile$' --output-on-failure`
  - `ctest --preset=linux-release -R '^(FileSystem|dune_misc|lemire|random|random_serialization)$' --output-on-failure`
  - The same `-R` filters work with `windows-x64-release`
- For finer-grained GoogleTest debugging, run a test binary directly from the build tree, for example: `out/build/<preset>/tests/random/random_test[.exe] --gtest_filter=SuiteName.TestName`
- Some tests depend on copied fixture files in the binary tree, especially `INIFileTestCase`, so prefer `ctest` or direct execution from `out/build/<preset>/tests/...` instead of launching test binaries from arbitrary working directories
- Formatting is provided by the custom `clangformat` target:
  - `cmake --build --preset=linux-release --target clangformat`
  - `cmake --build --preset=windows-x64-release --target clangformat`
- Locale extraction/update is also a build target:
  - `cmake --build --preset=linux-release --target update_locale`
  - `cmake --build --preset=windows-x64-release --target update_locale`

## High-level architecture

- The build produces a large static library target, **`dune`**, that holds most game code, plus a thinner **`dunelegacy`** executable for startup, configuration, logging, and platform entry points. The build also copies `data/locale` and `data/maps` into the runtime output directory.
- `src/main.cpp` is the bootstrap layer: initialize SDL/video/audio, create the shared managers in `dune::globals`, load palettes and assets through `FileManager`, then enter the menu flow before handing off to a `Game` instance.
- The menu/UI layer is its own loop-oriented subsystem under `src/Menu` and `src/GUI`. In-game runtime work is then split across `Game*.cpp` by responsibility:
  - `Game.cpp`: setup, save/load, replay setup, and main-loop orchestration
  - `GameInput.cpp`: SDL event handling and translation from UI actions to gameplay commands
  - `GameUpdate.cpp`: deterministic simulation ticks, command execution, triggers, networking wait states, and object cleanup
  - `GameRender.cpp`: world rendering, fog/shroud, overlays, and placement previews
- Multiplayer, replay, and determinism depend on the command pipeline. Input handlers and AI enqueue `Command`s through `CommandManager`; those commands are scheduled by cycle and executed later in `GameUpdate.cpp`.
- Asset loading is centralized in `src/FileClasses`. `FileManager` searches filesystem data directories first and then falls back to PAK archives, using case-insensitive lookup. Runtime behavior depends on shipped assets plus original Dune II PAK content.
- Rendering still starts from palette-based legacy assets. `src/Renderer/DuneTextures.cpp` assembles texture atlases and generated textures, and `src/misc/draw_util.cpp` contains the SDL3 surface helpers used to preserve indexed palette data correctly.
- The codebase is organized mostly by domain (`units`, `structures`, `players`, `Renderer`, `Network`, `Menu`, `GUI`, `FileClasses`, `INIMap`, `MapEditor`) rather than by narrow feature slices, and public headers mirror that structure under `include/`.

## Key conventions

- **Preserve deterministic simulation.** Core gameplay uses `FixPoint` and `_fix` literals instead of floating-point math in simulation-sensitive paths.
- **Do not mutate simulation state directly from input code.** Follow the existing split where `handle*` / `*Click` methods interpret UI input and route it into command or gameplay action paths, while actual gameplay work is performed later through the established action methods and command execution flow.
- **Gameplay actions should continue to flow through the shared command/action surface.** `CommandManager` is part of the correctness model for multiplayer and replay, and AI code under `src/players` uses the same `Player::do*` helpers as the human-controlled path.
- **Do not keep durable raw object pointers.** Long-lived references are usually stored as object IDs or `ObjectPointer` and re-resolved through `ObjectManager`, because objects are created and removed dynamically.
- **Expect shared runtime state in `dune::globals`.** Managers, window/renderer state, current game, current map, and local player/house pointers are intentionally shared there across subsystems.
- **New user-visible strings should go through localization.** Use the `_()` macro from `globals.h` / `TextManager`, and run `update_locale` after changing UI text.
- **SDL3 indexed surfaces need palette-preserving handling.** Use `SDL_DuplicateSurface` for indexed-surface clones and keep the custom indexed-to-ARGB conversion paths when touching renderer or asset-loading code.
- **Adding a new game option is cross-cutting.** Expect coordinated updates across data types/config parsing, options UI, and `GameInitSettings` persistence rather than a single isolated change.
- **Follow the repository’s wrapping style for long argument lists.** When a call or constructor wraps across multiple lines, prefer one argument per line instead of bin-packing several arguments onto the same line.
