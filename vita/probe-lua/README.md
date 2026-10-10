# M1B Lua and filesystem probe — Issue #5

Standalone C++17 Vita diagnostic based on upstream CorsixTH v0.70.1 lineage and M1A merge `0671151ee45f591f08d57c5d5c3f0590cf1aa1c0`. It does not start CorsixTH or modify its engine. The attached design reference is represented by a dark 960×544 status panel with per-test PASS/FAIL labels and a Start exit instruction; no external fonts or assets are packaged.

## Native Windows build

```powershell
$env:VITASDK = 'C:/vitasdk'
$env:Path = 'C:/vitasdk/bin;C:/Program Files/CMake/bin;' + $env:Path
cmake -S vita/probe-lua -B build-vita-probe-lua -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:/vitasdk/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo '-DPROBE_SDK_ID=2026.08-2026092555'
cmake --build build-vita-probe-lua --parallel 4
```

Build outputs are `build-vita-probe-lua/corsixth_m1b_probe` (unstripped, symbol-bearing ELF), `eboot.bin`, and `corsixth_m1b_probe.vpk` with non-production title ID `CORSXP002`. Dependencies and license details are in [M1B_PROVENANCE.md](../../third_party/M1B_PROVENANCE.md).

## Device procedure

Install the VPK with VitaShell, launch without game assets, photograph the status panel, press Start to flush the final summary and exit, then retrieve `ux0:data/corsixth/m1b-probe.log`. The log starts before SDL and flushes every line. It records Lua 5.1 ABI, protected expression result, LuaJIT version/JIT status, static LPeg and LuaFileSystem loading, native/Lua/`std::filesystem` ux0 file operations, errors, and a final summary. A logger failure is shown in the panel when SDL display is available.

The probe only writes its log and short tokens inside `ux0:data/corsixth/m1b/`. It removes its token files after successful tests and leaves M1A's `probe.log` alone. JIT status is informational. Runtime PASS requires the Director's actual Vita log and photo; a host build proves only compilation/linking/packaging.

The final runtime log records `runtime=VITA_EXECUTED` and `device_acceptance=PENDING_DIRECTOR_REVIEW`. It counts required test failures separately from the optional C++17 filesystem result. A `std::filesystem` API error is logged as `UNSUPPORTED_OPTIONAL` with the operation, code, category, message, and required native fallback result; it is never converted to PASS.

## Machine-readable acceptance checklist

```json
{
  "issue": 5,
  "build": ["configure_exit_0", "compile_exit_0", "link_exit_0", "vpk_exists", "elf_has_symbols", "vpk_sha256_recorded"],
  "device": ["lua_abi_01", "lpeg_mod_02", "lfs_mod_01", "native_fs_01", "lua_fs_02", "cpp17_fs_03_or_documented_fallback", "start_sys_01", "log_retrieved", "photo_matched"],
  "device_status": "NOT_RUN"
}
```
