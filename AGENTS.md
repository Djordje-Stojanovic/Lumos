# AGENTS.md - Single-Agent Workflow

This repo runs a single coding-agent workflow with direct user validation.

## Prime Directive

Ship product progress fast in small safe increments.
After initial bootstrap push, every code change must go through a feature branch + PR.

## Mandatory Standards (Non-Negotiable)

1. `coding_guidelines.md` is the style, quality, and architecture source of truth.
2. Every task must follow `coding_guidelines.md` conventions for UI quality, maintainability, readability, and low-bloat implementation.
3. Elon principles are mandatory on every task:
   - Question the requirement
   - Delete unnecessary parts/processes
   - Simplify what remains
   - Accelerate only validated bottlenecks
   - Automate only stable workflows
4. If there is a conflict between habits and `coding_guidelines.md`, follow `coding_guidelines.md`.

## The Algorithm (Run Before Every Task)

1. Question the requirement.
2. Delete unnecessary parts.
3. Simplify what remains.
4. Accelerate only validated bottlenecks.
5. Automate only stable workflows.

## Ownership Rules

1. Codex edits implementation and tests.
2. User validates behavior and merges.
3. `src/contracts/**` is deliberate-change-only.
4. One change per branch. One concern per PR.
5. Never push directly to `main` after bootstrap.

## Directory Scope

- Code: `src/`
- Tests: `tests/`
- Build/config when needed: `cmake/`, `tools/`

## Session Workflow

### Start

1. Read and apply `coding_guidelines.md`
2. Sync: `git fetch origin && git rebase origin/main`
3. Branch: `git checkout -b feature/<scope>-<short-desc>`

### Work

- Implement only the scoped change.
- Follow `coding_guidelines.md` style and quality bar during implementation.
- Prioritize premium UI feel and clarity while keeping code simple and maintainable.
- Run relevant build/tests.
- Keep commits surgical.

### Finish

1. `git add -A && git commit -m "<type>(<scope>): <what and why>"`
2. `git push origin feature/<scope>-<short-desc>`
3. `gh pr create --title "<type>(<scope>): <desc>" --body "<what changed and why>"`
4. User tests and either merges or requests changes.

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
- Ignoring `coding_guidelines.md` or the mandatory Elon principles
