# Historical Vita branch audit

Historical donor: https://github.com/MrProcastinator/CorsixTH-vita/tree/feature/vita_port . Verified six-commit delta `ac2b13ea174a900a84b2e7e0b872577a377fcb7d..ed5f8408ee7192afb19b60560b5ad34448470a5e`, touching 23 files. Donor pre-port base is official CorsixTH commit dated 2023-02-23. Uploaded ZIP name `CorsixTH-vita-feature-vita_port.zip`, SHA256 `1e5b7befbab1caed3d348c796e088ada3d29e1ea3b011095270e7cbb17676c0e`; archive integrity check passed (685 entries). **No whole-branch merge.**

## Historical commits
| Commit | Purpose | Disposition |
|---|---|---|
| `5b59befe` | Vita CMake, manual giant dependency list, `ux0:` path, VPK references | Concept useful; rewrite against 0.70.1 and pinned VitaSDK. Referenced `vita/sce_sys/*` assets were missing. |
| `884e74eb` | File logging macros sprayed through engine and host tools | Discard; implement Vita-only early logger and ensure parent dirs exist. |
| `d5c829e9` | Static lfs/lpeg preloading | Functionality upstreamed in 0.70.1 via `CORSIX_TH_LINK_LUA_MODULES`; implement Vita static build rules only. |
| `3795d084` | Lua print redirection | Superseded; discard. |
| `e51eed82` | PSP2 debugscreen source files | Unconnected to target in historical delta; discard. |
| `ed5f8408` | Custom PSP2 Lua logging package | Discard: Lua index loop starts at 0; missing break for string case; uses obsolete package glue. |

## File-level decisions
| File(s) | Classification | Target action |
|---|---|---|
| Root `CMakeLists.txt`, `CorsixTH/CMakeLists.txt`, `CorsixTH/Src/CMakeLists.txt` | Rewrite | Minimal Vita integration modules. |
| `CorsixTH/SrcUnshared/main.cpp` | Still applies conceptually | Call one early platform initializer. |
| `CorsixTH/Src/main.cpp` | Upstreamed | Enable and satisfy existing static preloads; no psp2 duplicate. |
| `CorsixTH/CorsixTH.lua` | Obsolete legacy delta | Do not add `.suprx` lookup or monkey-patch print. |
| `psp2_output.*`, `psp2_lua_libs.*`, `psp2_debugscreen*` | Obsolete | Keep out of new target. |
| `sdl_core.cpp`, `th_map.cpp`, `SpriteEncoder/*`, `tools/rnc/rnc_decode_cli.cpp` | Obsolete Vita logging edits | Exclude historical modifications, desktop host tools disabled for Vita. |
| `CorsixTH/Src/th_gfx_sdl.cpp` | **Missing in legacy attempt** | On v0.70.1 remove mandatory `SDL_WINDOW_OPENGL` flag for Vita and verify native GXM render targets. |

## Critical modern differences
- Official v0.70.1 still uses SDL2 / SDL2_mixer, C++17; current upstream `master` migrated build dependencies/API toward SDL3/SDL3_mixer. Future upgrade needs deliberate SDL-major compatibility work.
- v0.70.1 `CorsixTH/CMakeLists.txt` static module path currently uses vcpkg `unofficial-luafilesystem` / `unofficial-lpeg`; don't enable that branch for Vita without adding a Vita-specific source-backed alternative.
- The historical branch still uses `SDL_WINDOW_OPENGL` in `th_gfx_sdl.cpp`; stock native Vita GXM should be separately probed.
