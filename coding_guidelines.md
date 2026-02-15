# coding_guidelines.md

Mandatory checklist for each coding session.

## Read Order

1. `CLAUDE.md`
2. `ai.md`
3. `AGENTS.md`
4. `lumos-ai-plan.md`

## Session Rules

1. Sync first: `git fetch origin && git rebase origin/main`
2. Branch per change: `git checkout -b feature/<scope>-<desc>`
3. Keep scope to one concern
4. Prefer deletion/simplification before adding complexity
5. Never push to `main` after bootstrap

## Commit Rules

1. One concern per commit
2. Message format: `<type>(<scope>): <what and why>`
3. Types: `feat`, `fix`, `refactor`, `chore`, `docs`
4. Push feature branch
5. Open PR with clear verification notes

## Verification Rules

Run what is relevant for the changed scope:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
ctest --test-dir build --output-on-failure
```

If a command cannot run locally, document exact reason in PR.

## Reviewer Checklist

1. Scope matches PR title/description
2. Behavior matches request
3. Verification evidence is present
4. No obvious regressions introduced
5. PR stays single-concern
