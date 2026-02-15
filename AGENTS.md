# AGENTS.md - Lean Ownership Workflow

This repo runs a low-overhead two-person flow:

- `builder` (agent): implements code changes
- `reviewer` (user): tests behavior, reviews PRs, merges, and validates scope

No coordination theater. No idle loops. No bloated process.

## Prime Directive

Ship product progress fast in small safe increments.
After initial bootstrap push, every code change must go through a feature branch + PR.

## The Algorithm (Run Before Every Task)

1. Question the requirement.
2. Delete unnecessary parts.
3. Simplify what remains.
4. Accelerate only validated bottlenecks.
5. Automate only stable workflows.

## Ownership Rules

1. `builder` edits implementation and tests.
2. `reviewer` validates behavior and merges.
3. `src/contracts/**` is deliberate-change-only.
4. One change per branch. One concern per PR.
5. Never push directly to `main` after bootstrap.

## Directory Scope

- Code: `src/`
- Tests: `tests/`
- Build/config when needed: `CMakeLists.txt`, `cmake/`, `tools/`

## Session Workflow

### Start

1. Sync: `git fetch origin && git rebase origin/main`
2. Branch: `git checkout -b feature/<scope>-<short-desc>`
3. Log start in `codex_agents_logs.md`:

```text
START | <timestamp+tz> | builder | task: <one-line summary>
SCOPE | <file paths>
BRANCH | feature/<scope>-<short-desc>
```

### Work

- Implement only the scoped change.
- Run relevant build/tests.
- Keep commits surgical.

### Finish

1. `git add -A && git commit -m "<type>(<scope>): <what and why>"`
2. `git push origin feature/<scope>-<short-desc>`
3. `gh pr create --title "<type>(<scope>): <desc>" --body "<what changed and why>"`
4. Reviewer tests and either approves/merges or requests changes.
5. Log finish in `codex_agents_logs.md`.

## Commit Types

- `feat`: new functionality
- `fix`: bug fix
- `refactor`: structural improvement without behavior change
- `chore`: tooling/config/process
- `docs`: documentation only

## Non-Negotiable Anti-Patterns

- Pushing directly to main
- Multi-feature PRs
- Big refactors without isolated value
- Skipping tests for behavior changes
- Adding complexity without measured benefit
