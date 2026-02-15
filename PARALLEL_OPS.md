# PARALLEL_OPS.md - Minimal Ops

## Model

This repo intentionally avoids multi-agent overhead for now:

- One active implementation owner (`builder`)
- One reviewer/merger (`user`)

Result: lower coordination cost, faster execution.

## Why This Is Fast

1. No ownership collisions
2. One change per branch
3. Quick reviewer feedback loop
4. Immediate merge-or-fix decisions

## Standard Change Loop

```bash
git fetch origin
git rebase origin/main
git checkout -b feature/<scope>-<short-desc>
# implement
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
ctest --test-dir build --output-on-failure
git add -A
git commit -m "<type>(<scope>): <what and why>"
git push origin feature/<scope>-<short-desc>
gh pr create --title "<type>(<scope>): <desc>" --body "<what and why + verification>"
```

## Reviewer Merge Loop

```bash
gh pr list
gh pr diff <number>
gh pr review <number> --approve
gh pr merge <number> --squash --delete-branch
```

## After Merge

```bash
git checkout main
git pull origin main
```

## Guardrails

- Never push directly to main after bootstrap.
- Never bundle multiple unrelated changes in one PR.
- If tests fail, do not open PR.
