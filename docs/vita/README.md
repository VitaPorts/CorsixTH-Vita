# CorsixTH-Vita: M0 foundation overlay

This directory documents the Vita-specific adaptation of the official CorsixTH `v0.70.1` engine. The original upstream root `README.md`, license and Git history remain intact.

This project ports the open-source CorsixTH engine to PlayStation Vita. It does not include original commercial Theme Hospital game assets. Initial target: v0.70.1 using the native SDL2 Vita GXM backend. Upstream upgrades are deliberate maintenance work, not blind merges.

Start here:
- [Project plan](PROJECT_PLAN.md)
- [Team workflow and authority](TEAM_WORKFLOW.md)
- [Current status](PORTING_STATUS.md)
- [Native Windows build environment](BUILDING.md)
- [M1A probe UI and acceptance contract](M1A_PROBE_UI_SPEC.md)
- [Hardware testing](HARDWARE_TESTING.md)
- [Upstream synchronization](UPSTREAM_SYNC.md)

The Director workstation route is native Windows 10 64-bit without a WSL2 or Docker requirement. The native Windows SDK/dependency inventory has been verified; actual Vita compile/link and physical-console functionality are separate later gates.
