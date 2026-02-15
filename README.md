# Lumos AI

Windows-native AI enhancement studio built for creators and gamers.
One-time purchase, local processing, premium output quality, no subscription lock-in.

## Get Running in 2 Steps

Prerequisites: Visual Studio 2022 Build Tools (C++ workload), CMake 3.24+, PowerShell.

### 1. Install Qt6 (one-time, about 2 minutes)

```powershell
uvx --from aqtinstall aqt.exe install-qt -O C:\Qt windows desktop 6.10.2 win64_msvc2022_64
```

### 2. Build and launch

```powershell
.\lumos.cmd gui
```

That is it. `lumos.cmd` auto-discovers Qt from `C:\Qt` (also checks `%USERPROFILE%\Qt` and `C:\Program Files\Qt`) and auto-builds when needed.

## Prefer an Explicit 3-Step Flow

```powershell
.\lumos.cmd doctor
.\lumos.cmd build --force
.\lumos.cmd start
```

## All Commands

```powershell
.\lumos.cmd gui             # auto-builds if needed, then launches GUI
.\lumos.cmd start           # launches existing GUI build
.\lumos.cmd build           # incremental GUI build
.\lumos.cmd build --force   # full rebuild
.\lumos.cmd doctor          # toolchain + Qt discovery check
.\lumos.cmd help            # usage reference
```

Command order rule:
- Correct: `.\lumos.cmd build --force`
- Wrong: `.\lumos.cmd --force build`

## Why Lumos

- Steam-first native desktop app feel (C++20 + Qt6)
- Local AI enhancement workflow users can trust
- Fast path to user delight: import -> enhance -> compare -> save
- Revenue model aligned with users: one-time purchase, no forced subscription

## CI Quality Gate

Required workflow: `core-quality-gate` (`.github/workflows/core-quality-gate.yml`)

CI parity command (core-only, no Qt dependency):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLUMOS_BUILD_UI=OFF -DLUMOS_BUILD_TESTS=ON
cmake --build build --config Release -j
ctest --test-dir build -C Release --output-on-failure
```

## Dev Workflow

After bootstrap, every change goes through a feature branch + PR:

1. `git fetch origin && git rebase origin/main`
2. `git checkout -b feature/<scope>-<desc>`
3. Implement one scoped change
4. Build/tests
5. Commit + push branch
6. Open PR
7. User validates/tests/merges
