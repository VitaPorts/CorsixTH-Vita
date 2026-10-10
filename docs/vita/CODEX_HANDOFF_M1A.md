# Codex execution brief: M1A reduced GXM diagnostic VPK

## Preconditions
Repo `main` must descend from upstream `v0.70.1` peeled SHA, M0 docs merged. Use a separate branch `codex/m1a-gxm-probe`. Native Windows VitaSDK installation from issue #3 must be version-pinned and its actual compiler/package inventory recorded. If building in CI, pin the Linux image by digest and verify target package parity. Provide a minimal CMake target inside `vita/probe/` without editing gameplay or engine CMake. If top-level integration would create unnecessary conflicts, compile probe separately using `cmake -S vita/probe -B build-vita-probe` and VitaSDK toolchain.

## Required implementation
- Standalone C++17 SDL2 Vita application with 960x544 window created without `SDL_WINDOW_OPENGL`.
- SDL renderer name and flags logged explicitly; report actual SDL errors on failure.
- Create RGBA target texture `SDL_TEXTUREACCESS_TARGET`; draw deterministic off-screen scene; set render target back to default; composite to screen. Display distinct visible output, retain a screenshot reference in task description.
- On startup create `ux0:data/corsixth`; write and flush `probe.log` per phase. Failed mkdir/fopen must not cause null `FILE*` crashes; fall back to display if logger unavailable.
- Optional: log C++17 path operations, but do not pull in Lua or SDL2_mixer here.
- Produce ELF with symbols and installable VPK with independent test title ID; retain build manifest, exact toolchain image digest, VPK SHA256 and command transcript.

## Acceptance gate
- CI/host: compile/link/package SUCCESS with actual commands + output.
- Director/Vita: install VPK, observe expected image, return `probe.log` + photo. SDL renderer info must include TARGETTEXTURE *and* offscreen render effect must be visible.
- No claim of hardware PASS until received from director.

## Explicit exclusions
Do not implement game port, full Lua integration, custom Vita OpenGL, FFmpeg, generic custom renderer or kernel-plugin dependency changes. Keep M1B independent.

Detailed screen, button and pass/fail contract: [M1A_PROBE_UI_SPEC.md](M1A_PROBE_UI_SPEC.md).
