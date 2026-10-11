# Vita changes to upstream files (maintenance ledger)

This ledger must be updated in each PR that changes a file owned by official CorsixTH. Add a row for each integration point once implemented.

| Upstream file | Added Vita behavior | PR / issue | Expected conflict surface | Test |
|---|---|---|---|---|
| `CMakeLists.txt` | `VITA` option disables M2.1 unsupported features and enables the existing static Lua preload; desktop defaults remain unchanged | #6 | Root options and SDL major transition | Vita configure passed; desktop configure blocked by missing host C/C++ compiler |
| `CorsixTH/CMakeLists.txt` | Selects `vita/cmake/VitaEngine.cmake` for Vita dependencies and package; preserves existing desktop dependency path | #6 | SDL2/3, mixer, static Lua modules | Approved Vita-only Option B adapter links the engine and produces an audited VPK; device not run |
| `CorsixTH/CMakeLists.txt` | Compile `app0:/CorsixTH.lua` as the Vita interpreter location while retaining the install data directory | #7 | Interpreter path and generated `config.h` | Generated path and 361-entry VPK audit pass; device not run |
| `CorsixTH/Src/main.cpp` | Vita uses an `fopen` interpreter probe and logs script selection/Lua panic; desktop `std::filesystem` remains | #7 | Interpreter search and Lua error handling | Vita compile/link pass; device lookup not run |
| `CorsixTH/Src/CMakeLists.txt` | Planned: add Vita source module | Not implemented | source list revisions | Link |
| `CorsixTH/SrcUnshared/main.cpp` | Vita early checked path/log initialization, boot-phase and startup exception diagnostics, ordinary log close | #7 | Main boot order and Lua/SDL error handling | Vita compile/link pass; device log not run |
| `CorsixTH/Src/th_gfx_sdl.cpp` | Planned: native Vita SDL renderer flags | Not implemented | renderer creation APIs | GXM target test |
