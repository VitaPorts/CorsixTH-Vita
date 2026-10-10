# Port status

Last updated: 2026-10-10.

## Verified foundation

| Subject | Status | Evidence / next action |
|---|---|---|
| Official engine baseline | **PASS** | `main` is official CorsixTH v0.70.1 peeled commit `56bd5d00f76331c7f76d7b696726a7926303ca0c`; Issue #1 closed. |
| Upstream ancestry | **PASS** | Original CorsixTH history retained; legacy 2023 branch remains donor/reference only. |
| Native Windows VitaSDK | **PASS (environment inventory)** | VitaSDK 2026.08, sequence 2026092555, GCC 15.2.0, CMake 4.4.3, Ninja 1.13.2; Issue #3 closed. |
| Target packages | **PASS (installed inventory)** | SDL2, SDL2_mixer, LuaJIT, FreeType, libpng, zlib and dependencies installed. Native Windows libpng alias workaround documented. |
| M0 foundation | **IN REVIEW** | `setup/m0-foundation` carries documentation/governance only; merge through Issue #2 PR. |
| Native SDL2/GXM rendering | **NOT RUN** | M1A Issue #4 is the first actual Vita application test. |
| Lua/LPeg/LFS/ux0 runtime | **NOT RUN** | M1B Issue #5 after graphics gate. |
| Full CorsixTH Vita build | **NOT IMPLEMENTED** | Begins at M2 after M1A/M1B evidence. |
| Physical Vita station | **READY** | Director can install VPKs with VitaShell and retrieve logs/photos. |

No VPK has yet earned DEVICE PASS. Cross-compilation success, when it exists, will remain separate from physical Vita acceptance.

## Immediate order

1. Merge the docs-only M0 foundation PR after review.
2. Start Issue #4 on `codex/m1a-gxm-probe`.
3. Codex produces the standalone SDL2/GXM probe and BUILD PASS evidence.
4. Director installs the exact VPK on the Vita FAT and returns `ux0:data/corsixth/probe.log` plus a photo.
5. GPT records G2 as PASS/FAIL/BLOCKED and either advances to M1B or sends a reduced correction back to Codex.

See [TEAM_WORKFLOW.md](TEAM_WORKFLOW.md) for authority and handoff rules.
