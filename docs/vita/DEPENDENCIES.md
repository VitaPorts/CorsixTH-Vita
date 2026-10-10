# Pinned dependency and build policy

## Initial baseline
- Upstream CorsixTH v0.70.1, peeled commit `56bd5d00f76331c7f76d7b696726a7926303ca0c`.
- **Native Windows 10 development**: official VitaSDK PowerShell bootstrap via `vitasdk/vdpm`, candidate supported channel `2026.08`; pin the actual installer/release, toolchain version and package inventory from issue #3.
- **Independent CI**: candidate SDK image `vitasdk/vitasdk:2026.08-20260916.2`, whose immutable digest must be recorded. Reconcile its Vita target dependency versions with Windows; parity is not assumed.
- Native v0.70.1 uses C++17, SDL2, SDL2_mixer, Lua/LuaJIT, FreeType, PNG, ZLIB. Start with movies/update-checker/RtMidi/Tracy/downloaded assets off.
- LPeg and LuaFileSystem: current project's static preloading exists in engine source; Vita linker/source acquisition requires separately vetted source versions, licenses, compatibility, exact Lua 5.1 ABI. Don't assume vcpkg `unofficial-*` find_package targets work on Vita.

## Baseline configuration after VITA CMake support exists (not valid on an untouched upstream checkout)
```bash
# Platform-neutral reference; use the corresponding PowerShell path variable on Windows.
cmake -S . -B build-vita -G 'Unix Makefiles'   -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake"   -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON   -DVITA=ON -DWITH_LUAJIT=ON -DWITH_MOVIES=OFF   -DWITH_UPDATE_CHECK=OFF -DWITH_MIDI_DEVICE=OFF -DWITH_TRACY=OFF   -DFETCH_SOUNDFONT=OFF -DFETCH_UNICODE_FONT=OFF   -DENABLE_UNIT_TESTS=OFF -DBUILD_ANIMVIEW=OFF -DBUILD_TOOLS=OFF   -DSEARCH_LOCAL_DATADIRS=OFF -DUSE_SOURCE_DATADIRS=OFF
cmake --build build-vita -j4
```

## M1A probe
Only SDL2, VitaSDK base, C++17, filesystem logger; do not add Lua, codec trees or actual engine. Require `SDL_RendererInfo.name`, flags, creation of `SDL_TEXTUREACCESS_TARGET`, two passes with `SDL_SetRenderTarget`, and visible verification against a known expected image. M1B adds Lua + module and read/write probes. Separate unrelated link issues.

## Third-party integrity
Maintain under `third_party/` only what is unavoidable. Pin upstream source tag/SHA, origin URL, applied patches, copyright/license and rationale. Keep commercial Theme Hospital resources out of source and release ZIP/VPK.
