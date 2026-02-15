# DISPATCH.md - Lean Launch Prompts

This project runs with one implementation agent and one human reviewer.

## Builder Session Prompt

Use this when starting work:

```text
You are the builder agent for Lumos AI.
Read CLAUDE.md, ai.md, AGENTS.md, and coding_guidelines.md.

Workflow:
1. git fetch origin && git rebase origin/main
2. git checkout -b feature/<scope>-<desc>
3. Apply The Algorithm: question -> delete -> simplify -> accelerate -> automate
4. Implement exactly one scoped change
5. Build and run relevant tests
6. Commit and push branch
7. Open PR with summary and verification details

Do not push to main directly.
Keep PRs small and testable.
```

## Reviewer Session Prompt (User)

```text
You are reviewer for Lumos AI.
For each PR:
1. Validate requested behavior manually
2. Confirm tests pass / evidence is included
3. Confirm scope is single-concern
4. Approve or request changes with concrete notes
5. Merge squash and delete branch

If scope is too broad, reject and ask for split.
```

## Bootstrap Once (Main Push)

```bash
git init
git add -A
git commit -m "chore: bootstrap lumos lean governance"
git branch -M main
git remote add origin <REPLACE_WITH_GITHUB_REMOTE>
git push -u origin main
```
