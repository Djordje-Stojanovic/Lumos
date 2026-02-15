# LUMOS AI — Complete Technical Architecture & Execution Plan

## Context

This document is the complete build specification for a local AI image and video enhancement desktop application sold as a one-time purchase on Steam and Microsoft Store. The working product name is "Lumos AI." The execution model is lean: one owner-operator user plus one coding agent. The target is $1M gross revenue in year 1, scaling to $100M cumulative revenue by year 5–7.

The opportunity: Topaz Labs proved a small team (~60 people) can build a $48M-valued business in local AI enhancement. In September 2025, Topaz switched from $299 one-time to $25–58/month subscriptions, triggering massive backlash. No AI enhancement tool exists on Steam, where 132–147M monthly active users own powerful GPUs. This is the gap.

---

## 1. Product Definition

**Product:** Local AI image and video enhancement tool for Windows.

**Core value proposition:** Professional AI image and video enhancement. One price. No subscription. No cloud. Yours forever.

**Positioning:** Anti-subscription alternative to Topaz Labs, sold on Steam and Microsoft Store. Same playbook Affinity used against Adobe — position as the consumer-friendly one-time-purchase option against the incumbent's subscription model.

**Target personas:**

- PC Gamer: Screenshots, texture mods, wallpaper enhancement. No easy one-click tool on Steam. Willingness to pay: $20–30.
- Hobbyist Photographer: Weekend shooter wanting better photos without Lightroom. Subscription fatigue from Adobe/Topaz. Willingness to pay: $30–50.
- Content Creator: YouTuber/streamer needing quick video enhancement. Topaz too expensive monthly. Willingness to pay: $30–50.
- Nostalgia Restorer: Old family photos, VHS digitization. Technical barrier to open-source tools. Willingness to pay: $20–30.
- Digital Artist: AI upscaling for artwork, texture creation. Existing tools lack Steam Workshop integration. Willingness to pay: $25–40.

**Pricing:**

- Base product: $29.99 on Steam and Microsoft Store.
- Complete Bundle (base + all DLC): $49.99.
- Launch discount: 10–15% for 7–14 days.
- Never discount below 40% in year 1. Stair-step discounts over 18+ months.

**Revenue targets:**

- Year 1: $1M gross (~33,400 units at $29.99).
- Year 2: $5–15M gross.
- Year 3–4: $10–25M/yr gross.
- Year 5–7: $100M cumulative gross achievable. At $29.99 base with ~$10 average DLC attach, this requires approximately 2.5M total customers over 5–7 years.

---

## 2. Technology Stack

**Decision: C++ with Qt 6 (LGPL).**

Single language. Single stack. Every successful desktop media app (DaVinci Resolve, OBS, Blender, FL Studio, Wallpaper Engine) uses C/C++. Qt 6 under LGPL is free for commercial use with dynamic linking.

**Why not Rust:** A 2025 survey of 43 Rust GUI libraries found 94.4% are not production-ready. Egui lacks native styling and accessibility. Iced is pre-1.0 with frequent breaking changes. Druid is abandoned. Slint is still adding basic desktop features like modal dialogs.

**Why not Python:** 10–100x slower CPU performance, GIL threading limitations, 50–150MB distribution bundles, unprofessional packaging quality.

**Why not Electron/Tauri:** Web-layer rendering cannot efficiently drive GPU compute pipelines. Copying pixel buffers between a web renderer and GPU compute is a performance-killing bottleneck.

### Core technology components

| Component | Technology | License | Purpose |
|---|---|---|---|
| Language | C++20 | N/A | Core application language |
| UI Framework | Qt 6 (Qt Quick/QML + Widgets) | LGPL v3 (dynamic linking) | Full desktop GUI with GPU-accelerated rendering |
| Build System | CMake 3.24+ | Open source | Cross-platform build orchestration |
| AI Inference | ONNX Runtime 1.17+ | MIT | Run AI models on any GPU vendor |
| GPU Compute | DirectX 12 Compute Shaders (primary) + Vulkan (fallback) | N/A | Image/video processing pipeline |
| Media I/O | FFmpeg 6.x (libavcodec, libavformat) | LGPL v2.1+ | Video decode/encode, format support |
| Image I/O | stb_image + libpng + libjpeg-turbo + OpenEXR | Various permissive | Image format read/write |
| Steam SDK | Steamworks SDK | Valve proprietary | Workshop, achievements, overlay, DRM |
| Packaging | Qt Installer Framework + Steam depot | LGPL | Windows installer and Steam distribution |
| Testing | Google Test + Qt Test | BSD-3 / LGPL | Unit and integration testing |
| Logging | spdlog 1.13+ | MIT | Structured logging |
| Local DB | SQLite3 3.44+ | Public domain | Enhancement history, recent files, settings |
| Crash reporting | Crashpad (Google) + Sentry/BugSplat | Apache 2.0 | Field crash collection and analysis |

### Dependency management

Use vcpkg for all C++ dependencies except Qt (installed via Qt Online Installer).

