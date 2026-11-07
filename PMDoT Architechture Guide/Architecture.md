# Architecture — PMDoT Engine

PMDoT is a specialized fork of **Godot Engine 4.3-stable**, maintained by PMDT Studios for high-performance Pixel Art game development.

> Documented state: **Current** (post-CHG-002, CHG-003, CHG-004: GDScript purged, native C#/C++/Java/Markdown script creation active, Mobile and Web platforms completely removed).

---

## 1. High-Level Repository Architecture

```mermaid
flowchart TD
    subgraph Root["d:/PMDoT (Root)"]
        CLI["pmdot.py + pmdot / pmdot.bat / pmdot.ps1"]
        BuildPy["build_pmdot.py"]
        Docker["Dockerfile / docker-compose.yml"]
        Guide["PMDoT Architechture Guide/"]
        OldGuide["architecture-guidelines/"]
        Cache[".pmdot_cache/ (build_segments.json)"]
    end
    subgraph Engine["source-godot/ (Godot 4.3 fork)"]
        Core["core/"]
        Scene["scene/"]
        Servers["servers/"]
        Editor["editor/"]
        Main["main/"]
        Drivers["drivers/"]
        Platform["platform/ (windows, linuxbsd, macos)"]
        Modules["modules/"]
        Tests["tests/"]
        Third["thirdparty/"]
    end
    CLI -->|"scons platform=... target=editor custom_modules=modules/pmdot_aseprite"| Engine
    BuildPy -->|scons| Engine
    CLI --> Cache
    Modules --> Aseprite["pmdot_aseprite (custom)"]
```

| Path | Responsibility |
|---|---|
| `pmdot.py` | Unified CLI: `install`, `version`, `update`, `build` (segmented incremental builds), `run`, `doctor`, `clean`, `--project create` |
| `pmdot`, `pmdot.bat`, `pmdot.ps1` | CLI platform wrappers for Linux, macOS, and Windows |
| `build_pmdot.py` | Standalone direct build script (SCons, `-j4`) |
| `.pmdot_cache/` | SHA256 build segment cache (git-ignored) |
| `architecture-guidelines/` | Low-level technical guidelines for Aseprite/SIMD and CLI reference |
| `PMDoT Architechture Guide/` | Core architectural guide: Architecture, Modules, Path Guidelines, Testing, and Changes |
| `source-godot/` | Engine codebase (Godot 4.3 fork targeting Windows, Linux, macOS) |

---

## 2. Supported Target Platforms & Runtimes

PMDoT is exclusively engineered for desktop workstations and desktop game distribution:

| Platform | Status | Architecture | Notes |
|---|---|---|---|
| **Windows** | ✅ Active | x86_64, ARM64 | Native MSVC toolchain, DirectX 12 / Vulkan / OpenGL 3.3 |
| **Linux / BSD** | ✅ Active | x86_64, ARM64 | GCC / Clang, X11 / Wayland, Vulkan / OpenGL 3.3 |
| **macOS** | ✅ Active | x86_64, Apple Silicon (arm64) | Apple Clang, MoltenVK / OpenGL 3.3 |
| **Android** | 🗑️ Purged | — | Removed completely (platform code, templates, gradle, JNI, OpenXR extension) |
| **iOS** | 🗑️ Purged | — | Removed completely (platform code, Xcode templates, codesign bundles, .NET AOT) |
| **Web (HTML5 / Wasm)** | 🗑️ Purged | — | Removed completely (platform code, Emscripten helpers, WebXR module, JS libraries) |

---

## 3. Segmented Incremental Build System (`pmdot build`)

`BUILD_SEGMENTS` in `pmdot.py` segments `source-godot` into 10 independent verification zones. Each computes a composite SHA256 hash across tracked source file extensions (`.cpp`, `.c`, `.h`, `.hpp`, `.py`, `.inc`, `.glsl`):

`godot_core` · `godot_scene` · `godot_servers` · `godot_editor` · `godot_drivers` · `godot_platform` · `godot_main` · `godot_thirdparty` · `godot_modules` (excluding `pmdot_*`) · `pmdot_aseprite`

Base SCons invocation:
```bash
scons platform=<os> target=editor disable_exceptions=no custom_modules=modules/pmdot_aseprite module_gdscript_enabled=no module_jsonrpc_enabled=no -j8
```

---

## 4. Scripting and Gameplay Runtime Layer

```mermaid
flowchart LR
    SS["ScriptServer / ScriptLanguage (core)"]
    CS["CSharpLanguage (modules/mono)"]
    GDE["GDExtension (core/extension)"]
    CPP["Native C++ Modules (pmdot_*)"]
    SS --> CS
    SS --> GDE
    CPP --> SS
```

| State | Supported Gameplay Languages |
|---|---|
| Target / Current | **C# (`mono` module / .NET)** and **C++ (GDExtension & Native Engine Modules)**. Script creation dialog natively supports **C#**, **C++**, **Java** (roadmap notice), and **Markdown**. |
| Planned / Roadmap | Native **Java** language and runtime support. |
| Legacy (Removed) | GDScript (`modules/gdscript`) and JSON-RPC (`modules/jsonrpc`) have been completely decoupled and removed. |

The core script abstraction layer (`core/object/script_language*`, `ScriptServer`, `core/extension/`) remains in place to serve C# and GDExtension bindings. The `ScriptCreateDialog` and editor docks provide default authoring for C#, C++, Java, and Markdown without requiring GDScript module presence.

---

## 5. Renderer & Project Creation Architecture

PMDoT standardizes on two desktop rendering backends:
1. **Forward+** (`forward_plus`): High-end desktop renderer leveraging the Vulkan RenderingDevice.
2. **Compatibility** (`gl_compatibility`): Lightweight OpenGL 3.3/GLES3 renderer optimal for 2D pixel art and legacy GPUs.

Legacy Godot "Mobile" rendering checkboxes and `.mobile` / `.web` project setting overrides have been completely removed from project scaffolding and the editor UI.

---

## 6. Module Initialization Lifecycle

1. `modules/SCsub` scans `config.py` in every module (`can_build`, `configure`) and dynamically generates `modules_enabled.gen.h` with `MODULE_<NAME>_ENABLED` preprocessor macros.
2. `register_module_types.gen.cpp` executes `initialize_<module>_module(level)` for each lifecycle phase (`CORE`, `SERVERS`, `SCENE`, `EDITOR`).
3. External code depending on any module must be guarded with `#ifdef MODULE_<NAME>_ENABLED`.

---

## 7. Aseprite & SIMD Pixel Art Pipeline
For low-level SIMD specifications (AVX2/SSE4.1 routines, `EditorAsepriteWorkspace`, and `PMDoTPixelArtPresets`), refer to `architecture-guidelines/aseprite_architecture_guide.md`.

---

## 8. Architectural Rules
- All new PMDoT features must reside in dedicated custom modules under `source-godot/modules/pmdot_<name>/`. Avoid direct modifications to upstream Godot directories (`core/`, `scene/`, `editor/`).
- Any unavoidable modification to upstream Godot files must be recorded in `Changes.md` with exact file paths and reasoning.
- Inter-module dependencies must be declared in `config.py` using `env.module_add_dependencies()`.
- Module deprecation and removal must follow the standard decoupling and verification lifecycle (flag disablement, source tree cleanup, macro scrubbing, and test suite adjustment).
- PMDoT remains strictly desktop-focused. Never re-introduce mobile or web dependencies, export platforms, or build chains.
