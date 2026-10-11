# Vita changes to upstream files (maintenance ledger)

This ledger must be updated in each PR that changes a file owned by official CorsixTH. Add a row for each integration point once implemented.

| Upstream file | Added Vita behavior | PR / issue | Expected conflict surface | Test |
|---|---|---|---|---|
| `CMakeLists.txt` | `VITA` option disables M2.1 unsupported features and enables the existing static Lua preload; desktop defaults remain unchanged | #6 | Root options and SDL major transition | Vita configure passed; desktop configure blocked by missing host C/C++ compiler |
| `CorsixTH/CMakeLists.txt` | Selects `vita/cmake/VitaEngine.cmake` for Vita dependencies and package; preserves existing desktop dependency path | #6 | SDL2/3, mixer, static Lua modules | Approved Vita-only Option B adapter links the engine and produces an audited VPK; device not run |
| `CorsixTH/CMakeLists.txt` | Compile `app0:/CorsixTH.lua` as the Vita interpreter location while retaining the install data directory | #7 | Interpreter path and generated `config.h` | Generated path and 361-entry VPK audit pass; original device run found the script |
| `CorsixTH/Src/main.cpp` | Vita uses an `fopen` interpreter probe and logs script selection/Lua panic; desktop `std::filesystem` remains | #7 | Interpreter search and Lua error handling | Vita compile/link pass; original device run found the script |
| `CorsixTH/Src/CMakeLists.txt` | Planned: add Vita source module | Not implemented | source list revisions | Link |
| `CorsixTH/SrcUnshared/main.cpp` | Vita early checked path/log initialization, boot-phase and startup exception diagnostics, ordinary log close | #7 | Main boot order and Lua/SDL error handling | Original device run wrote boot log and captured Lua error |
| `CorsixTH/SrcUnshared/main.cpp` | On Vita, exit with status 1 after a failed protected Lua startup and its error reporter returns; clean up Lua, mixer, and SDL rather than entering the normal main loop | #7 / PR #21 follow-up | Error reporter and main-loop control flow | Full Vita rebuild; follow-up device test pending |
| `CorsixTH/Lua/app.lua` | Treat absent `USER` and `USERNAME` as an empty player name so the existing `PLAYER` fallback executes; behavior applies on all platforms | #7 / PR #21 follow-up | `App:fixConfig()` player-name default | Five LuaJIT 2.1 regression cases; follow-up device test pending |
| `CorsixTH/Src/th_gfx_sdl.cpp` | Vita creates a shown native SDL window without OpenGL/resizable flags, logs renderer capabilities, rejects missing target-texture support, and records ABGR8888 target failures; desktop window flags remain | #8 / M2.3 | SDL2 window flags, renderer creation and target texture lifecycle | Full Vita build and package audit; integrated engine device test pending |