| Dependency | vcpkg Package | Version | Link Type |
|---|---|---|---|
| Qt 6 | System install (Qt Online Installer) | 6.6+ | Dynamic (LGPL requirement) |
| ONNX Runtime | onnxruntime-gpu | 1.17+ | Dynamic |
| FFmpeg | ffmpeg[x264,x265,vpx,aom] | 6.x | Dynamic (LGPL) |
| stb_image | stb | Latest | Header-only |
| libpng | libpng | 1.6+ | Static |
| libjpeg-turbo | libjpeg-turbo | 3.0+ | Static |
| SQLite3 | sqlite3 | 3.44+ | Static |
| spdlog | spdlog | 1.13+ | Header-only |
| Google Test | gtest | 1.14+ | Static (test builds only) |

---

## 3. System Architecture

The application uses a 5-layer architecture with strict separation between UI, business logic, and GPU processing. All heavy computation runs on background threads. The UI thread never blocks.

### Layer 1: Presentation (Qt Quick/QML)

The entire UI is built in QML with a C++ backend. QML provides GPU-accelerated rendering, smooth animations, and a modern look. The UI communicates with the processing engine via Qt signals/slots and a shared job queue.

Components:

- **MainWindow.qml**: Top-level shell with sidebar navigation, toolbar, and central content area.
- **EnhanceView.qml**: Primary workspace. Drag-and-drop import, before/after slider, enhancement controls, one-click enhance button.
- **BatchView.qml**: Multi-file queue with progress bars, thumbnail previews, and batch settings.
- **CompareView.qml**: Side-by-side or slider comparison of original vs. enhanced. Zoom synchronized across both panels.
- **WorkshopView.qml**: Browse, download, and manage community presets and models from Steam Workshop.
- **SettingsView.qml**: GPU selection, output format preferences, default enhancement settings, theme toggle.

### Layer 2: Application Logic (Pure C++)

Manages state, job scheduling, and platform integration. Pure C++ with no GPU dependencies — easy to test.

- **JobQueue**: Thread-safe priority queue. Each job contains input path, output path, enhancement settings, and a progress callback. Jobs execute on a dedicated thread pool (default: 2 concurrent jobs to avoid GPU memory exhaustion).
- **PresetManager**: Loads/saves enhancement presets as JSON files. Presets contain model selection, parameters, and processing chain configuration. Workshop presets download to a sandboxed directory.
- **LicenseManager**: Checks Steam DLC ownership via Steamworks API. DLC features are gated at the application layer, not the UI layer.
- **ProjectManager**: Manages enhancement history, undo/redo state, and recently opened files. Persists to a local SQLite database.
- **SettingsManager**: Application preferences, GPU selection, output defaults. Persists to JSON config file + Steam Cloud.

### Layer 3: Processing Engine

The core value of the product. Orchestrates AI models and image processing operations in a configurable pipeline.

**Pipeline architecture:**

```
Input → Decode → Pre-process → [Tile Splitter] → AI Inference → [Tile Merger] → Post-process → Encode → Output
```

Key design decisions:

- **Tile-based processing**: Large images are split into overlapping tiles (e.g., 512x512 with 32px overlap) to fit GPU VRAM. Tiles are processed independently and blended at seams with linear interpolation. This allows 8K+ image processing on 4GB GPUs.
- **Pipeline chaining**: Multiple operations (denoise → upscale → sharpen) execute as a single pipeline without writing intermediate files to disk.
- **Zero-copy GPU buffers**: Intermediate results stay in GPU memory between pipeline stages. Only the final output is copied back to CPU for file writing.
- **Tile size auto-detection**: 256px tiles for 4GB VRAM GPUs, 512px for 6–8GB, 768px for 12GB+. User-overridable in settings.

### Layer 4: GPU Backend

ONNX Runtime is the AI inference engine, providing a single API across all GPU vendors. Execution provider priority: TensorRT (NVIDIA, fastest) > CUDA (NVIDIA, reliable) > DirectML (any GPU, broadest compatibility) > CPU (fallback). The application attempts providers in order and falls back automatically.

| GPU Vendor | AI Inference Backend | Compute Backend | Steam Market Share (2025) |
|---|---|---|---|
| NVIDIA | ONNX Runtime + CUDA EP / TensorRT EP | DirectX 12 Compute | ~75% |
| AMD | ONNX Runtime + DirectML EP | DirectX 12 Compute | ~15% |
| Intel (Arc/integrated) | ONNX Runtime + DirectML EP | DirectX 12 Compute | ~10% |

For non-AI image processing (sharpening, color correction, format conversion, tile blending), DirectX 12 compute shaders provide maximum Windows performance. Vulkan compute shaders serve as fallback.

### Layer 5: Platform Integration

- **Steamworks SDK**: Workshop upload/download, DLC ownership checks, achievement tracking, overlay integration, cloud save for settings.
- **FFmpeg (dynamic linking)**: Video decode (H.264, H.265, VP9, AV1), video encode (H.264, H.265), container read/write (MP4, MKV, MOV, AVI). Ship FFmpeg DLLs alongside the application.
- **Windows Shell Integration**: "Enhance with Lumos" right-click context menu entry. File association for common image formats. Drag-and-drop from Explorer.

---

## 4. Repository Structure

Monorepo. Everything in one place.

