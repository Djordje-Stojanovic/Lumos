# coding_guidelines.md

Mandatory engineering standard for Lumos AI (C++20 + Qt 6 + CMake + ONNX Runtime + DirectX 12/Vulkan fallback).

This file is the style and quality source of truth for all agents and contributors.

## Product North Star

Build the best-feeling local AI enhancement desktop app in its category.
Quality means:

1. Beautiful, premium UI feel
2. Fast and smooth interactions
3. Reliable output quality
4. Maintainable, readable code with low bloat

## Mandatory Decision Algorithm (Elon Principles)

Apply this sequence before and during every task:

1. Question the requirement.
2. Delete unnecessary parts/processes.
3. Simplify what remains.
4. Accelerate only validated bottlenecks.
5. Automate only stable workflows.

If a change fails this sequence, do not ship it.

## Priority Order (What Matters Most)

1. User-perceived quality: UI feel, clarity, beauty, responsiveness.
2. Correctness and safety: no silent failure, no data loss, no broken core flow.
3. Performance: meet targets without premature complexity.
4. Maintainability: readable, modular, low-coupling code.
5. Tests and automation: enough to protect behavior and speed iteration.

Tests are mandatory for behavior risk, but test count is never the goal.
The goal is a best-in-class user experience with confidence to ship fast.

## UI/UX Quality Bar (2026 Standard)

1. Every screen must feel intentional, not generic.
2. Interaction response must feel immediate (show progress or feedback within 200ms).
3. Avoid UI jitter, layout jumps, and blocking operations.
4. Keep the primary flow obvious: import -> enhance -> compare -> save.
5. Prefer progressive disclosure: simple defaults, advanced controls behind clear toggles.
6. Empty/loading/error/success states are required for every critical flow.
7. Keyboard shortcuts, focus behavior, and accessibility labels must be considered in UI work.
8. Preserve visual consistency:
   - shared spacing scale
   - shared typography scale
   - shared color tokens
   - consistent control states (hover/active/disabled/loading)
9. Before/after interactions must feel premium (smooth slider movement, stable zoom/pan behavior).
10. Never ship UI that is technically correct but visually rough or confusing.

## Stack-Specific Engineering Rules

### C++20 and Architecture

1. Keep modules small and responsibility-focused.
2. Use clear interfaces at boundaries (`src/contracts/**` deliberate changes only).
3. No speculative abstraction layers.
4. Prefer explicit data flow over hidden global state.
5. Keep engine logic independent from UI where practical.
6. Use RAII and deterministic cleanup; avoid manual lifetime traps.
7. Prefer simple value types and clear ownership semantics.
8. Do not let exceptions leak across major module boundaries.

### Qt 6 (QML + C++)

1. Keep business logic in C++; keep QML declarative and presentation-focused.
2. Avoid heavy JavaScript logic in QML.
3. Prevent binding loops and runaway updates.
4. Never block the UI thread with file I/O, inference, encode/decode, or heavy compute.
5. Use async worker patterns with clear progress and cancellation paths.
6. Reuse components and design tokens; do not duplicate near-identical UI blocks.

### Performance and Efficiency

1. Measure before optimizing.
2. Optimize user-facing bottlenecks first.
3. Respect performance targets in `CLAUDE.md` and architecture docs.
4. Keep memory behavior predictable; avoid unnecessary copies in hot paths.
5. If complexity increase is proposed, provide measurable benefit.

## Readability and Maintainability Conventions

1. Prefer boring, obvious code over clever code.
2. Keep functions small; split when intent becomes unclear.
3. Use names that reveal domain intent, not implementation trivia.
4. Comments explain why/constraints, not obvious what.
5. Remove dead code instead of parking it behind flags without clear use.
6. Minimize dependencies per module.
7. One change per branch, one concern per PR.

## Git and PR Workflow

1. Sync first: `git fetch origin && git rebase origin/main`
2. Branch per scoped change: `git checkout -b feature/<scope>-<desc>`
3. Commit format: `<type>(<scope>): <what and why>`
4. Allowed types: `feat`, `fix`, `refactor`, `chore`, `docs`
5. Push feature branch and open PR
6. Never push directly to `main` after bootstrap

## Verification Standard

Run the relevant checks for changed scope:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
ctest --test-dir build --output-on-failure
```

For UI-heavy changes, include visual/interaction validation notes in PR:

1. What was improved in feel/clarity
2. Which states were checked (empty/loading/error/success)
3. Evidence that responsiveness did not regress

If a command cannot run locally, document exact reason in PR.

## Review Gate (Must Pass)

1. Scope is single-concern and high leverage.
2. Change improves user-perceived quality, correctness, or meaningful speed.
3. Code is simpler or no more complex than before.
4. No obvious UI roughness or confusing behavior.
5. Verification evidence is present and honest.
