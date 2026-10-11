# M2.2 Vita startup, filesystem and logging — Issue #7

Initial package status (before the hardware follow-up): **LOCAL BUILD PASS / DEVICE NOT RUN**.
See the hardware follow-up section below for the observed Vita failure and the
replacement build status.
Branch `codex/m2-startup-filesystem`
starts from accepted M2.1 merge `fb48daadcac92effdc55f4c71c83631fa931fc30`;
official CorsixTH baseline remains v0.70.1
(`56bd5d00f76331c7f76d7b696726a7926303ca0c`). The VPK has not been
installed or run. A compiled package does not establish that engine startup,
SDL rendering, external game-data discovery, or gameplay works on a Vita.

## Path decision and source changes

| Vita path | Purpose |
| --- | --- |
| `app0:/CorsixTH.lua`, `app0:/Lua/`, `app0:/Bitmap/`, `app0:/Campaigns/`, `app0:/Levels/` | Packaged, read-only CorsixTH resources. |
| `app0:/config.path.txt` | Existing upstream config override, containing exactly `ux0:data/corsixth/user` and a newline. |
| `ux0:data/corsixth/user/` | Writable `config.txt`, `hotkeys.txt`, `Saves/`, `Screenshots/`, `Levels/`, `Campaigns/`, and game `Logs/`. |
| `ux0:data/corsixth/logs/boot.log` | Vita boot-phase diagnostic log, flushed after each line. |
| Director-selected `ux0:` location | Separately installed, legally owned Theme Hospital assets; no original game data in Git or VPK. |

`CorsixTH/CorsixTH.lua` derives its base directory from its source path;
`Lua/config_finder.lua` derives `app0:/` and reads `config.path.txt` there.
It trims that file and uses its nonempty contents as the user config directory.
The early Vita module creates that directory before Lua begins, avoiding the
upstream fallback to the read-only resource directory if directory creation
fails. The app's existing config-path-derived defaults then select user
Saves/Screenshots/Levels/Campaigns/Logs. No Lua game file was changed in the
initial M2.2 package.

- `vita/platform/startup.h`, `startup.cpp`: checked `ux0:` directory setup,
  early `app0:` resource and config redirect checks, errno diagnostics, and
  flushed boot logging. No `app0:` write is attempted.
- `vita/resources/config.path.txt`, `.gitattributes`: exact packaged user
  config directory with LF line ending on Windows checkouts.
- `vita/cmake/VitaEngine.cmake`: include the Vita startup source and package
  the override at the VPK root; add it to the resource manifest.
- `CorsixTH/CMakeLists.txt`: set the Vita generated interpreter path to
  `app0:/CorsixTH.lua`, preserving its install data directory and desktop path.
- `CorsixTH/Src/main.cpp`: use `fopen` for the Vita interpreter existence
  probe because VitaSDK C++ `std::filesystem::status` is not a validated
  `app0:` probe; log interpreter selection and Lua panic.
- `CorsixTH/SrcUnshared/main.cpp`: initialize before Lua, log Lua/SDL phases,
  errors and ordinary shutdown, and catch startup C++ exceptions to record
  their message and SDL's last error. Desktop code stays behind Vita guards.
- `vita/tests/audit_startup_package.py`: verify the built interpreter path,
  upstream source-path slices, config redirect, exact archive entries, bytes,
  CRCs and development title ID.
- `docs/vita/UPSTREAM_DELTA.md`: record each official upstream-owned edit.

No dependency version changed. Native SDL2/GXM, static LPeg/LFS preloads,
and the M2.1 Option B dynamic-loading/symlink limits are retained. No new
plugin, generic compatibility stub, or proprietary asset was added.

## Build and audit

Pinned Windows VitaSDK `2026.08`, sequence `2026092555`, GCC/G++ `15.2.0`,
CMake `4.4.3`, Ninja `1.13.2`. Dependency inventory remains the M2.1 list in
`M2_1_BUILD_REPORT.md`; the installed SDK and vendored versions did not change.
Exact commands:

```powershell
$env:VITASDK = 'C:/vitasdk'
cmake -S . -B build-vita-engine -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:/vitasdk/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DVITA=ON -DWITH_LUAJIT=ON -DWITH_MOVIES=OFF -DWITH_UPDATE_CHECK=OFF -DWITH_MIDI_DEVICE=OFF -DWITH_TRACY=OFF -DFETCH_SOUNDFONT=OFF -DFETCH_UNICODE_FONT=OFF -DENABLE_UNIT_TESTS=OFF -DBUILD_ANIMVIEW=OFF -DBUILD_TOOLS=OFF -DSEARCH_LOCAL_DATADIRS=OFF -DUSE_SOURCE_DATADIRS=OFF
cmake --build build-vita-engine --parallel 4
python vita/tests/audit_startup_package.py
```

