# CorsixTH-Vita contributor / Codex contract

## Goal and provenance
- Maintained native PS Vita port of official CorsixTH; start from **v0.70.1** (peeled release tag commit `56bd5d00f76331c7f76d7b696726a7926303ca0c`).
- `origin`: `https://github.com/VitaPorts/CorsixTH-Vita.git`; `upstream`: `https://github.com/CorsixTH/CorsixTH.git`; read-only `legacy`: `https://github.com/MrProcastinator/CorsixTH-vita.git`, branch `feature/vita_port`, head `ed5f8408ee7192afb19b60560b5ad34448470a5e`.
- Retain official Git ancestry. Never replace the upstream source with the legacy ZIP, squash official history or merge the old branch wholesale.

## Engineering boundaries
1. Prioritize reproducible evidence over optimistic claims. A VPK built in CI is not a hardware pass.
2. Each change needs a numbered issue, narrow branch, PR, build output, test instructions and a reviewer.
3. Keep Vita-only implementation under `vita/` where practical. Any edit to upstream engine files requires a short justification and an entry in `docs/vita/UPSTREAM_DELTA.md`.
4. Keep desktop behavior and supported game data paths intact. Do not rewrite gameplay merely to simplify Vita bring-up.
5. Use native SDL2 GXM first on 0.70.1; test SDL render-target textures. Do not introduce vitaGL without a failing reduced reproduction and design review.
6. Use existing `CORSIX_TH_LINK_LUA_MODULES` preload for LuaFileSystem and LPeg; the Vita CMake path must bypass vcpkg-only find_package branches.
7. Disable movies, update checks, MIDI device, Tracy and host-only tools for first boot. Do not disable ordinary audio compilation without examining unconditional SDL_mixer includes.
8. Never bundle or commit original commercial Theme Hospital game data. No game data in logs or Actions artifacts.
9. Pin and record the native Windows VitaSDK release/channel, compiler and package inventory. Independent CI may use a dated image digest; verify target SDK/library compatibility rather than assuming parity.
10. Treat source/user paths separately: read-only app resources under `app0:` and writable data under `ux0:data/corsixth`.

## Required PR report
- Objective + linked issue and upstream baseline.
- Exact changed files, reasoning, third-party provenance and new dependencies.
- Reproduction commands, compile results, warnings and binary/artifact hashes.
- Machine-readable acceptance checks; tests *actually executed* vs hardware-only/unverified.
- Potential upstream rebase conflict surfaces and update to `UPSTREAM_DELTA.md` where needed.
- If a test fails, keep the exact failure. No unverified "works on Vita" claims.

## Order of work
M0 import upstream history -> M1A native SDL2/GXM probe -> M1B Lua/filesystem probe -> M2 engine build -> M3 boot -> M4 gameplay -> M5 controls/audio -> M6 stability -> M7 beta. Do not bypass gates without documenting a risk acceptance.
