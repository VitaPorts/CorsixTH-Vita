# Project plan and gates

## Mission
Ship a reproducible, maintainable native CorsixTH port to PS Vita. Preserve direct Git ancestry with official CorsixTH and a clean upgrade path. Initial baseline: official v0.70.1 at peeled commit `56bd5d00f76331c7f76d7b696726a7926303ca0c` (verify locally with `git rev-parse 'v0.70.1^{}'`). Console validation owner: project director with Vita FAT, VitaShell and log access. Implementer: Codex under issue-specific PRs. Technical lead/reviewer: GPT, supplemented by real build and hardware evidence.

## Blocking priority and exit gates
| Milestone | Implementation | Exit evidence | Owner |
|---|---|---|---|
| M0.0 | Populate independent repo from official v0.70.1 Git commit, preserving ancestry | `git merge-base --is-ancestor 56bd5d0 HEAD`; `git remote -v`; current branch `main` | Director/local terminal |
| M0.1 | Commit reviewed docs & tracking templates | One foundation PR/commit; no engine changes | GPT / director |
| M0.2 | Inventory pinned VitaSDK deps and desktop smoke baseline | digest, `cmake`/compiler and package versions; desktop build result or an explicit environment blocker | Implementer |
| M1A | Reduced SDL2 GXM probe (no CorsixTH or Lua) | installable VPK; `SDL_RendererInfo`; target texture demonstrated on hardware | Codex + Vita |
| M1B | Lua/JIT + LPeg/LFS + C++17 file read/write capability | logged module versions and ux0 write/read results | Codex + Vita |
| M2 | Engine cross compilation and VPK generation | CI link and package success, exact commit and artifact SHA256 | Codex |
| M3 | Boot to title screen, paths, external game data | photo, complete boot log and no missing packaged resources | Codex + Vita |
| M4 | First hospital playable, save/reload | physical run and returned save test summary | Codex + Vita |
| M5 | Controller input, audio, suspend/resume | hardware input coverage, audio test, safe suspend cycle | Codex + Vita |
| M6 | Performance/stability | 30 min session, 10 save/load and suspend cycles, baseline performance numbers | Codex + Vita |
| Infrastructure (starts at M1A) | Initial CI on probe, expands for engine at M2; keep hardware gate separate | Actual repeatable Actions logs, pinned SDK digest, VPK hash | Technical lead/Codex |
| M7 | Beta | reviewed licences, installation instructions, external-data packaging audit, hardware release certificate | Director |

## Working agreements
- Scope PRs to one milestone or subtest. No merging code on architecture rationale alone.
- Fixed source baseline / fixed toolchain; pin dependency sources and licenses.
- Track unverified assumptions explicitly. Green CI is *not* a green Vita hardware test.
- Separate versioning: `vita-v0.70.1-alpha.1` etc; upstream tags are never rewritten.
- Desktop regressions block merges, subject to documented test-environment availability.
- Keep stable release branches for SDL-major migrations; do not prematurely port upstream `master` while SDL3 is in transition.

## Dependency edges
M0 -> M1A -> M1B -> M2 -> M3 -> M4; M5 can branch after M4; M6 requires M4+M5; M7 requires all. M1B can perform initial CMake/library archaeology while M1A hardware is under test but must not declare gate success until measured.

## Release governance
- #1 initializes authentic Git history once. `main` is a baseline until the first hardware-proven Vita build exists; thereafter merge changes by narrow PR with actual test results.
- #14 upstream-sync policy is an M0 prerequisite; future upstream releases use fresh `upgrade/` issues/PRs.
- #16 starts CI with the first compilable probe, **not** in M7; require passing status checks once jobs exist.
- Human director approves hardware-test results and release candidates. GPT review is advisory; do not falsely report it as an authenticated human GitHub review.
- Each milestone has one test artifact (commit SHA, SDK inventory/digest, VPK hash, test-case ID, observed pass/fail), plus a rollback reference to the last known-good build.
