# Vita changes to upstream files (maintenance ledger)

This ledger must be updated in each PR that changes a file owned by official CorsixTH. Add a row for each integration point once implemented.

| Upstream file | Added Vita behavior | PR / issue | Expected conflict surface | Test |
|---|---|---|---|---|
| `CMakeLists.txt` | `VITA` option disables M2.1 unsupported features and enables the existing static Lua preload; desktop defaults remain unchanged | #6 | Root options and SDL major transition | Vita configure passed; desktop configure blocked by missing host C/C++ compiler |
| `CorsixTH/CMakeLists.txt` | Selects `vita/cmake/VitaEngine.cmake` for Vita dependencies and package; preserves existing desktop dependency path | #6 | SDL2/3, mixer, static Lua modules | Approved Vita-only Option B adapter links the engine and produces an audited VPK; device not run |
| `CorsixTH/Src/CMakeLists.txt` | Planned: add Vita source module | Not implemented | source list revisions | Link |
| `CorsixTH/SrcUnshared/main.cpp` | Planned: early platform initialization | Not implemented | boot order | Vita startup log |
| `CorsixTH/Src/th_gfx_sdl.cpp` | Planned: native Vita SDL renderer flags | Not implemented | renderer creation APIs | GXM target test |