```
lumos-ai/
├── CMakeLists.txt                    # Root CMake config
├── cmake/                            # CMake modules, toolchain files, Find*.cmake
├── src/
│   ├── main.cpp                      # Entry point, Qt application init
│   ├── app/                          # Application layer
│   │   ├── JobQueue.h/.cpp           # Thread-safe processing queue
│   │   ├── PresetManager.h/.cpp      # Load/save/Workshop preset management
│   │   ├── LicenseManager.h/.cpp     # DLC ownership via Steamworks
│   │   ├── ProjectManager.h/.cpp     # History, undo, recent files
│   │   └── SettingsManager.h/.cpp    # App preferences, GPU selection
│   ├── engine/                       # Processing engine
│   │   ├── Pipeline.h/.cpp           # Processing chain orchestrator
│   │   ├── TileProcessor.h/.cpp      # Tile split/merge logic
│   │   ├── AIInference.h/.cpp        # ONNX Runtime wrapper
│   │   ├── ImageOps.h/.cpp           # Sharpen, denoise, color ops (compute shaders)
│   │   ├── VideoProcessor.h/.cpp     # Frame extraction, processing, re-encoding
│   │   └── GPUContext.h/.cpp         # GPU device management, memory allocation
│   ├── ui/                           # QML files and UI C++ backends
│   │   ├── qml/                      # All .qml files
│   │   │   ├── MainWindow.qml
│   │   │   ├── EnhanceView.qml
│   │   │   ├── BatchView.qml
│   │   │   ├── CompareView.qml
│   │   │   ├── WorkshopView.qml
│   │   │   ├── SettingsView.qml
│   │   │   └── components/           # Reusable QML components
│   │   └── models/                   # C++ QAbstractListModel subclasses for QML
│   ├── platform/                     # Platform-specific code
│   │   ├── steam/                    # Steamworks integration
│   │   ├── win32/                    # Shell integration, context menu
│   │   └── media/                    # FFmpeg wrappers
│   └── common/                       # Shared utilities
│       ├── Logger.h/.cpp
│       ├── Config.h/.cpp
│       └── Types.h                   # Shared enums, structs, type aliases
├── models/                           # AI model files (.onnx)
│   ├── upscale/                      # Real-ESRGAN variants
│   ├── denoise/                      # Noise reduction models
│   └── restore/                      # Photo restoration models (DLC)
├── shaders/                          # HLSL/GLSL compute shaders
│   ├── hlsl/                         # DirectX 12 compute shaders
│   └── glsl/                         # Vulkan compute shaders (fallback)
├── resources/                        # Icons, fonts, default presets
├── tests/                            # Google Test + Qt Test
├── tools/                            # Build scripts, model conversion tools
├── docs/                             # Technical docs, API reference, ADRs
└── packaging/                        # Installer scripts, Steam depot config
```

### Architectural rules

1. **No singletons.** Use dependency injection. Every class receives its dependencies through the constructor.
2. **UI thread never blocks.** All file I/O, AI inference, and image processing runs on worker threads. The UI communicates via signals/slots and observes progress through Q_PROPERTY bindings.
3. **Engine has zero Qt dependency.** The processing engine (src/engine/) uses only standard C++, ONNX Runtime, and GPU APIs. It can be compiled and tested without Qt installed.
4. **Error handling via std::expected (C++23) or Result types.** No exceptions crossing module boundaries. The engine returns results; the application layer decides how to present errors.
5. **All strings are UTF-8.** Use std::u8string or QString internally. Convert at system API boundaries only.

---

## 5. Core Feature Specifications

### v1.0: AI Image Upscaling

- **Function**: Increase image resolution by 2x, 4x, or 8x (8x = two sequential 4x passes).
- **Primary model**: Real-ESRGAN x4plus (ONNX format, ~67MB).
- **Secondary models**: Real-ESRGAN Anime x4, ESRGAN (legacy), community Workshop models.
- **Input formats**: JPEG, PNG, WebP, BMP, TIFF, RAW (via LibRaw for DNG/CR2/NEF/ARW).
- **Output formats**: JPEG (quality slider 1–100), PNG (8/16-bit), WebP (lossy/lossless), TIFF.
- **Max input size**: Limited by GPU VRAM. Tile-based processing enables any resolution on 4GB+ GPUs.
- **Tile size**: Auto-detected based on VRAM: 256px (4GB), 512px (6–8GB), 768px (12GB+). User-overridable.
- **Tile overlap**: 32px default. Blended with linear interpolation to eliminate seams.
- **Performance target**: < 5 seconds for a 12MP photo at 4x on RTX 3060.
- **UI interaction**: Drag image onto window → click Enhance → before/after preview → Save. Three clicks maximum.

### v1.0: AI Noise Removal

- **Function**: Remove image noise (grain, ISO noise, compression artifacts) while preserving detail.
- **Model**: SCUNet or NAFNet (ONNX format). Both outperform BM3D on standard benchmarks.
- **Strength control**: Slider from 0–100. Maps to model parameter interpolation between no-op and full denoise.
- **Preserve detail toggle**: When enabled, applies edge-aware masking so denoising is reduced on detected edges/textures.
- **Pipeline integration**: Denoise → Upscale runs as a single pass without intermediate file write.

### v1.0: AI Sharpening

- **Function**: Enhance edge detail and perceived sharpness using AI-guided unsharp masking.
- **Approach**: Hybrid — AI model detects edges and textures, then applies targeted unsharp mask via compute shader. Not a simple convolution kernel.
- **Strength control**: Slider from 0–100 with real-time preview.
- **Halo prevention**: Clamped sharpening that prevents white/dark halos at contrast edges. Key quality differentiator vs. traditional sharpening.

