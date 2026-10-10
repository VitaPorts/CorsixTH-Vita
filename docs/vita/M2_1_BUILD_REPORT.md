# M2.1 engine build evidence — Issue #6

## Approved Option B result

Status: **BUILD PASS / DEVICE NOT RUN** for the approved Vita-only adapter on
branch `codex/m2-cmake-packaging`, starting from
`4ad8365000eaa6d2c489faca5858ce40f40cf123`. See the [Issue #6 architecture
decision](https://github.com/VitaPorts/CorsixTH-Vita/issues/6#issuecomment-6101884632).
PR #20 remains draft. This is a link/package result, not evidence of boot,
rendering, resource lookup, or gameplay on Vita hardware.

The only production source changes are vendored LuaFileSystem's `lfs.c`,
`vita/platform/vita_dynamic_loader_unavailable.c`, and the Vita CMake module.
`CORSIXTH_VITA_LFS_NO_LINKS` is private to the engine LFS target. On Vita,
`lfs.link` (hard and symbolic), `lfs.lock_dir`, and `lfs.symlinkattributes`
(table and `"target"` forms) retain their Lua names and return
`nil, "<operation>: <strerror(ENOSYS)>", ENOSYS`. The ordinary
directory, attributes, and file code remains compiled. The M1B probe and
desktop builds do not define that guard. LuaJIT's four correctly typed `dl*`
entry points deny dynamic native module/FFI loading with `ENOSYS`; `dlerror`
returns a nonempty per-thread diagnostic once. No `libdl`, taiHEN import,
kernel plugin, new LuaJIT build, or dependency version was added.

The first Option B engine link produced a symbol-bearing ELF and SELF but
packaging failed on Windows: `vita-pack-vpk` received 358 separate file
mappings, making the command approximately 36,475 characters, and `cmd.exe`
reported `The system cannot execute the specified program.` The Vita CMake
module now passes directory mappings for `Lua`, `Campaigns`, and `Levels`,
and stages only the selected `Bitmap` files. This resolves the Windows
command-length failure without adding resources.

Exact final build commands, with VitaSDK `2026.08` / sequence `2026092555`,
GCC 15.2.0, CMake 4.4.3, and Ninja 1.13.2:

```powershell
$env:VITASDK = 'C:/vitasdk'
cmake -S . -B build-vita-engine -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:/vitasdk/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DVITA=ON -DWITH_LUAJIT=ON -DWITH_MOVIES=OFF -DWITH_UPDATE_CHECK=OFF -DWITH_MIDI_DEVICE=OFF -DWITH_TRACY=OFF -DFETCH_SOUNDFONT=OFF -DFETCH_UNICODE_FONT=OFF -DENABLE_UNIT_TESTS=OFF -DBUILD_ANIMVIEW=OFF -DBUILD_TOOLS=OFF -DSEARCH_LOCAL_DATADIRS=OFF -DUSE_SOURCE_DATADIRS=OFF
cmake --build build-vita-engine --parallel 4
```

Final configure and build exited 0 without compiler/linker warnings. Logs are
in `evidence/m2-1-option-b/configure.log` and `build.log`. The host contract
tests used Zig 0.14.1 as a Windows test compiler, not as the Vita compiler:

```powershell
& build-vita-engine/host-tools/ziglang/zig.exe cc -target x86_64-windows-gnu -O2 -ffunction-sections -fdata-sections '-Wl,--gc-sections' -std=c11 -D__vita__=1 -DCORSIXTH_VITA_LFS_NO_LINKS=1 -I C:/vitasdk/arm-vita-eabi/include/luajit-2.1 vita/tests/lfs_link_contract.c -o build-vita-engine/lfs_link_contract.exe
& build-vita-engine/lfs_link_contract.exe
& build-vita-engine/host-tools/ziglang/zig.exe cc -target x86_64-windows-gnu -std=c11 -D__vita__=1 -I vita/tests/include vita/platform/vita_dynamic_loader_unavailable.c vita/tests/dynamic_loader_contract.c -o build-vita-engine/dynamic_loader_contract.exe
& build-vita-engine/dynamic_loader_contract.exe
```

Both host tests exited 0. The LFS test invokes the actual vendored guarded
functions with a minimal Lua stack recorder and checks the three return
values, `errno`, and operation text for all five forms. It compiled with one
host-only `_WIN32_WINNT` macro redefinition warning from vendored LFS. This
test compiles LFS's Windows host branch with the Vita guard forced on; the
Vita POSIX branch was compiled and linked by GCC but not executed on device.
The loader test checks all four entry points and `dlerror` consume/clear
behavior. Evidence is in `evidence/m2-1-option-b/*host*.log`.

| Artifact | Size | SHA256 |
| --- | ---: | --- |
| `build-vita-engine/CorsixTH/CorsixTH` (symbol-bearing ELF) | 12,915,024 bytes | `ad1ca1375fb9cacafcff30bf83f95200871fd34c8b08da8946da60c0a36b102e` |
| `build-vita-engine/CorsixTH/eboot.bin` (SELF) | 2,846,576 bytes | `299b2e86de84622a30380e23a43a5bec90aff89ed6419aa1586f44cecbf117b6` |
| `build-vita-engine/CorsixTH/corsixth_vita_engine.vpk` | 5,699,087 bytes | `c50c12e31ee7484ff6849e00bc265a8ccfc657f453bf4966f180baf4223dbbc8` |

`readelf` confirms `.symtab` and `.debug_info` in the ELF. `nm` confirms
`luaopen_lfs`, `luaopen_lpeg`, and all four `dl*` definitions; no unresolved
`readlink`, `symlink`, `dl*`, `taiGetModuleInfo`, or
`sceSblDmac5HashTransform` remains. The complete 360-entry VPK listing is
`evidence/m2-1-option-b/vpk-audit.log`; it exactly matches the 358-item
resource manifest plus `eboot.bin` and `sce_sys/param.sfo`. There are zero
duplicate or extra entries, all ZIP CRCs pass, each resource's SHA256 matches
the tracked source, and the SFO contains development ID `CORSXP003`. No
commercial Theme Hospital game data is packaged.

No official upstream-owned source file changed in Option B; the existing
`UPSTREAM_DELTA.md` integration points remain the maintenance ledger.
Runtime risks remain: ordinary LFS operations have not been exercised in the
full engine on hardware, dynamic native modules and FFI loading are explicitly
unavailable, and startup paths/SDL window behavior belong to later issues.
The VPK has not been installed.

```text
VITA_CONFIGURE=PASS
ENGINE_COMPILE=PASS
ENGINE_LINK=PASS
STATIC_LFS_LPEG=LINKED
UNSUPPORTED_API_HOST_CONTRACT=PASS
SELF=PASS
VPK=PASS
VPK_MANIFEST=PASS_360_ENTRIES
DEVICE=NOT_RUN
```

## Earlier M2.1 result (before Option B)

At the earlier branch head, status was **BLOCKED at final ELF link**. Vita configure passed and all 41 engine/vendor
compile steps completed. No ELF, SELF, or VPK was produced. Device was not run.
The source baseline is CorsixTH v0.70.1, peeled commit
`56bd5d00f76331c7f76d7b696726a7926303ca0c`. The M2 branch began at
`8ecc94cae9f41eefc014ceb39b863dd07493b126`; use `git rev-parse HEAD`
after checkout to identify the final review commit.

## Native Windows environment

- VitaSDK `C:/vitasdk`, release `2026.08`, sequence `2026092555`, toolchain
  `sdk-snapshot-20260825.611.1`, packages snapshot
  `packages-2026.08-snapshot-20260925.50.1`.
- `arm-vita-eabi-g++` 15.2.0, host CMake 4.4.3, Ninja 1.13.2.
- Installed target packages: SDL2 2.32.8-1, SDL2_mixer 2.8.2-1,
  LuaJIT 2.1.0-1, FreeType 2.14.3-1, libpng 1.6.58-1, ZLIB 1.3.2-2,
  bzip2 1.0.8-1, FLAC 1.3.4-1, libmodplug
  0.8.9.0.r461.gd1b97ed-1, libogg 1.3.6-2, libvorbis 1.3.7-2,
  libxmp-lite 4.7.3-1, opus 1.6.1-1, opusfile 0.12-1.
- LPeg 1.1.0 and LuaFileSystem 1.8.0 use the existing vetted sources and
  hashes in `third_party/M1B_PROVENANCE.md`. No dependency version changed.

## Reproduce

```powershell
$env:VITASDK = 'C:/vitasdk'
cmake -S . -B build-vita-engine -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:/vitasdk/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DVITA=ON -DWITH_LUAJIT=ON -DWITH_MOVIES=OFF -DWITH_UPDATE_CHECK=OFF -DWITH_MIDI_DEVICE=OFF -DWITH_TRACY=OFF -DFETCH_SOUNDFONT=OFF -DFETCH_UNICODE_FONT=OFF -DENABLE_UNIT_TESTS=OFF -DBUILD_ANIMVIEW=OFF -DBUILD_TOOLS=OFF -DSEARCH_LOCAL_DATADIRS=OFF -DUSE_SOURCE_DATADIRS=OFF
cmake --build build-vita-engine --parallel 4
```

The exact configure/build output is in `evidence/m2-1/configure.log` and
`evidence/m2-1/build.log`. Configure succeeded without warnings. The compiler
reported no warnings. The first failed operation is the final C++ link.
Undefined references are `readlink` and `symlink` from vendored LuaFileSystem,
and `dlopen`, `dlsym`, `dlclose`, `dlerror` from VitaSDK's static LuaJIT.

The installed `libdl.a` exports the four `dl*` functions, but `nm -u` shows
it requires `taiGetModuleInfo` and other runtime APIs. Linking it would expand
the runtime/plugin dependency contract. No installed VitaSDK static archive
exports `readlink` or `symlink`. The M1B probe's `ENOSYS` functions were not
copied into the engine. Proposed review decision: approve a separately
specified Vita filesystem/dynamic-loading compatibility layer, or select
SDK builds of LuaJIT/LuaFileSystem that remove unsupported paths. Do not treat
these functions as harmless without auditing actual engine use and device
behavior.

## Packaging plan and current limit

`vita/cmake/VitaEngine.cmake` configures development title ID `CORSXP003`
(nine characters, distinct from `CORSXP001` and `CORSXP002`). Its generated
manifest has `eboot.bin` (SELF, when available), `sce_sys/param.sfo` (generated
by VitaSDK), `LICENSE.txt`, `CorsixTH.lua`, and 356 files under `Lua/`,
`Bitmap/`, `Campaigns/`, and `Levels/`. The 358 source resources in
`evidence/m2-1/vita-resource-manifest.txt` are tracked project files; no
commercial Theme Hospital HOSP/game installation directory is included.
The actual archive manifest, package command viability, ELF symbols, and
SHA256 values remain **NOT RUN** because linking stopped first. The VPK is
not an install/test candidate yet. Startup resource lookup and SDL window
flags are separately owned by Issues #7 and #8, and boot by #9.

## Desktop smoke

The Vita path is conditional. A desktop CMake configure with the same optional
features disabled was attempted and stopped before dependency resolution:
there is no native C or C++ compiler on this Windows host. Exact output is in
`evidence/m2-1/desktop-configure.log`; this is **NOT RUN** for desktop build.

```powershell
cmake -S . -B build-desktop-m2 -G Ninja -DWITH_MOVIES=OFF -DWITH_UPDATE_CHECK=OFF -DWITH_MIDI_DEVICE=OFF -DFETCH_SOUNDFONT=OFF -DFETCH_UNICODE_FONT=OFF -DENABLE_UNIT_TESTS=OFF -DBUILD_ANIMVIEW=OFF -DBUILD_TOOLS=OFF
```

## Acceptance

```text
VITA_CONFIGURE=PASS
ENGINE_COMPILE=PASS
ENGINE_LINK=BLOCKED_UNDEFINED_SYMBOLS
SELF=NOT_RUN
VPK=NOT_RUN
VPK_MANIFEST=NOT_RUN
ELF_SHA256=NOT_AVAILABLE
VPK_SHA256=NOT_AVAILABLE
DESKTOP_CONFIGURE=BLOCKED_NO_HOST_COMPILER
DEVICE=NOT_RUN
```
