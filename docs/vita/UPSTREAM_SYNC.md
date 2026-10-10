# Permanent upstream synchronization policy

## Principles
- Preserve actual Git ancestry from `CorsixTH/CorsixTH`; an independent GitHub repo can still share upstream commits.
- Initial `main` is the untouched imported official v0.70.1 baseline; after Vita bring-up, `main` tracks the last reviewed Vita working build. It is never a mirror of upstream `master`.
- `origin` points to VitaPorts, `upstream` to official CorsixTH, optional `legacy` is read-only. Remote settings are local to a clone; document them, not assume they exist on GitHub.
- Pin initial baseline to annotated `v0.70.1` peeled commit `56bd5d00f76331c7f76d7b696726a7926303ca0c`; verify with `git rev-parse 'v0.70.1^{}'`.
- Vita release tags use `vita-v<upstream>-<stage>.<n>` and cannot conflict with upstream tags. Record merged upstream tag and SDK image digest per release.
- Upstream security/correctness fixes can be cherry-picked with an issue and original commit reference. No blind `master` pulls.

## One-time bootstrap: run only while origin has no commits
```bash
git clone https://github.com/CorsixTH/CorsixTH.git CorsixTH-Vita
cd CorsixTH-Vita
git remote rename origin upstream
git remote add origin https://github.com/VitaPorts/CorsixTH-Vita.git
git remote add legacy https://github.com/MrProcastinator/CorsixTH-vita.git
git fetch upstream --tags
git fetch legacy feature/vita_port
git switch --detach v0.70.1
git switch -c main
test "$(git rev-parse HEAD)" = "56bd5d00f76331c7f76d7b696726a7926303ca0c" || { echo 'Unexpected v0.70.1 tag target'; exit 1; }
git push -u origin main
```
If the remote acquires commits before bootstrap: STOP and review ancestry rather than force-pushing or creating an orphan merge.

Windows 10 PowerShell equivalents of these bootstrap commands and native VitaSDK setup are maintained in [`BUILDING.md`](BUILDING.md). Git remotes are local clone settings, not GitHub server configuration.

## New release process (e.g. v0.70.2)
```bash
git switch main
git pull --ff-only origin main
git fetch upstream --tags
git switch -c upgrade/v0.70.2
git merge --no-ff v0.70.2
```
Resolve conflicts individually. Record `git range-diff`/`git diff` between previous and new upstream tags, changes in build flags, Lua API, SDL renderer, audio, asset paths and save format. Update `UPSTREAM_DELTA.md` whenever a Vita integration seam moves. Run desktop baseline -> cross compile -> probe if SDL/Lua changed -> on-device smoke + save/load -> PR review. Merge only after gate sign-off.

## SDL3 migration caution
Official master at the time of planning uses SDL3 and SDL3_mixer; latest released v0.70.1 uses SDL2. Do not switch main to SDL3 until exact VitaSDK SDL3 and mixer link/runtime requirements are verified through independent probes. Keep the last functional release branch during major migrations.

## Backport policy
Bugfix backports: create `backport/<upstream-issue>`, `git cherry-pick -x <sha>` after verifying applicability. Document reason for divergence. Prefer merged upstream fixes for future versions; avoid local duplicate implementations.