### v1.0: Batch Processing

- **Function**: Process multiple files with identical settings in a queue.
- **UI**: Drag folder or multi-select files → choose enhancement preset → set output folder → Start. Progress bar per file + overall progress.
- **Concurrency**: Configurable: 1–4 simultaneous files depending on GPU VRAM. Auto-detected default.
- **Output naming**: Configurable pattern: `{original_name}_{enhancement}_{scale}x.{ext}`. Default: `{original_name}_enhanced.{ext}`.
- **Error handling**: Failed files are skipped and logged. Queue continues. Summary shown at completion.
- **Watch folder mode**: Optional — monitor a folder for new files and auto-enhance them.

### v1.0: Before/After Comparison

- **Modes**: (1) Draggable slider divider, (2) Side-by-side, (3) Toggle (press spacebar to flip).
- **Zoom**: Synchronized zoom on both views. Scroll to zoom, click-drag to pan. Zoom-to-fit, 100%, and 200% presets.
- **Rendering**: Both images rendered as GPU textures. Custom QML shader samples from two textures based on slider position. Zero CPU overhead.
- **Performance**: Must maintain 60fps during slider interaction, even for 8K images.

### v1.0: GPU Selection & Compatibility

- **Auto-detection**: On first launch, enumerate all GPUs via DirectX 12. Select the most powerful discrete GPU as default.
- **Manual override**: Settings panel lists all detected GPUs with VRAM, driver version, and compute capability.
- **Minimum requirements**: DirectX 12 capable GPU with 4GB VRAM (GTX 1050 Ti and newer).
- **CPU fallback**: ONNX Runtime CPU execution provider as last resort. 10–50x slower. Show warning.
- **VRAM monitoring**: Real-time VRAM usage display during processing. Automatically reduce tile size if allocation fails.

### v1.0: Steam Workshop Integration

- **Content types**: (1) Enhancement presets (JSON), (2) Custom AI models (ONNX), (3) Style packs (preset bundles with preview images).
- **Upload flow**: User creates a preset → clicks "Share to Workshop" → adds title, description, preview image → uploads via Steamworks UGC API.
- **Download flow**: Browse Workshop in-app → Subscribe → model/preset downloads to local cache → appears in preset list immediately.
- **Safety**: ONNX models cannot execute arbitrary code. Presets are JSON — validated on load.
- **Strategic purpose**: Community content keeps users engaged long after purchase. Every new Workshop item increases the product's value for every owner. This is the Wallpaper Engine flywheel.

---

## 6. AI Model Pipeline

### Model inventory

| Enhancement Type | Model | Origin/License | ONNX Size | Quality |
|---|---|---|---|---|
| Image Upscale 4x (General) | Real-ESRGAN x4plus | Tencent ARC / BSD-3 | ~67MB | State-of-art for general upscaling |
| Image Upscale 4x (Anime) | Real-ESRGAN Anime6B | Tencent ARC / BSD-3 | ~67MB | Best for anime/illustration |
| Image Upscale 4x (Photo) | SwinIR or HAT | ETH Zurich / Various | ~120MB | Slightly better PSNR on photos, heavier |
| Denoise | SCUNet / NAFNet | Various / MIT | ~15–25MB | Outperforms BM3D. SCUNet handles JPEG artifacts. |
| Face Enhancement | GFPGAN / CodeFormer | Tencent ARC / Apache 2.0 | ~350MB | DLC: restore faces in old photos |
| Video Upscale | Real-ESRGAN (frame-by-frame) | Same as image | ~67MB | DLC: process video frame-by-frame |
| Frame Interpolation | RIFE / IFRNet | Various / MIT/Apache | ~30–60MB | DLC: 30fps → 60fps, slow motion |

### Model conversion workflow

1. Find best open-source PyTorch model.
2. Convert to ONNX using torch.onnx.export or model author's export script.
3. Optimize with ONNX Runtime tools (graph optimization, quantization if quality allows).
4. Benchmark on target GPUs (GTX 1050 Ti, RTX 3060, RTX 4070, RX 6700 XT, Arc A770).
5. Ship the ONNX file.

### Model loading strategy

Models are large (67–350MB). Loading them into GPU memory takes 2–5 seconds.

- **Lazy loading**: Don't load models at app startup. Load the first time a model is needed.
- **Keep in memory**: Once loaded, keep the model in GPU memory until the user switches to a different model or the app closes.
- **Preload on idle**: After the UI is rendered, preload the user's most recently used model in the background.
- **VRAM budget**: Track total VRAM usage. If loading a new model would exceed 80% VRAM, unload the least-recently-used model first.

### Performance targets

| Operation | Input | GPU | Target Time |
|---|---|---|---|
| 4x Upscale | 1080p image (2MP) | RTX 3060 12GB | < 2 seconds |
| 4x Upscale | 12MP photo (4000x3000) | RTX 3060 12GB | < 5 seconds |
| 4x Upscale | 12MP photo | GTX 1050 Ti 4GB | < 30 seconds |
| Denoise | 12MP photo | RTX 3060 12GB | < 3 seconds |
| Denoise + 4x Upscale | 12MP photo | RTX 3060 12GB | < 7 seconds |
| Video 4x (per frame) | 1080p frame | RTX 3060 12GB | < 0.5 seconds (~2 fps) |

