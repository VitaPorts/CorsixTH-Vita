# M1A native SDL2/GXM probe (Issue #4)

Standalone Vita diagnostic. It does not need CorsixTH, game data, audio, Lua, or extra plugins.

## Build on the pinned Windows VitaSDK

```powershell
$env:VITASDK = 'C:/vitasdk'
$env:Path = 'C:/vitasdk/bin;C:/Program Files/CMake/bin;' + $env:Path
cmake -S vita/probe -B build-vita-probe -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:/vitasdk/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo '-DPROBE_SDK_ID=2026.08-2026092555'
cmake --build build-vita-probe --parallel 4
```

The unstripped, symbol-bearing executable is `build-vita-probe/corsixth_m1a_probe` (ELF). Packaging also creates `build-vita-probe/eboot.bin` and `build-vita-probe/corsixth_m1a_probe.vpk`, title ID `CORSXP001`.

## Expected screen and device test

At 960×544, expect a dark background with a `CORSIXTH-VITA / M1A GXM PROBE` header, build ID and run number. The left status strip lists seven checks. A large magenta and teal panel in the right half contains a red rectangle and two yellow crossing diagonal lines. That panel is first rendered into a 320×240 RGBA8888 target texture, then copied to the backbuffer. White vertical bars beside it and a blue bar near the bottom are drawn directly to the backbuffer. The footer shows `CROSS: RERUN | START: EXIT`.

Install the VPK using VitaShell. Launch without game assets, photograph the screen, press Cross once to increment `RUN`, then press Start. Retrieve `ux0:data/corsixth/probe.log` over USB or FTP. The log is created before SDL startup, flushed after each line, and includes SDL versions, renderer capabilities, API results and run summaries. If file opening fails, the on-screen LOGFILE check shows FAIL when a renderer is available. A renderer creation failure may leave no display, but is retained in the log.

The source checks SDL return values and reports attempted calls. `SDL_RenderPresent` has no return value. A `COMPOSITE: PASS` line means SDL calls succeeded; only the device photo can confirm pixels reached the Vita display. `RENDERER: PASS` requires a native accelerated Vita-named renderer and its target-texture flag.

## Machine-readable acceptance checklist

```json
{
  "issue": 4,
  "build": ["configure_exit_0", "compile_exit_0", "link_exit_0", "vpk_exists", "elf_has_symbols", "vpk_sha256_recorded"],
  "device": ["native_renderer_identified", "targettexture_flag", "offscreen_panel_visible", "cross_increments_run", "start_exits", "probe_log_retrieved"],
  "device_status": "NOT_RUN"
}
```
