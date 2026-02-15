# Lumos AI

Lumos AI is a local-first Windows desktop application for AI image and video enhancement, sold as a one-time purchase on Steam and Microsoft Store.

## Product Position

- No subscription lock-in
- No cloud dependency for core enhancement
- Fast, high-quality enhancement on consumer GPUs

## Tech Stack

- C++20
- Qt 6 (QML + C++)
- CMake 3.24+
- ONNX Runtime 1.17+
- DirectX 12 compute (+ Vulkan fallback)
- FFmpeg 6.x
- SQLite3
- spdlog
- Google Test + Qt Test (via CTest)

## Repository Layout

- `src/` application code
- `tests/` unit/integration tests
- `models/` AI model assets
- `shaders/` GPU shaders
- `resources/` static assets
- `docs/` design and architecture notes

## Local Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
ctest --test-dir build --output-on-failure
```

## Windows Launcher Workflow

Use the shareable launcher interface from repo root:

```powershell
.\lumos.cmd doctor
.\lumos.cmd build all --force
.\lumos.cmd build gui --force
.\lumos.cmd build cli --force
.\lumos.cmd
.\lumos.cmd gui
.\lumos.cmd cli --json --no-persist
```

Command-order rule:

- Correct: `.\lumos.cmd build all --force`
- Wrong: `.\lumos.cmd --force build`

Launcher implementation files:

- `lumos.cmd` thin wrapper entrypoint
- `installer/windows/lumos_launcher.ps1` command dispatcher
- `installer/windows/build_shell_native.ps1` GUI build + dist staging
- `installer/windows/build_platform_native.ps1` CLI build + dist staging
- `installer/windows/build_native.ps1` shared native build logic

## Development Workflow

Bootstrap once (allowed direct push to main):

```bash
git init
git add -A
git commit -m "chore: bootstrap lumos lean governance"
git branch -M main
git remote add origin <REPLACE_WITH_GITHUB_REMOTE>
git push -u origin main
```

Then for every change:

1. `git fetch origin && git rebase origin/main`
2. `git checkout -b feature/<scope>-<desc>`
3. Implement one scoped change
4. Build/tests
5. Commit + push branch
6. Open PR
7. User validates/tests/merges
8. Delete branch

## Core Rule

After bootstrap, no direct pushes to `main`.
