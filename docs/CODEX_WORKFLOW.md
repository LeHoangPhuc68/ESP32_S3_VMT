# Codex Workflow for VMT

## Initial setup

1. Open the repository root in VS Code.
2. Sign in to the official Codex IDE extension with the ChatGPT account.
3. Confirm the repository builds locally:

```bash
pio run -e lilygo
```

4. Keep GitHub authentication configured through GitHub Desktop, Git Credential Manager, or SSH.
5. Review every diff before allowing a commit or push.

Codex can edit files, run commands, and execute builds in the local repository when granted the required workspace and terminal permissions. It must not be given secrets that are unnecessary for the task.

## Standard task prompt

Use this template for implementation work:

```text
Read AGENTS.md and the relevant documents under docs/ before editing.

Task:
<clear task description>

Requirements:
- Preserve existing behavior outside this task.
- Inspect all call sites before changing public APIs.
- Keep service logic out of UI classes.
- Do not add placeholders or TODO-only implementations.
- Run `pio run -e lilygo` after the changes.
- Fix compile errors introduced by the change.
- Review the final diff for unrelated edits.
- Do not commit or push until I approve the diff.

At the end, report:
1. files changed;
2. behavior implemented;
3. build result;
4. remaining hardware tests;
5. risks or follow-up work.
```

## Recommended task size

Give Codex one coherent milestone at a time. Good task boundaries:

- centralize Wi-Fi radio ownership;
- refactor scanner to use the manager;
- add BLE device record and cache;
- add a single screen and its navigation wiring;
- add CI;
- fix one reproduced bug.

Avoid asking for Wi-Fi, BLE, RF, NFC, storage, settings, and UI redesign in one task. Large mixed tasks produce harder-to-review diffs and make regressions more likely.

## Review gate

Before accepting Codex changes:

```bash
git status
git diff --stat
git diff
pio run -e lilygo
```

Check:

- no secrets were added;
- no generated `.pio/` files were added;
- no unrelated formatting churn occurred;
- public API changes have all call sites updated;
- UI does not directly own radio drivers;
- serial errors include useful context;
- hardware-only claims are not presented as verified.

## Commit and push

After reviewing and testing:

```bash
git add <specific files>
git commit -m "type(scope): clear description"
git push origin <branch-name>
```

Recommended branch names:

```text
feat/wifi-manager
feat/ble-scanner
fix/wifi-rescan
refactor/ui-navigation
chore/platformio-ci
```

Recommended policy:

- `main` must remain buildable;
- feature work occurs on a branch;
- merge only after local build and hardware smoke test;
- avoid force-push on shared or published branches.

## Safe autonomy levels

### Level 1 — Read and propose

Codex reads code and provides a plan. No edits.

Use for architecture-sensitive changes.

### Level 2 — Edit and build

Codex edits files and runs PlatformIO. No commit or push.

Use as the default.

### Level 3 — Commit after approval

After the diff is reviewed, explicitly ask Codex to commit with an approved message.

### Level 4 — Push after approval

Only after hardware testing, explicitly ask Codex to push the reviewed branch.

Do not give permanent blanket permission to push every change to `main`.

## Recovery

Before a large change:

```bash
git switch -c backup/pre-major-refactor
git switch -c feat/<task-name>
```

To discard uncommitted Codex changes after review:

```bash
git restore .
git clean -fd
```

Run `git clean -fd` only after checking that no important untracked files will be deleted.

To return to the last committed state:

```bash
git reset --hard HEAD
```

Use destructive commands only when the current work is safely committed or intentionally disposable.
