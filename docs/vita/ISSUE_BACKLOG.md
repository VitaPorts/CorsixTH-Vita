# Suggested GitHub issue backlog (dependency-ordered, not calendar promises)

Issue order follows implementation gates. GitHub issue numbers are whatever GitHub assigns. This file is the original issue specification; live GitHub issues are the status authority. As of 2026-10-02, #1 and #3 closed, #2 in progress; blocked work should not be started until prerequisite evidence is linked.

## 1. P0 · M0.0: Import official v0.70.1 with intact Git ancestry

Blocker for every source commit. Populate the currently empty independent repository using the upstream Git remote and `v0.70.1` release tag. Verify peeled SHA `56bd5d00f76331c7f76d7b696726a7926303ca0c`. Do not create an orphan initial commit, upload a source ZIP as root, merge legacy work wholesale or force push. Evidence: `git rev-parse HEAD`, `git remote -v`, `git merge-base --is-ancestor <sha> HEAD`, GitHub main branch URL. Owner: director/local terminal. See `docs/vita/UPSTREAM_SYNC.md` in the M0 overlay.

## 2. P0 · M0.1: Review and apply foundation documentation overlay

Blocked by M0.0. Add AGENTS.md, architecture, legacy audit, upstream sync and delta ledger, decisions, dependencies, building, hardware protocol, PR/issue templates. Preserve existing upstream README/.gitignore, append `.gitignore.m0-additions`, rename README-M0.md to docs/vita/README.md if appropriate. Check relative links and that no original commercial game data are present. Deliver reviewed foundation commit/PR.

## 3. P0 · M0.2: Verify pinned VitaSDK and record dependency inventory

Windows 10 native VitaSDK via official `vdpm` PowerShell bootstrap is the director host route; verify compiler/CMake, SDL2, mixer, LuaJIT, FreeType, PNG, zlib actual package versions and link availability. Linux CI is independent and should pin the dated SDK image digest, checking target ABI parity. Document exact commands and missing packages. Environment inspection can start before #1; commits to the repo wait for #1. This is a build-environment report, not a Vita runtime PASS.

## 4. P0 · M1A: Build standalone native SDL2/GXM diagnostic VPK

Blocked by M0.1 and M0.2. Independent `vita/probe/` SDL2 C++17 test only. 960×544 window without OPENGL flag; log renderer name/flags; exercise target texture and `SDL_SetRenderTarget` visually; write `ux0:data/corsixth/probe.log`; generate ELF/VPK using distinct development title ID. Done after CI build **and** director photograph plus logfile from Vita FAT. No Lua, FFmpeg or game logic. See CODEX_HANDOFF_M1A.md.

## 5. P0 · M1B: Prove Lua ABI, LPeg/LFS and Vita filesystem runtime

After M1A native graphics evidence (dependency source review can start earlier). Minimal separate probe: Lua state, Lua/JIT version, `lfs`/`lpeg` static load with Lua 5.1 ABI, `ux0:` read/write and C++17 filesystem/stat fallback evidence. Pin upstream sources and third-party licenses. Hardware proof is `probe.log` and pass/fail table. Do not copy old `psp2_lua_libs` or output macros.

## 6. P1 · M2.1: Minimal Vita CMake/packaging integration for CorsixTH 0.70.1

Blocked by M1A/M1B. Scope root CMake, CorsixTH target CMake and vita/cmake only. Reuse actual SDK targets, static Lua module build path, disable movies/update/MIDI/host tools and host-only doc generation. Produce symbolic ELF plus VPK. Preserve desktop configure/build; log exact linker dependency failures. No giant inherited 2023 link list.

## 7. P1 · M2.2: Vita platform startup, paths and logging

Blocked by M1B, coordinate with M2.1 to avoid overlapping source changes. Add early init in SrcUnshared/main.cpp; create ux0 subdirectories safely, stdout/stderr to files, source/asset discovery under app0, `config.path.txt` writable user path, explicit error handling. Do not redefine stdout globally. Update UPSTREAM_DELTA.md.

## 8. P1 · M2.3: Adopt native SDL2 renderer in upstream gfx seam

Blocked by M1A and coordinate M2.1. Minimal Vita window-flag path in `th_gfx_sdl.cpp` to avoid mandatory SDL_WINDOW_OPENGL; preserve host behavior; guard renderer creation error paths; confirm render target assumptions. Compile + visual smoke test on Vita. Update UPSTREAM_DELTA.md. vitaGL requires separate ADR and hardware reproduction.

## 9. P1 · M3: First full engine boot and VPK resource manifest

Blocked by M2.1/2.2/2.3. Package CorsixTH.lua, Lua/Bitmap and other engine-owned resources in app0; keep commercial Theme Hospital files external. No-data run must fail gracefully. With director-provided owned data under tested ux0 subfolder, reach title screen. Deliver VPK SHA256, sanitized boot log, photograph, full VPK resource manifest.

## 10. P2 · M4: One hospital playable and save/reload

Blocked by M3. Start new game; move camera via touch or temporary pointer; build/furnish room; hire staff; run patient simulation; save, exit and load. Treat each as observed on Vita FAT. No controller polish until actual game loop operates.

## 11. P2 · M5: Vita physical controls and touch adapter

Blocked by M4. Implement isolated Vita input module mapping analog cursor, Cross primary action, Circle secondary, camera/pause and direct front touch. Drag/drop, velocity/deadzone, touch event duplication and OSK; avoid scattered Vita calls in gameplay. Director verifies room build, UI menus, pause and save by physical controls. VitaTV remains optional follow-up.

## 12. P2 · M5.2: Audio, lifecycle and safe suspend/resume

Blocked by M4, may progress parallel to controls on separate files. Verify SDL2_mixer initialization/audio in game; test background music where supported; record suspend and resume behavior, persistence, clean shutdown and save safety. No new plugins without documented measured need.

## 13. P3 · M6: Performance and stability test protocol

Blocked by M4 plus controls/audio. Record FPS/frame time, memory, graphics texture/resource metrics in early and crowded hospital; benchmark standard clocks. Run ≥30min, 10 save-load cycles, 10 suspend cycles, collect log and results. Optimizations require profiler evidence and rollback path.

## 14. P0 · MAINT-001: Adopt upstream intake policy in M0; track SDL3 risk

Foundation policy blocked by #1/#2, reviewed in M0. For each actual stable upstream upgrade, open a new separate issue: compare stable tags; record upstream dependency/SDL and save-format changes; open `upgrade/vX` from current main, merge new official release with ancestry, review conflicts and UPSTREAM_DELTA; test desktop, Vita build and hardware before merging. For 0.71 SDL3, independently verify VitaSDK SDL3 renderer+SDL3_mixer support; do not migrate active stable main prematurely. Pin release tag and SDK digest.

## 15. P3 · M7: Release compliance and director-approved beta

Blocked by the actual M3-M6 hardware gates and early CI #16. Confirm third-party notices, source release, VPK contents excluding original commercial Theme Hospital assets, original game data install instructions, known issues, standard-clock Vita FAT test report and source/SDK/artifact hashes. Only director-approved working commits may be tagged.

## 16. P1 · INFRA-001: Establish CI with first compilable target

Coordinate with #4 for a standalone probe configure/build/VPK artifact pipeline, then expand with #6 to the main engine build, host regression smoke, package manifest and SHA256. Pin SDK image by immutable digest. Activate main required status checks only after jobs exist and pass. A CI PASS is not a physical Vita HARDWARE PASS.
