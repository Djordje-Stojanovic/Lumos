# ARCHITECTURE.md - Lumos AI (Lean Build Plan)

## Product Goal

Native Windows desktop AI image/video enhancement app with one-time purchase economics.
Core value: fast, high-quality local enhancement without subscription bloat.

## Stack (Unchanged)

- C++20
- Qt 6 (QML + C++)
- CMake 3.24+
- ONNX Runtime 1.17+
- DirectX 12 compute + Vulkan fallback
- FFmpeg 6.x
- SQLite3
- spdlog
- Google Test + Qt Test (via CTest)

## Lean Module Layout

| Layer | Path | Responsibility |
|---|---|---|
| App entry | `src/main.cpp` | App startup and wiring |
| Application logic | `src/app/` | Jobs, settings, presets, project state |
| Processing engine | `src/engine/` | Pipeline, tiling, inference, image/video processing |
| UI | `src/ui/` | QML views and UI-backend bindings |
| Platform integrations | `src/platform/` | Steam, Windows shell, media wrappers |
| Shared utilities | `src/common/` | Logging, config, shared primitives |
| Contracts | `src/contracts/` | Shared interfaces/types (protected path) |
| Tests | `tests/` | Unit/integration coverage |

## Processing Pipeline

`Input -> Decode -> Pre-process -> Tile Split -> AI Inference -> Tile Merge -> Post-process -> Encode -> Output`

## First-Ship Scope (No Bloat)

1. Single image enhancement path
2. Denoise + upscale chain
3. Before/after slider
4. Basic batch queue
5. GPU selection + fallback

Everything else is optional until these are stable and fast.

## Constraints

- UI thread never blocks.
- Heavy compute stays off UI thread.
- No cloud dependency for core processing.
- No subscription lock-in mechanics.
- Avoid speculative abstractions.

## Coordination Rules

1. One active implementation branch at a time.
2. All post-bootstrap changes go via PR.
3. Reviewer validates behavior before merge.
4. If scope grows, split PR before merge.
5. Keep changes reversible and test-backed.

## Performance Targets

- Startup `< 2s`
- Preview load `< 500ms` (up to 50MP)
- Comparison interaction `60 FPS`
- Action-to-progress `< 200ms`
- 12MP 4x upscale on RTX 3060 `< 5s`
