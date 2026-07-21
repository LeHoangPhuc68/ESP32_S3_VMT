# Contributing to VMT

## Development requirements

- VS Code or another PlatformIO-compatible editor
- PlatformIO
- Git
- LilyGO T-Display S3 for hardware validation

## Build

```bash
pio run -e lilygo
```

## Branching

Create a focused branch from an up-to-date `main`:

```bash
git switch main
git pull --ff-only
git switch -c feat/<short-name>
```

Use prefixes such as:

- `feat/`
- `fix/`
- `refactor/`
- `docs/`
- `chore/`

## Change requirements

- Read `AGENTS.md` and `docs/ARCHITECTURE.md`.
- Keep changes focused.
- Preserve existing behavior outside the task.
- Do not commit credentials or generated build output.
- Build before opening a pull request.
- Include a hardware test checklist for hardware-dependent work.
- Update documentation for architecture or behavior changes.

## Commit messages

Use Conventional Commits:

```text
feat(wifi): add centralized scan ownership
fix(ui): restore back navigation from AP detail
refactor(ble): separate device cache from screen
docs(hardware): document module bus rules
chore(ci): add PlatformIO build workflow
```

## Pull requests

A pull request should state:

- problem or feature goal;
- implementation summary;
- affected files or subsystems;
- build result;
- hardware tests performed;
- remaining limitations or risks.

Do not combine unrelated refactors and features in one pull request.
