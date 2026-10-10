# Team workflow and authority

This document defines how the CorsixTH-Vita studio operates. GitHub issues and pull requests are the source of truth; chat instructions are converted into repository work before implementation when practical.

## Roles

### Director
Product owner, release authority and physical-hardware acceptance owner.

Responsibilities:
- Choose product scope, priorities and risk tolerance.
- Maintain the native Windows development machine, VitaSDK installation and private credentials.
- Own the physical PS Vita FAT, VitaShell, original legally owned Theme Hospital data and any device-specific configuration.
- Install test VPKs and return requested photos, logs, crash codes and observed behavior.
- Give final go/no-go for hardware gates, merges that change product behavior, release tags and public beta distribution.
- Never be asked to pretend a hardware test passed when it was not actually run.

The Director does not need to perform routine GitHub issue/branch/PR administration when GPT can do it through the repository connector.

### GPT
Technical lead, repository coordinator and independent reviewer.

Responsibilities:
- Maintain architecture, issue ordering, acceptance gates, documentation and upstream-sync policy.
- Use GitHub directly for routine repository work when authorized: inspect commits, create/update issues, create branches, open draft PRs, review diffs, record evidence and merge only after the appropriate Director authorization.
- Turn approved milestones into narrow Codex briefs with explicit scope, exclusions and acceptance criteria.
- Review Codex changes against upstream compatibility, dependency provenance, Vita constraints and the issue contract.
- Triage compiler, CI and device logs and decide the next engineering experiment.
- Keep BUILD PASS, DEVICE PASS and ACCEPTED distinct.

Limits:
- GPT cannot physically test a Vita and must never invent a hardware result.
- GPT review is technical/advisory and is not a second authenticated human GitHub reviewer.
- Architecture changes that materially alter scope or release risk return to the Director for approval.

### Codex
Implementation engineer.

Responsibilities:
- Work one approved issue at a time on the named implementation branch.
- Read AGENTS.md and the issue/Codex handoff before changing files.
- Implement the smallest change that satisfies the issue.
- Run the available configure/build/tests and preserve exact errors, warnings and commands.
- Produce inspectable artifacts where required: VPK, symbol-bearing ELF, hashes, manifests and logs.
- Update UPSTREAM_DELTA.md whenever official CorsixTH-owned source files are changed.
- Push implementation work for GPT review.

Limits:
- Codex does not redefine architecture, merge its own PR, declare a hardware PASS, change upstream baseline, add new runtime/plugin dependencies or expand issue scope without review.

## Normal issue lifecycle

1. **Director / GPT choose next gate.** GPT confirms prerequisites and keeps the issue body current.
2. **GPT prepares implementation contract.** Branch name, files, exclusions, test IDs and evidence are explicit.
3. **Codex implements.** One narrow branch and PR; exact build output is retained.
4. **GPT reviews.** Diff, upstream delta, packaging, dependency changes and build evidence are checked. Corrections go back to Codex.
5. **Director tests hardware when required.** VPK is installed on the Vita; Director returns photo/log/result.
6. **GPT records gate result.** PASS, FAIL, BLOCKED or NOT RUN is written to GitHub with the relevant commit/artifact identity.
7. **Director authorizes integration/release where required.** GPT may execute the mechanical GitHub merge after authorization.
8. **Next issue starts only when its prerequisite gate is satisfied or the Director explicitly accepts the risk.**

## Branch convention

- `main`: reviewed integration branch and last accepted baseline.
- `setup/<topic>`: repository/foundation work.
- `codex/<milestone>-<topic>`: Codex implementation work.
- `upgrade/<upstream-tag>`: official CorsixTH release intake.
- `backport/<upstream-issue>`: targeted upstream fix.
- `hotfix/<topic>`: urgent fix against the current accepted release.

Never develop directly on `main`.

## Evidence language

- **BUILD PASS**: configure/compile/link/package succeeded for the identified commit and environment.
- **DEVICE PASS**: the identified artifact was actually run on the target Vita and the required observation matched.
- **ACCEPTED**: the Director/technical gate has approved the result for integration.
- **BLOCKED**: prerequisite or environmental failure prevents a valid test.
- **NOT RUN**: no claim is made.

For M1A, Codex owns BUILD PASS, Director owns the physical observation, and GPT correlates both into the G2 decision.
