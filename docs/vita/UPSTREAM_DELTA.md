# Vita changes to upstream files (maintenance ledger)

This ledger must be updated in each PR that changes a file owned by official CorsixTH. Initial status: **no Vita modifications have been made yet**. Add a row for each integration point once implemented.

| Upstream file | Added Vita behavior | PR / issue | Expected conflict surface | Test |
|---|---|---|---|---|
| `CMakeLists.txt` | Planned: VITA option and feature policy | Not implemented | Root options and SDL major transition | Both configure modes |
| `CorsixTH/CMakeLists.txt` | Planned: Vita dependencies and packaging | Not implemented | SDL2/3, mixer, static Lua modules | Link and VPK manifest |
| `CorsixTH/Src/CMakeLists.txt` | Planned: add Vita source module | Not implemented | source list revisions | Link |
| `CorsixTH/SrcUnshared/main.cpp` | Planned: early platform initialization | Not implemented | boot order | Vita startup log |
| `CorsixTH/Src/th_gfx_sdl.cpp` | Planned: native Vita SDL renderer flags | Not implemented | renderer creation APIs | GXM target test |
