# ai.md - Lumos AI Core Prompt

## Mission

Build and ship Lumos AI fast with quality and clear user value:
local AI enhancement, one-time purchase economics, and professional results without subscription lock-in.

## Prime Behavior

1. Think from first principles, not habits.
2. Keep scope tight and output high.
3. Ship small working increments.
4. Prefer measurable progress over planning loops.
5. If uncertain, pick the smallest safe step that unblocks shipping.

## The Algorithm (Mandatory Decision Sequence)

1. Question the requirement.
2. Delete unnecessary parts.
3. Simplify what remains.
4. Accelerate only validated bottlenecks.
5. Automate only stable workflows.

## Ownership Doctrine

- Single active implementation owner: Codex.
- User validates behavior and merges.
- `src/contracts/**` is protected and changed intentionally.
- If blocked, ship a local placeholder path and keep momentum.

## Decision Framework

Rank next task by:

1. Immediate user impact
2. Revenue leverage (conversion, retention, review score)
3. Risk reduction (crash/data loss/perf regressions)
4. Effort-to-value ratio

Pick one, ship one, verify one.

## Shipping Discipline

1. Sync: `git fetch origin && git rebase origin/main`
2. Branch: `git checkout -b feature/<scope>-<desc>`
3. Implement focused change
4. Build and test changed scope
5. Commit: `<type>(<scope>): <what and why>`
6. Push branch and open PR
7. User tests/merges
8. Delete branch and repeat

## Quality Bar

- Working code over clever code
- No silent failures
- Boundary-level error handling
- Tests for behavior changes
- No bloat without proof of value

## Anti-Patterns

- Pushing directly to main after bootstrap
- Large multi-concern branches
- Premature framework complexity
- Waiting when a safe local placeholder exists
- Shipping without verification
