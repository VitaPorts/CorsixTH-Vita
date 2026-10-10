# M2.1 final linker compatibility investigation — Issue #6 / draft PR #20

**Decision status: architecture review required.** This document investigates
commit `be725a6d270c4755ebab4ae16c415cff81a7d279`. It does not implement
the proposed adapter. The engine remains **BUILD BLOCKED** at six unresolved
symbol names; no ELF, SELF, VPK, hashes, or device run exist.

## Reachability in the shipped engine

Search scope: `CorsixTH/CorsixTH.lua`, all `CorsixTH/Lua/**/*.lua`, and
`CorsixTH/Src{,Unshared}` C++/headers. `SrcUnshared/main.cpp` calls
`luaL_openlibs`; `Src/main.cpp` registers `lfs` and `lpeg` through the
existing `package.preload` hook. `CorsixTH.lua` adds `base_dir .. '?.so'` to
`package.cpath`, so the general Lua native-module loader is exposed even
though the shipped code uses static preloads. Normal Lua source loading uses
`package.path` and `corsixth.require`, which explicitly loads `.lua` files.

| Capability | Shipped call-site finding | Consequence |
| --- | --- | --- |
| `require('lfs')` | `Lua/app.lua`, `audio.lua`, `config_finder.lua`, `strings.lua`, `utility.lua`, `ui.lua`, browsers and others | Required. Keep static preload and normal LFS I/O. |
| `require('lpeg')` | `Lua/string_extensions.lua` | Required. Keep static preload and LPeg behavior. |
| LFS `attributes`, `dir`, `mkdir`, `currentdir` | Startup config creation, save/log directories, resource discovery, browsers, audio lookup | Required. M1B tested only a reduced subset on Vita; full engine paths remain untested. |
| LFS `link`, `lock_dir`, `symlinkattributes` | No call sites in shipped Lua/C++ | Archive compatibility paths. Disabling them would change the public LFS API for Vita scripts/mods and needs review. |
| LFS `lock`, `unlock`, `touch`, `rmdir`, `chdir` | No call sites in shipped Lua/C++ | Present APIs; do not change as part of this link fix. |
| `package.loadlib`, `.so`/`.dll` native `require` | No call sites in shipped code; `package.cpath` is set in `CorsixTH.lua:23` | Dynamic native modules are an exposed extension capability, not a shipped startup requirement. |
| `ffi.load`, `ffi.C`, `require('ffi')` | No call sites in shipped code | LuaJIT archive includes FFI loader references, but shipped engine does not request FFI. External mods may. |

The two LFS symbols arise at `lfs.c:474` (`lock_dir`), `:583` (`link`), and
`:1089` (`symlinkattributes` target lookup). LuaJIT's four `dl*` symbols
arise from `lj_clib.o` and `lib_package.o`. These are link-time references
even when the corresponding Lua APIs are not called. A source search cannot
prove what user-installed mods might do; this boundary applies only to the
redistributed engine code.

## SDK `libdl` linker experiment

The reproducible C file and outputs are in `evidence/m2-1-linker/`. Commands
run with the pinned Windows VitaSDK 2026.08 / GCC 15.2.0:

```powershell
& C:/vitasdk/bin/arm-vita-eabi-gcc.exe '-Wl,-q' build-vita-engine/dl_probe.c -ldl -o build-vita-engine/dl_probe.elf
& C:/vitasdk/bin/arm-vita-eabi-gcc.exe '-Wl,-q' build-vita-engine/dl_probe.c -ldl -ltaihen_stub -lSceSblSsMgr_stub -o build-vita-engine/dl_probe_base.elf
& C:/vitasdk/bin/arm-vita-eabi-gcc.exe '-Wl,-q' build-vita-engine/dl_probe.c -ldl -lSceSblSsMgr_stub -o build-vita-engine/dl_probe_no_tai.elf
& C:/vitasdk/bin/arm-vita-eabi-gcc.exe '-Wl,-q' build-vita-engine/dl_probe.c -ldl -ltaihen_stub -o build-vita-engine/dl_probe_no_sbl.elf
```

