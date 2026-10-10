# Architecture decision log

| ADR | Decision | Reason | Revisit condition |
|---|---|---|---|
| ADR-001 | Start official v0.70.1, preserve Git ancestry; independent VitaPorts repo | 2023 donor is stale and upstream-tracking must remain easy | Never substitute old ZIP as upstream engine |
| ADR-002 | Native SDL2 GXM first, not vitaGL | CorsixTH uses SDL_Renderer; Vita SDL2 advertises TARGETTEXTURE | If reduced M1A hardware test fails after diagnosis |
| ADR-003 | Isolate Vita-specific code under `vita/`, track upstream deltas | Contain future merge conflicts | If platform seams demand a targeted, documented upstream edit |
| ADR-004 | Two early diagnostic VPK stages | Separate graphics from Lua/dependency failures | After both gates pass |
| ADR-005 | Native Windows 10 VitaSDK bootstrap for director; independently pinned, digest-identified VitaSDK in Linux CI | No Linux requirement on developer PC, reproducible CI and matching target SDK/library ABI | Update only via separate SDK change issue and checks in #3 |
| ADR-006 | `main` follows verified stable CorsixTH release tags, not rolling `master` | Upstream SDL2->SDL3 is a major platform risk | 0.71 migration branch after readiness checks |
| ADR-007 | Read-only app0 versus writable ux0 and external commercial data | Vita filesystem semantics, license and safe updates | Installation evidence suggests more precise sublayout |
| ADR-008 | GPT plans/reviews, Codex implements, director tests Vita FAT | Independent verification and real hardware gates | Revise only if testing infrastructure grows |
