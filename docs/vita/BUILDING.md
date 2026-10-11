# Developer environment: native Windows 10, alternative Linux CI

> Status, 2026-10-10: native Windows VitaSDK installation and inventory were verified under Issue #3. The standalone M1A SDL2/GXM and M1B Lua/filesystem diagnostics achieved BUILD PASS and physical Vita FAT DEVICE PASS under Issues [#4](https://github.com/VitaPorts/CorsixTH-Vita/issues/4) and [#5](https://github.com/VitaPorts/CorsixTH-Vita/issues/5). The actual CorsixTH M2.1 engine compiles but is **BLOCKED at ELF link**; it has no SELF/VPK or hardware result. The original Windows bootstrap needed the documented checksum-preserving Git for Windows bzip2 workaround in Issue #3.

## Director's host: Windows 10 x64, PowerShell 5.1+
Install Git for Windows (with Git Credential Manager), CMake, Ninja, VS Code + GitHub/Codex extensions as needed, and the official native Windows VitaSDK. Python 3 may be useful for future project scripts but is not an M0 blocker. No WSL2/Linux/Docker is required on the director's PC. Native Linux/Docker may independently be used by cloud CI.

### Import authentic upstream history FIRST (issue #1)
Open PowerShell and run (stop if the destination folder or origin already contains unexpected data):

```powershell
New-Item -ItemType Directory -Force "$HOME\projects" | Out-Null
Set-Location "$HOME\projects"
git clone https://github.com/CorsixTH/CorsixTH.git CorsixTH-Vita
Set-Location CorsixTH-Vita
git remote rename origin upstream
git remote add origin https://github.com/VitaPorts/CorsixTH-Vita.git
git remote add legacy https://github.com/MrProcastinator/CorsixTH-vita.git
git fetch upstream --tags
git fetch legacy feature/vita_port
git switch --detach v0.70.1
git switch -c main
$actual = (git rev-parse HEAD).Trim()
$expected = "56bd5d00f76331c7f76d7b696726a7926303ca0c"
if ($LASTEXITCODE -ne 0 -or $actual -ne $expected) { throw "Unexpected tag SHA: $actual. Stop." }
git status --short
git remote -v
git push -u origin main
```

After the first push, import this **overlay** using a development branch, verify it does not overwrite upstream README/licenses, merge `.gitignore.m0-additions` into the existing `.gitignore`, then submit a narrow M0 foundation PR. Do not commit a raw donor ZIP.

### VitaSDK: official native Windows route (issue #3)
Official repository: https://github.com/vitasdk/vdpm ; releases: https://github.com/vitasdk/vdpm/releases . It ships `bootstrap-vitasdk.ps1` and a native Windows vdpm client. Inspect official download, checksum/signature guidance and bootstrap before running:

```powershell
$env:VITASDK_CHANNEL = "2026.08"
Invoke-WebRequest 'https://github.com/vitasdk/vdpm/releases/latest/download/bootstrap-vitasdk.ps1' -OutFile bootstrap-vitasdk.ps1
Get-Content .\bootstrap-vitasdk.ps1 -TotalCount 40
.\bootstrap-vitasdk.ps1
```

Use the resulting install path from the installer; make sure `$env:VITASDK`, the SDK's `bin` directory, and `vdpm`/`arm-vita-eabi-g++` are set up in the active PowerShell environment according to that output. Run `vdpm status` and inspect `vdpm info sdl2`, `vdpm info sdl2_mixer`, `vdpm info luajit`, etc. Install missing packages using native vdpm after checking the exact package names on that installed release. Save terminal output, package versions, and SDK release/channel as reproducibility evidence. Do not claim Windows 10 **CorsixTH build success** before the compiler and Vita have actually been tested.

### First native configure (AFTER #4 probe files exist)

```powershell
cmake -S vita/probe -B build-vita-probe -G Ninja -DCMAKE_TOOLCHAIN_FILE="$env:VITASDK/share/vita.toolchain.cmake" -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-vita-probe --parallel 4
```

Command line and target names must be reconciled with the actual PR #4 CMake implementation. Full `-DVITA=ON` engine build is not available in unmodified upstream 0.70.1 and belongs to #6. Log and retain errors rather than suppressing them.

### CI (independent host; not required on Windows PC)
Linux CI may use `vitasdk/vitasdk:2026.08-20260916.2` as a candidate, but MUST record the immutable digest, compiler/package inventory, and compare relevant target versions with native Windows SDK. If differences exist, pin and resolve them. Do not silently use a rolling tag or mix binaries from incompatible SDK snapshots.

## Physical Vita FAT testing
Director can install a diagnostic VPK via VitaShell and return `ux0:data/corsixth/probe.log` over FTP/USB, plus a photograph of output and the build's Git SHA/VPK checksum. Do not request additional kernel plugins or overclocking without a failing test and explicit approval. Original commercial Theme Hospital data stays outside the public VPK/repository.

## Source/CI distinction
Director may work entirely in native Windows 10. GitHub-hosted Linux CI is an independent build environment, not a local WSL/Docker requirement. For SDK mismatch, compare compiler target triplet and library/package ABI; do not mix host-built and CI libraries. Complete issue #3 inventory before a full game link is treated as meaningful.

## M2.1 engine build attempt

The Vita-only CMake route, exact Windows configure/build commands, dependency
inventory, resource plan, full logs, and first link blocker are recorded in
[M2_1_BUILD_REPORT.md](M2_1_BUILD_REPORT.md). Current status is **BLOCKED** at
ELF link; no engine VPK exists for installation or hardware testing.
The [linker investigation](M2_1_LINKER_INVESTIGATION.md) records actual API
reachability, SDK `libdl` experiments, and a proposed compatibility boundary
awaiting GPT architecture review.

### Verified native Windows host, 2026-10-02
```powershell
$env:VITASDK = "C:\vitasdk"
$env:Path = @("$env:VITASDK\bin", "C:\Program Files\CMake\bin", "$env:LOCALAPPDATA\Microsoft\WinGet\Links", $env:Path) -join ";"
cmake --version
ninja --version
arm-vita-eabi-g++ --version
vdpm status
vdpm list
```
Confirmed: SDK release 2026.08, channel sequence 2026092555, sdk-snapshot-20260825.611.1, GCC 15.2.0, CMake 4.4.3, Ninja 1.13.2. Installed: sdl2 2.32.8-1, sdl2_mixer 2.8.2-1, luajit 2.1.0-1, freetype 2.14.3-1, libpng 1.6.58-1, zlib 1.3.2-2 and declared dependencies. Native Windows package extraction failed to materialize libpng unversioned aliases: `libpng16.a` and `libpng16.pc` existed, and locally copying them to `libpng.a` and `libpng.pc` resolved missing aliases (verified matching file lengths). If rebuilding the SDK after a package upgrade, check the versioned originals and avoid blindly overwriting files. The `libpng-config` alias remains optional and untested.
