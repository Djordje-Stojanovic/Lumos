# Lumos AI

Lumos AI is a local-first Windows desktop app for AI image and video enhancement. It is sold as a one-time purchase on Steam and Microsoft Store, with no subscription requirement and no cloud dependency for core processing.

## Tech Stack

- Language: C++20
- UI: Qt 6 (QML + C++)
- Build: CMake 3.24+
- Inference: ONNX Runtime 1.17+
- GPU: DirectX 12 compute (primary), Vulkan compute (fallback)
- Media I/O: FFmpeg 6.x
- Storage: SQLite3
- Logging: spdlog
- Tests: Google Test + Qt Test (run via CTest)

## Operating Model

One coding agent (Codex) implements changes. The user sets priorities, validates behavior, and approves merges.

## Module Ownership

| Module | Owner | Code Directory | Test Directory | Responsibility |
|---|---|---|---|---|
| `core` | `codex` | `src/` | `tests/` | End-to-end app implementation across UI, engine, and platform integrations |

Shared contracts path: `src/contracts/**` (protected; deliberate changes only).

## The Algorithm (Mandatory Decision Sequence)

1. Question the requirement.
2. Delete unnecessary parts.
3. Simplify what remains.
4. Accelerate only validated bottlenecks.
5. Automate only stable workflows.

## Git Workflow

All work after bootstrap happens on feature branches and pull requests.

### Bootstrap Once (allowed direct push to main)

1. `git init`
2. `git add -A`
3. `git commit -m "chore: bootstrap lumos governance and workflow"`
4. `git branch -M main`
5. `git remote add origin <REPLACE_WITH_GITHUB_REMOTE>`
6. `git push -u origin main`

### Normal Change Loop (every change after bootstrap)

1. `git fetch origin && git rebase origin/main`
2. `git checkout -b feature/<scope>-<short-desc>`
3. Implement only scoped change
4. Run tests relevant to changed scope
5. `git add -A && git commit -m "<type>(<scope>): <what and why>"`
6. `git push origin feature/<scope>-<short-desc>`
7. `gh pr create --title "<type>(<scope>): <desc>" --body "<what changed and why>"`
8. User validates and merges (`--squash --delete-branch`)
9. Sync main and start next small branch

## Verification Commands

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
ctest --test-dir build --output-on-failure
```

## Performance Targets

- App cold start to usable UI: `< 2s`
- Image load to preview (up to 50MP): `< 500ms`
- Before/after slider: `60 FPS`
- Enhance click to visible progress: `< 200ms`
- 12MP image upscale 4x on RTX 3060: `< 5s`

## Guardrails

- No subscription logic in base product.
- No cloud dependency for core enhancement.
- No speculative architecture layers without measured need.
- No large multi-concern PRs.
