# Platform architecture, v0.70.1

## Baseline
Use official CorsixTH v0.70.1, retaining `CorsixTH/` Lua/C++ logic and existing SDL2 abstraction. Native route: CorsixTH -> SDL2 `SDL_Renderer` -> SDL2 Vita GXM -> Vita. This is a hypothesis until the M1A hardware probe verifies all required operations.

## Controlled Vita surface
`vita/platform/`: directory creation, read-only vs writable paths, stderr/stdout logging, lifecycle. `vita/input/`: Vita controls mapped to existing CorsixTH mouse/keyboard semantics. `vita/cmake/`: dependency and packaging details. `vita/probe/`: isolated diagnostics. `third_party/`: vetted pinned LuaFileSystem/LPeg if not available as VitaSDK packages, retain source provenance and licence.

## Essential upstream integration seams
- Root `CMakeLists.txt`: add `VITA` option and feature policy without overriding platform-independent defaults.
- `CorsixTH/CMakeLists.txt`: Vita-target dependency path, static LFS/LPeg distinct from vcpkg `unofficial-*`, data files and VPK install.
- `CorsixTH/Src/CMakeLists.txt`: one explicit set of Vita sources and Lua module sources; avoid host CLI tools.
- `CorsixTH/SrcUnshared/main.cpp`: minimal early directory/logging initialization.
- `CorsixTH/Src/th_gfx_sdl.cpp`: on Vita avoid requiring `SDL_WINDOW_OPENGL` when using native SDL renderer, test `SDL_RENDERER_TARGETTEXTURE` and failure paths.
- `CorsixTH/Src/main.cpp`: current upstream has `CORSIX_TH_LINK_LUA_MODULES` static preloads; reuse them, do not duplicate historical psp2 Lua glue.
- `CorsixTH/Lua/config_finder.lua`: existing `config.path.txt` can steer user config to writable storage; test the mechanism before any Lua edits.

## Initial resource map (proposed; test against actual packaging and game-data discovery)
- `app0:/eboot.bin`, `app0:/CorsixTH.lua`, `app0:/Lua/`, `app0:/Bitmap/`, other required engine-owned assets.
- `app0:/config.path.txt` containing `ux0:data/corsixth/user`.
- `ux0:data/corsixth/user/` for config, saves, screenshots.
- `ux0:data/corsixth/logs/` for boot and probe logs.
- `ux0:data/corsixth/theme-hospital/` for separately installed owned original data. Confirm exact inner folder (for example HOSP) with CorsixTH data detection before publishing user-facing installation instructions.

## Non-goals until playable
No FFmpeg intro movies, built-in update checker, RtMidi/MIDI device, vitaGL, custom font renderer, aggressive patches to engine gameplay, VitaTV optimization, mandatory overclock. SDL2_mixer itself remains in first engine integration because upstream has unconditional includes in some files.

## Safety/security
Make write paths explicit; never write inside app0; check mkdir return values. Avoid `#define stdout` redirection, broken legacy psp2 print. Leave room for sanitized diagnostic logs. Do not commit proprietary Theme Hospital asset data.