### Image quality validation

Automated quality checks compare output against reference images using PSNR and SSIM. Every model update must pass these gates:

| Model | Test Set | Min PSNR | Min SSIM |
|---|---|---|---|
| Real-ESRGAN x4plus | Set5 + Set14 | 26.0 dB | 0.78 |
| SCUNet (denoise, sigma 25) | CBSD68 | 31.0 dB | 0.88 |
| Sharpening | Custom 50-image set | 35.0 dB | 0.95 |

---

## 7. GPU Compute Architecture

### DirectX 12 compute shaders

Non-AI operations run as DirectX 12 compute shaders:

| Shader | Input | Output | Purpose |
|---|---|---|---|
| TileSplit.hlsl | Full image texture | Array of tile textures | Split image into overlapping tiles for AI processing |
| TileMerge.hlsl | Array of processed tiles | Full image texture | Merge tiles with linear blend at overlaps |
| UnsharpMask.hlsl | Image texture + edge mask | Sharpened image | AI-guided sharpening with halo prevention |
| ColorCorrect.hlsl | Image texture + LUT | Color-corrected image | Apply color grading via 3D LUT |
| FormatConvert.hlsl | Image in any color space | Image in target color space | sRGB, Linear, Adobe RGB, Rec.709 conversions |
| CompareSlider.hlsl | Two image textures + slider position | Composited output | Render the before/after comparison view |

### GPU memory management

This is the single most important technical challenge.

1. **Ring buffer for tiles**: Allocate a fixed-size ring buffer in GPU memory sized to hold N tiles (N = max concurrent tiles based on VRAM). Tiles cycle through: load → process → merge → free.
2. **Staging buffers for CPU↔GPU transfer**: Use D3D12 upload/readback heaps. Double-buffer to overlap transfer and computation.
3. **VRAM budget tracking**: Query DXGI adapter memory info. Maintain a running allocation counter. Refuse to allocate if exceeding 85% of available VRAM.
4. **Graceful degradation**: If VRAM allocation fails, halve the tile size and retry. If it fails again at minimum tile size (128px), fall back to CPU processing with a user warning.

---

## 8. Steam Integration

### Steamworks API integration points

| Feature | Steamworks API | Implementation Notes |
|---|---|---|
| App ownership | SteamApps()->BIsSubscribedApp() | Check on startup |
| DLC ownership | SteamApps()->BIsDlcInstalled(dlcAppId) | Check per DLC. Cache result per session. |
| Workshop upload | SteamUGC()->CreateItem() + SetItemContent() | Async. Show progress bar. |
| Workshop download | SteamUGC()->SubscribeItem() | Steam handles download. App handles install to local cache. |
| Workshop browse | SteamUGC()->CreateQueryAllUGCRequest() | Paginated results. Preview images in QML grid. |
| Achievements | SteamUserStats()->SetAchievement() | Unlock on milestones. |
| Overlay | SteamFriends()->ActivateGameOverlay() | Works automatically with Qt. Test for input conflicts. |
| Cloud saves | SteamRemoteStorage() | Save settings.json and presets. Max 100MB. |
| Rich presence | SteamFriends()->SetRichPresence() | Show "Enhancing a photo" in friends list. |

### Steam achievements

| Achievement | Trigger | Purpose |
|---|---|---|
| First Light | Enhance first image | Onboarding completion |
| Centurion | Enhance 100 images | Engagement milestone |
| Batch Master | Process 50+ files in one batch | Feature discovery |
| Workshop Creator | Upload first preset to Workshop | UGC flywheel |
| Curator | Download 10 Workshop items | Workshop engagement |
| Pixel Perfectionist | Use 8x upscale | Feature discovery |
| Time Traveler | Restore a photo older than 30 years (EXIF date) | Emotional hook, social sharing |
| Thousand Words | Process 1,000 images total | Long-term retention |

### Steam store page optimization

- Capsule images: Main (460x215, 231x87), hero (3840x1240), library assets. Budget $500–1000 for professional design.
- Trailer: 60–90 second video showing dramatic before/after transformations. No talking. Music + visual results.
- Screenshots: 5–8 showing UI, before/after, batch, Workshop, settings. Text overlays explaining what's shown.
- Description: Lead with anti-subscription message. Bullet features. GPU compatibility chart. Workshop pitch.
- Tags: "Photo Editing", "Video Production", "Utilities", "Software".
- Target 10,000+ wishlists before launch (threshold for meaningful algorithmic visibility).
- Price 20–30% higher than target since most Steam sales happen during discounts.
- Ensure 90%+ positive reviews (triples conversion vs. Mixed ratings).

---

## 9. UI/UX Design Specification

### Design philosophy

The UI must make complex AI processing feel effortless. Primary flow: drop a file → see preview → click Enhance → see result → save. Five actions, three clicks. Every additional setting is optional and hidden behind "Advanced" toggles.

Visual style: Dark theme default (matches Steam/gaming aesthetic), with optional light theme. Minimal chrome. The image is the hero.

### Layout

Three-panel layout:

