"""Audit the M2.2 Vita package against its generated resource manifest."""

from pathlib import Path
import hashlib
import re
import sys
import zipfile


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    build = root / (sys.argv[1] if len(sys.argv) > 1 else "build-vita-engine")
    archive = build / "CorsixTH/corsixth_vita_engine.vpk"
    manifest = build / "vita-resource-manifest.txt"
    config_h = build / "CorsixTH/Src/config.h"
    resources = [line for line in manifest.read_text().splitlines()
                 if line and not line.startswith("#")]
    expected = set(resources) | {"eboot.bin", "sce_sys/param.sfo"}
    failures = []
    configured = re.search(r'^#define CORSIX_TH_INTERPRETER_PATH "([^"]+)"',
                           config_h.read_text(), re.MULTILINE)
    if not configured or configured.group(1) != "app0:/CorsixTH.lua":
        failures.append("compiled interpreter path is not app0:/CorsixTH.lua")
    for source, slice_call in (
        ("CorsixTH/CorsixTH.lua", 'source:sub(2, -13)'),
        ("CorsixTH/Lua/config_finder.lua", 'source:sub(2, -22)'),
        ("CorsixTH/Lua/app.lua", 'source:sub(2, -12)'),
    ):
        if slice_call not in (root / source).read_text(encoding="utf-8"):
            failures.append(f"upstream source-path parsing changed: {source}")

    with zipfile.ZipFile(archive) as package:
        names = package.namelist()
        actual = set(names)
        if len(names) != len(actual):
            failures.append("duplicate ZIP entries")
        if actual != expected:
            failures.append(f"missing={sorted(expected - actual)} extra={sorted(actual - expected)}")
        bad_crc = package.testzip()
        if bad_crc:
            failures.append(f"bad CRC: {bad_crc}")
        for name in resources:
            if name not in actual:
                continue
            if name == "LICENSE.txt":
                source = root / name
            elif name == "config.path.txt":
                source = root / "vita/resources/config.path.txt"
            else:
                source = root / "CorsixTH" / name
            if hashlib.sha256(package.read(name)).digest() != hashlib.sha256(source.read_bytes()).digest():
                failures.append(f"resource bytes differ: {name}")
        if package.read("config.path.txt") not in (
            b"ux0:data/corsixth/user\n", b"ux0:data/corsixth/user\r\n"
        ):
            failures.append("config.path.txt is not the expected writable directory")
        if b"CORSXP003" not in package.read("sce_sys/param.sfo"):
            failures.append("development title ID missing from SFO")
        for prefix in ("HOSP/", "Data/", "DATA/", "QData/", "QDATA/", "ANIMS/"):
            if any(name.startswith(prefix) for name in names):
                failures.append(f"commercial data prefix present: {prefix}")

        # Mirror the three upstream filename slices against the Vita loadfile paths.
        script = "@app0:/CorsixTH.lua"
        finder = "@app0:/Lua/config_finder.lua"
        app = "@app0:/Lua/app.lua"
        if (script[1:-12], finder[1:-21], app[1:-11]) != ("app0:/",) * 3:
            failures.append("upstream Lua source-path slices do not resolve to app0:/")

        print(f"archive={archive}")
        print(f"entries={len(names)} unique={len(actual)} resources={len(resources)}")
        print(f"compiled_interpreter={configured.group(1) if configured else 'missing'}")
        print("config_redirect=ux0:data/corsixth/user")
        print(f"crc={'PASS' if not bad_crc else 'FAIL'} sha256_resources={'PASS' if not failures else 'CHECK'}")
        for name in sorted(names):
            print(name)
    for failure in failures:
        print(f"FAIL {failure}", file=sys.stderr)
    print(f"audit={'PASS' if not failures else 'FAIL'}")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
