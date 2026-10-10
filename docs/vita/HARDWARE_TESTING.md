# Vita FAT hardware test procedure and evidence

## Preconditions
- Authorized development homebrew console and VitaShell; FTP or USB works.
- No commercial Theme Hospital data is required for M1A/M1B.
- Record hardware/firmware, installed relevant runtime plugins, clock settings and any unusual environment choices. Do not change kernel plugins for one failed run without documenting reason and backing up current config.

## Required build/test identification
Every build supplies: Git commit, upstream engine tag, pinned SDK image+digest, output ELF/VPK SHA256, target/title ID, expected log path, test case IDs and known issues. Distinguish *compile PASS*, *hardware PASS* and *not run*.

## M1A native SDL2/GXM test (first VPK)
1. Receive VPK built from documented commit with non-production title ID.
2. Copy over FTP/USB and install through VitaShell. Confirm it launches.
3. Expected: 960 x 544 diagnostics scene, solid clear color plus identifiable shape drawn through a target texture; test verifies off-screen target then final display and logs the renderer driver/flags.
4. Collect photo and `ux0:data/corsixth/probe.log`. If crash/blank display: record whether Vita returned to LiveArea, froze or showed an error code.
5. Pass only when both logs AND observed expected frame match. `SDL_RENDERER_TARGETTEXTURE` flag alone is insufficient.

## M1B Lua + filesystem test
- Record C++17 filesystem operations (`ux0` path creation, read/write, expected contents), Lua runtime version, `lfs`/`lpeg` preload and sample expression results. Isolate `LuaJIT` JIT enablement from mere Lua state creation.

## M3 boot and M4 first hospital
- First no-data run must fail gracefully with actionable installation message.
- With user-owned original Theme Hospital data installed outside app0, reach title, load first map, build room and hire staff, save, close, relaunch and load. Record warnings and screenshot.

## Results template
```text
Test ID:
Date/time and tester:
Hardware model / firmware:
Installed plugin/runtimes relevant to test:
Clock mode:
Build commit:
VPK SHA256:
VitaSDK image digest:
Expected result:
Observed result:
PASS / FAIL / BLOCKED / NOT RUN:
Log file attached:
Photo/video attached:
Crash code / reproduction steps:
Next action:
```

## Log schema requested from Codex
`ux0:data/corsixth/probe.log` for probes; `ux0:data/corsixth/logs/vita.log` for full game. Header contains commit/tag/SDK, then `[PLATFORM]` filesystem/SDL/renderer, `[LUA]`, `[ENGINE]`, `[ERRORS]`. Timestamp and flush after each major phase, so premature crashes still yield usable evidence. Do not log user content or commercial assets.