- **Left sidebar (240px, collapsible)**: Navigation icons (Enhance, Batch, Compare, Workshop, Settings). Recent files list. GPU status indicator (name + VRAM usage bar).
- **Center canvas (flexible width)**: Image preview area. Drag-and-drop. Before/after comparison when enhancement is active. When empty: "Drop an image here or click to browse" prompt.
- **Right panel (320px, collapsible)**: Enhancement controls. Model selector, enhancement type toggles (Upscale/Denoise/Sharpen), strength sliders, output format picker. Big "Enhance" button at bottom. Progress bar replaces button during processing.

### Color palette

| Element | Dark Theme | Light Theme |
|---|---|---|
| Background | #1A1A2E | #F8F9FA |
| Surface | #16213E | #FFFFFF |
| Surface Elevated | #0F3460 | #E9ECEF |
| Primary Accent | #E94560 | #E94560 |
| Text Primary | #FFFFFF | #1A1A2E |
| Text Secondary | #ADB5BD | #6C757D |
| Success | #2D6A4F | #2D6A4F |
| Border | #2A2A4A | #DEE2E6 |

### Critical UX flows

**Flow 1: Single Image Enhancement (Golden Path)**

1. User drops image onto center canvas (or clicks Browse). Image loads at fit-to-window zoom.
2. Right panel auto-populates with recommended settings based on image analysis (detected noise level → denoise strength, resolution → upscale factor).
3. User optionally adjusts settings or selects a different model/preset.
4. User clicks Enhance. Button transforms into progress bar with percentage and ETA.
5. When complete, canvas switches to before/after comparison (slider). Save and Save As buttons appear.
6. User clicks Save (overwrites with backup) or Save As (choose location and format).

**Flow 2: Batch Processing**

1. User switches to Batch view. Drag-and-drop zone accepts folders or multi-selected files.
2. Files appear as scrollable thumbnail grid. Total file count and estimated time shown.
3. User selects enhancement preset from dropdown.
4. User sets output folder (default: subfolder "enhanced" next to input) and naming pattern.
5. User clicks Start Batch. Per-file progress + overall progress bar.
6. Completed files show green checkmarks. Failed files show red X with error tooltip.
7. Summary dialog on completion: X processed, Y failed, total time. "Open Output Folder" button.

### Performance requirements

| Interaction | Target |
|---|---|
| App cold start to usable UI | < 2 seconds |
| Image load to preview | < 500ms (up to 50MP) |
| Before/after slider interaction | 60 fps |
| Enhancement start to progress visible | < 200ms |
| Settings change to preview update | < 100ms |

---

## 10. DLC & Monetization Architecture

### DLC products (post-launch, months 9–18)

| DLC Name | Price | Content | Target Persona | Dev Effort |
|---|---|---|---|---|
| Video Enhancement Pack | $9.99 | Video upscaling (frame-by-frame), video denoising, video export (MP4/MKV) | Content creators, video archivists | 8–12 weeks |
| Photo Restoration Pack | $7.99 | Old photo restoration (scratch/damage removal), face enhancement (GFPGAN), colorization | Nostalgia restorers, genealogy | 6–8 weeks |
| Background Studio Pack | $7.99 | AI background removal, background replacement, bokeh simulation | Photographers, e-commerce | 4–6 weeks |
| Slow-Mo Creator Pack | $9.99 | Frame interpolation (RIFE/IFRNet), 30→60fps, 120fps, slow-motion creation | Gamers, content creators | 6–8 weeks |
| Pro Preset Collection | $4.99 | 50+ curated presets for specific use cases (landscape, portrait, street, macro) | Photographers | 2–3 weeks |
| Complete Bundle | $34.99 | All DLC at ~30% discount vs. individual | Power users | Packaging only |

### DLC implementation

DLC is implemented as feature flags, not separate codebases. All DLC code ships with the base product but is gated by Steamworks DLC ownership checks.

- Model files: DLC-specific ONNX models stored in Steam depots associated with DLC app IDs. Steam downloads depot automatically on purchase.
- Feature gating: `LicenseManager.hasDLC(DLC_VIDEO)` returns true/false. UI shows locked features with lock icon and "Get DLC" button → opens Steam store page.
- Promotion: When user attempts a DLC action (e.g., opens video file), show dialog: "Video enhancement is available with the Video Pack. One-time purchase, $9.99." with "Get it" and "Not now". Never block or nag.

### Revenue model

| Scenario | Base Units | Avg DLC Revenue/User | Effective Revenue/User | Gross Revenue |
|---|---|---|---|---|
| Year 1 (base only) | 33,400 | $0 | $29.99 | $1.0M |
| Year 2 (DLC active) | 100,000 | $8.50 | $38.49 | $3.85M |
| Year 3 (Workshop flywheel) | 250,000 | $10.00 | $39.99 | $10.0M |
| Year 5 (category leader) | 700,000 | $12.00 | $41.99 | $29.4M |
| Cumulative Year 5 | 1,200,000+ | — | — | $50–70M |
| Cumulative Year 7 | 2,500,000+ | — | — | $100M+ |

---

## 11. Build & Distribution Pipeline

### Distribution channels

| Channel | Dev Revenue Share | Registration Fee | Audience | Priority |
|---|---|---|---|---|
| Steam | 70% (first $10M) / 75% ($10M–$50M) / 80% ($50M+) | $100 per app | 132M+ MAU | Primary |
| Microsoft Store | 85–100% | $0 | 250M+ MAU | Secondary |
| Direct (website via Paddle/Gumroad) | ~95% | ~$20/month | SEO/organic | Tertiary |