`-ldl` alone fails on `taiGetModuleInfo` and
`sceSblDmac5HashTransform`. Adding both stubs links the **minimal probe**;
omitting either reproduces its named unresolved symbol. `nm -u libdl.a`
also lists `sceKernelLoadStartModule`, `sceKernelStopUnloadModule`,
`sceKernelGetModuleInfo`, `module_get_export` (defined in the archive), C
library functions, and the two symbols above. Toolchain defaults resolve the
other imports in this small experiment. The [libdl maintainer's usage
instructions](https://github.com/isage/vita-rss-libdl) also specify
`-ltaihen_stub` and `-lSceSblSsMgr_stub` and describe loading a `.suprx`.
The installed package metadata declares `taihen 0.11-1` as a dependency.

This shows **link feasibility only**. `libdl` uses Vita module loading and
the taiHEN export lookup service; the [taiHEN project](https://github.com/yifanlu/taiHEN)
documents that its user stub calls into the taiHEN runtime framework. Whether
the Director's device already provides every runtime service, and whether a
particular `.suprx` can be loaded, were not tested. Linking `libdl` into the
engine would add a taiHEN API dependency, so it is not an approved M2.1 fix.
It also does not supply LFS `symlink`/`readlink`.

## Options for GPT review

| Option | Size / link outlook | Static LFS/LPeg | Dynamic C modules and FFI | Runtime / plugin risk | Maintenance |
| --- | --- | --- | --- | --- | --- |
| A. Link SDK `libdl` plus a separate LFS adaptation | Small CMake change plus LFS work; `dl*` link proven only in minimal probe | Preserved | Attempts `.suprx` loading | Adds taiHEN service import; device behavior unknown | SDK/link dependency, LFS patch |
| B. Vita-gated LFS unsupported symlink operations plus explicit LuaJIT dynamic-loader denial | Small source adapter; expected to resolve six symbols, **not yet linked** | Preserved if ordinary LFS code stays intact | Explicit failure with error text, never success | No added plugin; loss of Vita dynamic native modules/FFI is a Lua API change | Small Vita guards in pinned LFS, small Vita loader adapter; review required |
| C. Rebuild/pin LuaJIT with dynamic loader/FFI removed plus LFS adaptation | Larger rebuild and ABI/toolchain validation | Must be retested | Removed or explicit failure, depending build | No added plugin if correctly built; broad runtime regression risk | New LuaJIT build provenance and ongoing fork burden |
| D. Implement POSIX links and use `libdl` | Large platform work; link unknown | Potentially preserved | Potentially enabled | Adds filesystem semantics and taiHEN service dependency | Highest |

**Recommendation for review: B**, scoped only to Vita. Preserve current
`lfs.attributes`, `dir`, `mkdir`, `currentdir`, normal Lua `io`, and static
`package.preload` behavior. Compile the pinned LFS source with a Vita-only
feature guard so `lfs.link`, `lfs.lock_dir`, and `lfs.symlinkattributes` return
an explicit Lua failure (for example `nil, "symbolic links unsupported on
Vita", ENOSYS`) instead of reporting success or silently treating links as
ordinary files. Keep the APIs visible so callers receive a clear error. A
separate Vita-only loader adapter would make LuaJIT's dynamic module and FFI
entry points fail with an explicit diagnostic while leaving static preloads
untouched. Its exact `dlerror`/`errno` semantics and user-script impact need
GPT approval before implementation. Do not copy the M1B probe's generic
global ENOSYS file into the engine. If approved, document and hash the
minimal change to pinned vendored LFS separately; retain its original
provenance hash and keep the M1B diagnostic build unaffected.

No solution is implemented here. After architecture approval, rerun the
actual engine link and then package only if it succeeds. PR #20 remains draft;
device installation and test remain out of scope.
