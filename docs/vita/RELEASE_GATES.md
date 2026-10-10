# Studio release governance and acceptance matrix

Status classes: NOT STARTED, BLOCKED, BUILD PASS, DEVICE PASS, ACCEPTED. These are distinct; build success does not attest console behavior. Every test references exact source commit, toolchain/package fingerprint, VPK SHA256, test ID and raw result.

| Gate | Evidence | Acceptance authority | Blocks |
|---|---|---|---|
| G0 Authentic upstream | Main parentage verified at v0.70.1; foundation diff only after import | Director + GPT audit | All source PRs |
| G1 Environment | Native Windows 10 SDK provenance and package inventory; separately pinned CI image | Technical lead | First VPK build |
| G2 Graphics | SDL2 GXM driver/error log; target-texture output visible on Vita FAT | Director hardware signoff | Engine graphics integration |
| G3 Lua/FS | Static LPeg/LFS and Lua ABI, ux0 read/write evidence from FAT | Director hardware signoff | Engine boot claim |
| G4 Engine link | Clean VPK and symbolic ELF from specific commit; manifest & hash; desktop regression status | Technical lead | Engine boot candidate |
| G5 Engine boot | Correct app0/ux0 resource paths, no-data behavior, title/menu photo and log | Director | Playable claim |
| G6 First playable | Room/furnishing, staff, patients, save and reload after restart | Director | Input/polish acceptance |
| G7 Beta | Input/audio/suspend, standard clock stability, third-party license/data audit and CI | Director | Release tag |

## PR requirements
One objective per issue/PR. Provide files changed, why, upstream delta rows, tested host commands/results, VPK SHA256 if present, exact Vita install/test instructions, pass/fail and rollback reference. Resolve known regressions before merging. GPT may review code but is **not an authenticated GitHub human reviewer**. A single-director project should not require a second human approval it cannot satisfy; configure main protection for PRs, no force push/deletion, and applicable working CI checks as available.

## Risk ownership
Graphics target-texture semantics (G2) and Lua/ABI/path (G3) are technical go/no-go gates. Native Windows packaging compatibility remains unproven until G1/G4. SDL3 transition is tracked by #14 and treated as a new major integration project only when official stable upstream requires it. User-supplied commercial Theme Hospital assets never enter git/VPK/CI artifacts.