Ship on Steam and Microsoft Store simultaneously. Direct website sales as third channel.

### Build/Release operations (lean)

- **Local verification first**: Before every PR, run local build and core tests on Windows x64 (MSVC 2022).
- **Release builds**: Tagged commits produce optimized signed artifacts and package outputs for Steam depot + MSIX.
- **Steam deployment**: Push to beta branch first, test, then promote to default branch.
- **Crash reporting**: Crashpad + Sentry/BugSplat for field issue diagnosis.
---

## 12. Execution Plan (Weeks 1–36, Lean User+Agent Model)

Operating model: one user (product/review/merge) + one agent (implementation). Keep weekly pace sustainable, prioritize shipping over process.

### Phase 1: Foundation (Weeks 1–8)

- Set up monorepo with CMake, vcpkg, Qt 6, ONNX Runtime.
- Implement GPUContext + AIInference proof-of-concept.
- Build minimal QML app shell with drag-and-drop.
- Implement tile split/merge and first end-to-end image upscale.
- Steam "Coming Soon" page goes live and wishlists start.

### Phase 2: Core Product (Weeks 9–20)

- Complete primary enhancement pipeline: denoise → upscale → sharpen.
- Build EnhanceView, before/after slider, and save/export options.
- Add batch queue, output naming patterns, and failure recovery.
- Implement GPU selection, VRAM monitoring, and fallback behavior.
- Add presets and initial Workshop read path (download/apply).

### Phase 3: Beta to Launch (Weeks 21–36)

- Closed beta: fix crashes first, then quality and UX friction.
- Open beta: creator content push, influencer seeding, review collection.
- Release candidate: feature freeze, regression pass, store submission.
- Launch: Steam first-week velocity, fast support loop, hotfix critical issues within 48 hours.

### Work Rhythm (No Bloat)

- One scoped change per branch
- Local build/tests before every PR
- Reviewer validates and merges
- Delete branch and repeat

---

## 13. Team Role Assignments (Lean)

| Role | Primary Responsibilities | Secondary |
|---|---|---|
| User (Owner/Reviewer) | Product priorities, acceptance testing, PR review/merge, release decisions | Marketing, community, pricing |
| Agent (Implementation) | Code changes, local verification, bug fixes, performance improvements | Documentation and technical decomposition |

### Team cadence

- Weekly priority reset (30-45 minutes max)
- PR-driven async collaboration
- Merge only tested, scoped changes

---
## 14. Quality Assurance & Testing

### Testing strategy

| Test Type | Framework | Scope | Frequency |
|---|---|---|---|
| Unit tests | Google Test | Engine logic: tile splitting, pipeline chaining, preset serialization, VRAM budget | Every commit (local) |
| Integration tests | Google Test + custom harness | Full pipeline: load → process → verify PSNR > threshold | Every PR merge (local + reviewer) |
| GPU smoke tests | Custom harness | Real GPU: ONNX model loads, inference completes, output dimensions correct | Weekly manual on target GPUs |
| UI tests | Qt Test + Squish (if budget) | Critical flows: drag-drop, enhance, before/after, batch | Weekly manual |
| Performance benchmarks | Custom harness + timer | Processing time per model per GPU. Alert if regression > 10% | Weekly manual |
| Compatibility matrix | Manual + automated | GTX 1050 Ti, RTX 3060, RTX 4070, RX 6700 XT, RX 7800 XT, Arc A770 | Before each release |

### Quality gates for release

- Zero P0 bugs (crashes, data loss, wrong output).
- < 5 P1 bugs (minor visual glitches, non-blocking UX). Documented in known issues.
- All GPU targets pass smoke tests.
- Cold start < 3 seconds on standard SSD.
- Memory leak check: process 1,000 images sequentially — VRAM and RAM must not grow unboundedly.

---

## 15. Marketing & Launch Playbook

### Pre-launch timeline

| Week | Channel | Action | KPI |
|---|---|---|---|
| 2 | Steam | "Coming Soon" page live | Wishlist tracking begins |
| 16 | Influencer | Early access keys to 10 micro-influencers (10K–100K subs) | 2–3 videos |
| 20 | All | Announce beta sign-up | 5,000 wishlists |
| 24 | TikTok/Shorts | 3–5 videos/week. Emotional: old photo restorations. | 10,000 wishlists |
| 28 | Reddit | AMA: "Indie team building a one-time-purchase Topaz alternative. AMA." | 500+ upvotes |
| 36 | LAUNCH | Coordinated push across all channels | 5,000 units Week 1 |

### Content strategy

AI enhancement tools produce naturally viral content: dramatic before/after transformations. Keep the format consistent and execution lightweight.

- Short-form before/after clips: TikTok/Shorts/Reels, 3-5 per week.
- Image showcases: Reddit and X, daily or near-daily cadence.
- Weekly community spotlight from user-submitted results.

### Launch week tactics

1. Launch with 10-15% discount for 7 days.
2. Coordinate influencer embargo lift with launch date.
3. Respond to every Steam review in week one.
4. Hotfix critical bugs within 48 hours.

---

## 16. Revenue Milestones & KPIs

### Year 1 target: $1M gross

