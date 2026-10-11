# M2.3 native SDL2/GXM renderer — Issue #8

Status: **BUILD PASS / ENGINE DEVICE NOT RUN / GPT REVIEW PENDING** on
`codex/m2-renderer-gxm`, based on Director-approved M2.2 merge
`f205bb9a015a95738d36a8683298ee450a57af07`. Official CorsixTH baseline
remains v0.70.1, peeled commit
`56bd5d00f76331c7f76d7b696726a7926303ca0c`.

## Reproduction and decision

The physical FAT Vita log in [Issue #8](https://github.com/VitaPorts/CorsixTH-Vita/issues/8#issuecomment-6105396130)
reached `TH.surface()` after filesystem and Lua startup, then failed with
`OpenGL support is either not configured in SDL or not available in current SDL video driver (VITA) or platform`.
The prior engine window requested `SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE`.
The [M1A reduced hardware test](https://github.com/VitaPorts/CorsixTH-Vita/issues/4)
proved a native `VITA gxm` accelerated renderer, target-texture capability,
and visible RGBA8888 offscreen composition at 960×544. Its reported flags
were `0x0000000a` (accelerated and target texture), max texture size 4096×4096,
and SDL2 runtime 2.32.8. That test did **not** exercise the engine's
ABGR8888 target format or its complete renderer lifecycle.

The only production edit is `CorsixTH/Src/th_gfx_sdl.cpp`:

- Vita now requests `SDL_WINDOW_SHOWN`, matching the reduced native probe.
  The fixed Vita display has no resizing use; desktop still requests the
  original OpenGL and resizable flags under the non-Vita branch.
- `SDL_AllocFormat`, `SDL_CreateWindow`, `SDL_CreateRenderer` and the initial
  `SDL_GetRendererInfo` now have checked error paths with operation names and
  exact `SDL_GetError` messages. The later renderer-details query is checked
  as well. Constructor failures still flow through `TH.surface()`'s existing
  protected Lua startup diagnostic and flushed Vita boot log.
- Vita logs the actual renderer name, flags, target-texture flag, advertised
  ABGR8888 format, maximum texture dimensions and format count to
  `ux0:data/corsixth/logs/boot.log`. A renderer without
  `SDL_RENDERER_TARGETTEXTURE` fails explicitly on Vita. The advertised
  format list is diagnostic; it does not prove ABGR8888 target creation.
- The existing ABGR8888 `SDL_TEXTUREACCESS_TARGET` creation now checks for
  null before `SDL_SetRenderTarget`. Vita logs exact allocation or target
  selection errors and the failed scoped target remains invalid; the
  existing `set_scale_factor()` path can detect that with `is_target()`.

`docs/vita/UPSTREAM_DELTA.md` records the upstream-owned edit. No SDL major
version, target dependency, CMake option, engine Lua/gameplay path, plugin,
vitaGL component, or commercial Theme Hospital asset changed. The renderer
still needs physical validation of ABGR8888 target creation, composition,
screen output and any next startup error.

## Toolchain, commands and results

Pinned native Windows VitaSDK channel `2026.08`, sequence `2026092555`,
core `sdk-snapshot-20260825.611.1`, packages
`packages-2026.08-snapshot-20260925.50.1`; GCC/G++ 15.2.0, CMake 4.4.3,
Ninja 1.13.2, SDK SDL2 2.32.8. Other target dependencies remain the pinned
M2.1/M2.2 inventory. Exact toolchain and package fingerprints are in
`evidence/m2-3/toolchain.txt`. The Vita compiler defines `__vita__=1`.

Run from the repository root:

```powershell
$env:VITASDK = 'C:/vitasdk'
cmake -S . -B build-vita-engine -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:/vitasdk/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DVITA=ON -DWITH_LUAJIT=ON -DWITH_MOVIES=OFF -DWITH_UPDATE_CHECK=OFF -DWITH_MIDI_DEVICE=OFF -DWITH_TRACY=OFF -DFETCH_SOUNDFONT=OFF -DFETCH_UNICODE_FONT=OFF -DENABLE_UNIT_TESTS=OFF -DBUILD_ANIMVIEW=OFF -DBUILD_TOOLS=OFF -DSEARCH_LOCAL_DATADIRS=OFF -DUSE_SOURCE_DATADIRS=OFF
cmake --build build-vita-engine --clean-first --parallel 4
python vita/tests/audit_startup_package.py
Get-FileHash -Algorithm SHA256 build-vita-engine/CorsixTH/CorsixTH,build-vita-engine/CorsixTH/eboot.bin,build-vita-engine/CorsixTH/corsixth_vita_engine.vpk
```

Configure exited 0. Clean full engine compile, link, ELF conversion, SELF
creation and VPK packaging exited 0. No compiler or linker warnings, errors,
or unresolved symbols were found. `arm-vita-eabi-readelf -S` confirms
`.symtab` and `.debug_info` in the ELF. Cross-compiler preprocessing of this
source selects `SDL_WINDOW_SHOWN` with `__vita__` and the exact prior
`SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE` expression with `-U__vita__`;
this verifies the source guard, not a desktop executable. Desktop configure
and tests were **NOT RUN** because this Windows environment has no `cl`,
`g++`, `clang++` or `gcc` host compiler.

The package audit exited 0: **361 unique VPK entries** comprising 359
redistributable CorsixTH resources, `eboot.bin`, and `sce_sys/param.sfo`.
It checked the exact generated manifest, CRCs, resource SHA256 bytes,
`app0:/CorsixTH.lua`, writable config redirect, development title ID
`CORSXP003`, and absence of commercial-data prefixes. Full transcripts,
the exact resource manifest and archive listing are in `evidence/m2-3/`.

| Artifact | Bytes | SHA256 |
| --- | ---: | --- |
| `build-vita-engine/CorsixTH/CorsixTH` (symbol-bearing ELF) | 12,744,548 | `7728f1169fe3c22ac49006830420b97370985e468176440e9eb5aafb01b5803d` |
| `build-vita-engine/CorsixTH/eboot.bin` (SELF) | 2,801,003 | `68a6066d147afcd7aeb3325cdcbb46cbb39f6132751e8ecde30413b607db6e8d` |
| `build-vita-engine/CorsixTH/corsixth_vita_engine.vpk` | 5,653,956 | `a99722120a6e781c5ca34ed9fd08fc7535ea5db9c87a51d257e7f329e2e8aa14` |

The VPK hash identifies this exact audited file. The Vita packaging step
can assign fresh ZIP timestamps on a rerun, so rebuilds may have a different
VPK checksum even when ELF and SELF bytes are unchanged.

```text
VITA_CONFIGURE=PASS
ENGINE_COMPILE_LINK=PASS
ELF_SYMBOLS=PASS
SELF=PASS
VPK=PASS
VPK_MANIFEST=PASS_361_ENTRIES
VITA_WINDOW_FLAG_PREPROCESS=PASS
DESKTOP_GUARD_PREPROCESS=PASS
DESKTOP_CONFIGURE=NOT_RUN_NO_HOST_COMPILER
ENGINE_GXM_DEVICE_TEST=NOT_RUN
ENGINE_ABGR8888_TARGET_DEVICE_TEST=NOT_RUN
ENGINE_RENDER_OUTPUT_DEVICE_TEST=NOT_RUN
GPT_REVIEW=PENDING
```

After GPT source/artifact review, the Director's hardware check should use
the exact VPK hash above, launch without original game data, inspect the
screen and `ux0:data/corsixth/logs/boot.log`, and report the renderer line,
any ABGR8888 target error, the first visible frame or next Lua error, and
whether shutdown is clean. That test has not been performed for this build.
