# M2.1 engine build evidence — Issue #6

Status: **BLOCKED at final ELF link**. Vita configure passed and all 41 engine/vendor
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