The first configure failed with three `install DIRECTORY/FILES given no
DESTINATION` errors at `CorsixTH/CMakeLists.txt:401–405` because the initial
Vita path override omitted `CORSIX_TH_DATADIR`. Setting the original install
data directory in that branch corrected it. The successful configure, full
46-step build, final 10-step rebuild, initial error, package audit and full
resource listing are in `evidence/m2-2/`. Final configure/build exited 0 with
no compiler or linker warnings. The audit exited 0: **361 unique VPK entries**
(359 resources, SELF, SFO), no missing/extra entries or CRC failures, and
every resource SHA256 matches its source. `config.path.txt` has the exact
writable path; generated `config.h` selects `app0:/CorsixTH.lua`; the SFO has
development title ID `CORSXP003`. `.symtab` and `.debug_info` are present in
the ELF.

| Artifact | Bytes | SHA256 |
| --- | ---: | --- |
| `build-vita-engine/CorsixTH/CorsixTH` (symbol-bearing ELF) | 12,695,028 | `bf307090608ad13c70245719c53c6ae9e54acb69963b56c7ec922a6e8bad5f04` |
| `build-vita-engine/CorsixTH/eboot.bin` (SELF) | 2,799,887 | `cfb256cd181d2940ed0f1779fb6b40373eb6e75b83674013158135b164ec890a` |
| `build-vita-engine/CorsixTH/corsixth_vita_engine.vpk` | 5,652,218 | `72765fce3cbc5f1c23d05ba90218441125c1a695a7100f8acc22f1f70d8f6eb5` |

## Remaining device risks

The package audit validates layout and file bytes, while directory creation,
Lua's `app0:` reads, config loading, and boot-log flushing have only compiled;
they need the Director's physical Vita evidence. Existing SDL window creation
still requests `SDL_WINDOW_OPENGL` in `th_gfx_sdl.cpp`, assigned to Issue #8.
If that path fails at runtime, the boot log should preserve a protected Lua
or caught C++ error and SDL's last error where available. A hard process crash
could stop logging before such a line; no renderer change was made here.
The first engine boot and original-data workflow remain Issue #9.
After GPT review and Director authorization, the device check should install
the identified VPK, launch once, retrieve
`ux0:data/corsixth/logs/boot.log`, inspect the created `user/` directories,
and report the observed screen/error and exact final log line. This check has
not been performed. Desktop configure/build was not rerun in M2.2; all new
engine hooks are Vita guarded.

```text
VITA_CONFIGURE=PASS
ENGINE_COMPILE_LINK=PASS
ELF_SYMBOLS=PASS
SELF=PASS
VPK=PASS
VPK_MANIFEST=PASS_361_ENTRIES
CONFIG_REDIRECT_PACKAGE=PASS
BUILD_GATE=PENDING_GPT_ARTIFACT_REVIEW
DEVICE_DIRECTORY_CREATION=NOT_RUN
DEVICE_RESOURCE_DISCOVERY=NOT_RUN
DEVICE_BOOT_LOG=NOT_RUN
DEVICE_BOOT=NOT_RUN
DESKTOP_BUILD=NOT_RUN
```

## Physical Vita report and focused follow-up (2026-10-11)

The Director's [Issue #7 hardware report](https://github.com/VitaPorts/CorsixTH-Vita/issues/7#issuecomment-6105211356)
used the original VPK above on a real FAT Vita. It created the expected
`ux0:data/corsixth/` directories and `user/config.txt`, found packaged
`app0:` resources, loaded `CorsixTH.lua`, and initialized LuaJIT. Protected
Lua startup then failed at `Lua/app.lua:1157` (`value:match` on nil) because
neither `USER` nor `USERNAME` existed. The log subsequently recorded
`[vita:mainloop] entering`, and the device showed a black screen for about
60 seconds until the PS button was used. This establishes a **partial device
pass for startup filesystem and logging**, not a full engine boot or render
pass. No original game data was on the device for this test.