| Period | Units (Cumulative) | Gross Revenue | Key Driver |
|---|---|---|---|
| Launch month | 5,000 | $150K | Launch discount + influencers + wishlist conversion |
| Month 2–3 | 12,000 | $360K | Word-of-mouth + content marketing + first Steam sale |
| Month 4–6 | 22,000 | $660K | Workshop growing + DLC launch |
| Month 7–12 | 35,000+ | $1M+ | Holiday sale + organic discovery + DLC revenue |

### KPIs

| KPI | Month 1 | Month 6 | Month 12 | Why |
|---|---|---|---|---|
| Steam review score | 90%+ | 90%+ | 90%+ | Below 80% = death. 90%+ = 3x conversion. |
| Steam wishlists | 15,000 (pre-launch) | 25,000 | 40,000 | Convert at ~10% during sales |
| Daily active users | 2,000 | 5,000 | 8,000 | Engaged users = reviews + Workshop |
| Workshop items | 50 | 500 | 2,000+ | UGC flywheel |
| DLC attach rate | N/A | 15% | 25% | Revenue multiplier |
| Refund rate | < 10% | < 8% | < 5% | High refunds = product problem |

---

## 17. Risk Register

| Risk | Probability | Impact | Mitigation |
|---|---|---|---|
| Topaz launches a Steam version | Medium | High | First-mover advantage. Maintain price advantage ($30 vs. $300/yr). Community features they can't replicate quickly. |
| Open-source GUI tool improves significantly (upscayl etc.) | Medium | Medium | Differentiate on polish, batch processing, Workshop, reliability. Upscayl is Electron — can't match native GPU performance. |
| AI model quality plateaus | Low | Medium | Models still improving annually. Workshop provides variety. Custom training if needed. |
| GPU compatibility issues | High | High | Budget 20% dev time for compatibility testing. DirectML broadest compat. Test 6+ GPUs. |
| Steam rejects or buries the app | Low | High | Comply with guidelines. Microsoft Store as backup (better margins anyway). |
| Team overload in a 2-person model | Medium | High | Keep sustainable pace, cap WIP, prioritize by revenue leverage, and defer non-core work. |
| Qt LGPL compliance misstep | Low | High | Dynamic linking only. Never modify Qt source. Budget for commercial license as insurance. |

---

## 18. Post-Launch Roadmap

### Year 2: DLC ecosystem & growth ($5–15M)

- Launch all DLC packs.
- Localization: German, Japanese, Chinese (Simplified), Korean, Portuguese.
- macOS port (Qt cross-platform; replace DX12 with Metal compute).
- Linux port (Vulkan already the fallback; lower effort than macOS).

### Year 3–4: Category leadership ($10–25M/yr)

- Target 500K+ total owners.
- API/SDK for third-party integration.
- Advanced AI: style transfer, AI color grading, intelligent cropping.

### Year 5–7: $100M cumulative & exit options

- Multiple exit paths with $100M+ cumulative and 2.5M+ customers:
  - **Continue independently**: $10–25M/yr with small team, 50%+ margins.
  - **Acquisition**: Canva acquired Affinity for $380M–$1B. Adobe, Canva, or similar could acquire for AI capabilities and user base.

---

## 19. Minimum System Requirements

| Component | Minimum | Recommended |
|---|---|---|
| OS | Windows 10 64-bit version 1903+ | Windows 11 |
| CPU | Intel Core i5-6400 / AMD Ryzen 3 1200 | Intel Core i7-10700 / AMD Ryzen 5 5600X |
| RAM | 8 GB | 16 GB |
| GPU | NVIDIA GTX 1050 Ti 4GB / AMD RX 570 4GB / Intel Arc A380 | NVIDIA RTX 3060 12GB / AMD RX 6700 XT 12GB |
| GPU Driver | DirectX 12 compatible | Latest driver |
| Storage | 2 GB + space for models and output | SSD with 10 GB free |
| Display | 1280x720 | 1920x1080 |

---

## 20. Open-Source Model Licenses

| Model | License | Commercial Use | Notes |
|---|---|---|---|
| Real-ESRGAN | BSD-3-Clause | Yes, freely | Attribute Tencent ARC in credits |
| GFPGAN | Apache 2.0 | Yes, freely | Attribute in credits |
| CodeFormer | S-Lab License 1.0 | Yes, with attribution | Check redistribution terms |
| SCUNet | Apache 2.0 | Yes, freely | Academic origin |
| NAFNet | MIT | Yes, freely | Most permissive |
| RIFE | MIT | Yes, freely | For frame interpolation DLC |

---

## 21. Key Technical References

| Resource | URL / Reference | Purpose |
|---|---|---|
| Qt 6 Documentation | doc.qt.io/qt-6/ | API reference and tutorials |
| ONNX Runtime | onnxruntime.ai/docs/ | API, execution providers, optimization |
| Real-ESRGAN | github.com/xinntao/Real-ESRGAN | Model architecture, ONNX export |
| Steamworks SDK | partner.steamgames.com/doc/ | Steam integration docs |
| DirectX 12 Guide | learn.microsoft.com/en-us/windows/win32/direct3d12/ | DX12 compute, resources |
| FFmpeg | ffmpeg.org/documentation.html | Video codec reference |
| vcpkg | github.com/microsoft/vcpkg | C++ dependency management |
| CMake | cmake.org/cmake/help/latest/ | Build system reference |
