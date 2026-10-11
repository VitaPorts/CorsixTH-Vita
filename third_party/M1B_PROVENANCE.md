# M1B static Lua module provenance (Issue #5)

These sources originated with the standalone `vita/probe-lua` diagnostic and
are reused by the Vita engine target in Issue #6. Their versions are unchanged.

| Component | Exact source | Integrity | License | Local adjustment |
| --- | --- | --- | --- | --- |
| LPeg 1.1.0 | [Author's release tarball](https://www.inf.puc-rio.br/~roberto/lpeg/lpeg-1.1.0.tar.gz) | downloaded tar.gz SHA256 `4b155d67d2246c1ffa7ad7bc466c1ea899bbc40fef0257cc9c03cecbaed4352a` | MIT-style Lua.org/PUC-Rio license in `lpeg-1.1.0/lpeg.html` | `lptypes.h`: undefine LuaJIT-provided `luaL_newlib` before LPeg's Lua 5.1 shim to avoid macro-redefinition warning; no behavior change to LPeg shim. |
| LuaFileSystem 1.8.0 | [lunarmodules/luafilesystem](https://github.com/lunarmodules/luafilesystem/tree/7c6e1b013caec0602ca4796df3b1d7253a2dd258), tag `v1_8_0` | Git commit `7c6e1b013caec0602ca4796df3b1d7253a2dd258`; original copied `src/lfs.c` SHA256 `15aea4c85ea4ddf8c26b96db171dbf92a1e300d211f0ad004aa2089cf0772afe` | MIT in `luafilesystem-1.8.0/LICENSE` | Issue #6: Vita-only `CORSIXTH_VITA_LFS_NO_LINKS` guard returns `nil, error, ENOSYS` for link-related APIs; applied `src/lfs.c` SHA256 `aa13c6ca1194f5962b5fe6c375744a908a81160f3d5679cf836ffbb70788b8ad`. The M1B probe does not define this guard. |
| LuaJIT | VitaSDK package `luajit 2.1.0-1`, sourced per Issue #5 from `SonicMastr/LuaJIT-Vita` commit `c329ddd10691c1875f26087ba23c2ae278515e24` | installed `libluajit-5.1.a` SHA256 `a86e433f50d2d35c55c998cc0c4c7c76f7d9d909e1b28fa41f670debdc9b5b4c` | LuaJIT MIT-style license from its upstream source | SDK package unmodified. |

The installed LuaJIT header states Lua ABI 5.1, `lua_Integer=ptrdiff_t`, `lua_Number=double`, and `LuaJIT 2.1.0-beta3`. The static archive exports `luaL_newstate`, `lua_pcall`, and `luaopen_jit`. The package declares `taihen 0.11-1` and `vita-rss-libdl 0.3.2-1` as dependencies, but their installation does **not** prove on-device JIT execution or authorize new plugins.

The probe statically preloads `lfs` and `lpeg`. `vita/probe-lua/vita_compat.c` returns `ENOSYS` for unused dynamic loading and POSIX symlink operations to avoid linking extra runtime/plugin dependencies. `lfs.link`, `lfs.symlinkattributes` targets, and Lua dynamic C module loading are outside M1B acceptance. The diagnostic records JIT status separately from Lua interpreter success.