`App:fixConfig()` now converts absent environment values to `""` before
trimming, so the existing `PLAYER` fallback runs. This one-line Lua fix also
preserves a configured player name and the existing `USER` then `USERNAME`
precedence on desktop. The protected error reporter uses an SDL video path
and can return without a usable app window. On Vita only, if it returns after
a failed protected startup, the engine logs the fatal exit, destroys Lua,
closes open mixer audio, calls `SDL_Quit`, and returns status 1 before the
normal main loop. This follows the cleanup order already used on normal exit.
Whether the error reporter itself can display on Vita remains unverified.

The regression test executes the actual `App:fixConfig()` Lua function with
host LuaJIT 2.1 via test-only `lupa==2.8`. It covers absent variables, blank
variables, `USERNAME`, `USER` precedence, and a configured name; all five
passed. `lupa` is installed only under ignored `build-vita-engine/host-tools/`
and is not linked into or packaged with the game. The test and full build
transcripts are in `evidence/m2-2-hardware-followup/`.

Exact commands from the repository root:

```powershell
python -m pip install --target build-vita-engine/host-tools/lupa lupa==2.8 --no-deps
python vita/tests/test_player_name.py
$env:VITASDK = 'C:/vitasdk'
cmake -S . -B build-vita-engine -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:/vitasdk/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DVITA=ON -DWITH_LUAJIT=ON -DWITH_MOVIES=OFF -DWITH_UPDATE_CHECK=OFF -DWITH_MIDI_DEVICE=OFF -DWITH_TRACY=OFF -DFETCH_SOUNDFONT=OFF -DFETCH_UNICODE_FONT=OFF -DENABLE_UNIT_TESTS=OFF -DBUILD_ANIMVIEW=OFF -DBUILD_TOOLS=OFF -DSEARCH_LOCAL_DATADIRS=OFF -DUSE_SOURCE_DATADIRS=OFF
cmake --build build-vita-engine --clean-first --parallel 4
cmake --build build-vita-engine --parallel 4
python vita/tests/audit_startup_package.py
Get-FileHash -Algorithm SHA256 build-vita-engine/CorsixTH/CorsixTH,build-vita-engine/CorsixTH/eboot.bin,build-vita-engine/CorsixTH/corsixth_vita_engine.vpk
```

Pinned SDK remains Windows VitaSDK channel `2026.08`, sequence
`2026092555`, GCC/G++ `15.2.0`, CMake `4.4.3`, Ninja `1.13.2`; target
dependency versions are unchanged. Configure, a clean full
compile/link/package build, and a final package rerun exited 0. The rerun
recreated the VPK with a different SHA256 while the ELF and SELF stayed
identical; the hash below identifies the final audited file. There were no compiler/linker warnings,
errors, or unresolved symbols. The symbol-bearing ELF has `.symtab` and
`.debug_info`. The package audit exited 0 with **361 unique entries**:
359 CorsixTH resources, `eboot.bin`, and `sce_sys/param.sfo`; no missing or
extra entries, CRC failures, source-hash mismatches, or commercial game data.
`app0:/CorsixTH.lua`, `config.path.txt` with the `ux0:` redirect, and
development title ID `CORSXP003` are present. The audit's full archive list
is preserved in `package-audit.log`.

| New artifact | Bytes | SHA256 |
| --- | ---: | --- |
| `build-vita-engine/CorsixTH/CorsixTH` (symbol-bearing ELF) | 12,695,624 | `c5e178941969fd8b2cec92b2ceb91865786e850ee7a2a7948c7ea8c74d91ed31` |
| `build-vita-engine/CorsixTH/eboot.bin` (SELF) | 2,800,072 | `191f73ec8f0e30ab85bf2f70607a68bfa11502f8a141caf144a52f209b360fdb` |
| `build-vita-engine/CorsixTH/corsixth_vita_engine.vpk` | 5,652,769 | `2d882b26dbc76120ff0094cabc615613743358518f1a189d0d3709915897b3c0` |

```text
FOLLOWUP_LUAJIT_REGRESSION=PASS_5
FOLLOWUP_VITA_CONFIGURE=PASS
FOLLOWUP_ENGINE_COMPILE_LINK=PASS
FOLLOWUP_ELF_SYMBOLS=PASS
FOLLOWUP_SELF=PASS
FOLLOWUP_VPK=PASS
FOLLOWUP_VPK_MANIFEST=PASS_361_ENTRIES
PR_REVIEW=PENDING
NEW_ARTIFACT_DEVICE_TEST=NOT_RUN
FULL_ENGINE_BOOT=BLOCKED_ON_NEW_DEVICE_TEST
RENDERER=NOT_REACHED_IN_REPORTED_TEST
```

The new VPK must receive separate GPT review and another Director hardware
test. No renderer flags or original game assets were changed or included.
