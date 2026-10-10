# M1A GXM probe: visible interaction and acceptance spec (Issue #4)

This is a **standalone diagnostic VPK**, not CorsixTH. The engine/game menu, Lua, SDL_mixer, movies, original Theme Hospital assets, custom LiveArea artwork, VitaGL and mandatory extra kernel plugins are out of scope.

## Visual design (960 x 544, native SDL2 Vita GXM)

One deterministic full-screen test panel without third-party font packages. Draw short labels using a minimal embedded bitmap font or render unmistakable status/color cues with SDL geometry; never turn font acquisition into a blocker.

- Header: `CORSIXTH-VITA / M1A GXM PROBE` and an identification/build string.
- Left diagnostic strip: SDL initialization, window, renderer name/flags, target texture creation, render-to-texture, composite, logfile. Each item displays `PASS`, `FAIL` or `UNTESTED`. Actual success/failure must also be written and flushed to log.
- Direct backbuffer reference: blue bar near the bottom of screen.
- Main evidence: large off-screen target texture composed onto backbuffer containing a vivid magenta/teal background, a red filled rectangle and two yellow diagonal crossing lines. Its presence demonstrates actual target-texture output rather than reliance on a renderer capability bit. Place contrasting native backbuffer shapes around the textured panel to make composition obvious.
- Footer: `CROSS: RERUN  |  START: EXIT` (or immediately visible symbolic button labels where embedded font is limited). No touch/controller adapter, audio menus or files from the original game.

## Deterministic test order
1. Create `ux0:data/corsixth` safely and open/flush `ux0:data/corsixth/probe.log` from early startup; if fopen fails, continue rendering an explicit failure signal without dereferencing null.
2. Log build commit, compiler/SDK, SDL compile/runtime versions, build mode, start timestamp if available. Initialize SDL video/events, 960x544 window without `SDL_WINDOW_OPENGL`.
3. Create renderer with native Vita backend; log actual `SDL_GetRendererInfo.name`, flags, `SDL_RENDERER_TARGETTEXTURE`, max width/height and any SDL error. Reject silent software fallback as evidence of *GXM* success.
4. Draw a blue direct-to-backbuffer reference. Create a RGBA8888 `SDL_TEXTUREACCESS_TARGET` texture (e.g. 320x240); set it as target; clear with distinct color, draw red rectangle and yellow X, reset target to NULL; copy onto backbuffer and present. Validate every API result and log pass/failure independently.
5. Display output until START exits; CROSS safely repeats diagnostic drawing and increments a run count; maintain stable event loop/cleanup. No forced automatic shutdown.
6. Always flush log at each stage; write final diagnostic summary and close on clean exit. On graphics failure write error, remain alive where possible so Director can photograph/retrieve log.

## Packaging/artifacts
- Separate C++17 CMake target under `vita/probe/`, configure via VitaSDK toolchain without modifying upstream gameplay or existing root CMake.
- Development title ID candidate `CORSXP001` (validate actual Vita packaging restrictions), title `CorsixTH Vita GXM Probe`; eboot.bin within an installable VPK. Keep an unstripped symbol-bearing ELF outside VPK.
- Provide VPK SHA256, Git SHA, toolchain/package inventory and complete configure/build output. No proprietary game data in VPK, repository or diagnostic artifacts.

## Human acceptance test
Director installs VPK with VitaShell on FAT, launches without original game assets, photographs screen, presses CROSS (visual repaint and run count), exits via START, retrieves `ux0:data/corsixth/probe.log` by FTP/USB. Include actual device/firmware details when convenient.
BUILD PASS means link/package succeeded. DEVICE PASS requires native renderer identified, target-texture flag logged **and** distinct off-screen texture actually visible on console. Record failure accurately and keep reduced reproduction for investigation.
