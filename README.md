# PMDoT Engine

**PMDoT Engine** is the official custom fork/build of the Godot Engine (Godot 4.3-stable) developed by PMDT Studios, tailored and optimized for Pixel Art game development.

---

## Features and Main Modifications

### 1. Embedded Aseprite Workspace in Editor
- **Native Editor Tab**: Aseprite is integrated directly into the editor interface as a native workspace.
- **Drawing Tools**: Pencil, Eraser, Bucket Fill (Flood Fill), Color Picker, and zoom controls.
- **Direct Export**: Real-time export directly to the project directory structure (`res://assets/sprites/`).

### 2. High-Performance C++ Pipeline with SIMD Optimizations
- **AVX2 / SSE4.1 Acceleration**: Real-time pixel buffer conversion and management via SIMD vector instructions.
- **Indexed Palette Expansion**: Fast vector lookup (8 simultaneous elements in AVX2) converting 8-bit color maps to RGBA32.
- **Channel Swizzling & Alpha Premultiplication**: Fast BGRA/RGBA channel swapping and vectorized alpha multiplication for zero-latency sprite layer blitting.

### 3. Project Scaffolding (`--project create`)
Create production-ready Godot projects from the terminal with pixel art presets and architecture patterns:

```bash
.\pmdot --project create 2d px320 mvc
.\pmdot --project create 2d-plat px640 feature --path D:\MyGame
```

- **5 Pixel Art Resolution Presets** with automatic upscaling:

  | Preset | Base Resolution | Window Override | Scale |
  |:---|:---|:---|:---|
  | `px128` | 128×72 | 1280×720 | 10x |
  | `px256` | 256×144 | 1280×720 | 5x |
  | `px320` | 320×180 | 1280×720 | 4x |
  | `px480` | 480×270 | 1440×810 | 3x |
  | `px640` | 640×360 | 1920×1080 | 3x |

- **7 Architecture Patterns** for folder structure:

  | Pattern | Description |
  |:---|:---|
  | `mvc` | Model-View-Controller separation |
  | `feature` | Feature-based (player/, enemy/, ui/) |
  | `layer` | Layered (presentation/, domain/, data/) |
  | `ecs` | Entity-Component-System |
  | `minimal` | Minimal for prototypes and game jams |
  | `modular` | Independent reusable modules |
  | `standard` | Standard Godot community convention |

### 4. Incremental Segmented Build
The build system tracks **10 source segments** via SHA256 hashes. Only changed components trigger a rebuild.

```bash
.\pmdot build                  # Incremental (skips if nothing changed)
.\pmdot build --force          # Force full rebuild
.\pmdot build --only pmdot_aseprite,godot_editor   # Rebuild specific segments
```

### 5. Native Pixel Art Presets & Automatic Rendering Configuration
- Default texture filter set to **Nearest** (`default_texture_filter = 0`).
- Window stretch mode set to `canvas_items` and aspect ratio to `keep`.
- Automatic `project.godot` generation with correct viewport and window override sizes.

---

## 🧰 PMDoT CLI Tool (`pmdot`)

The project includes an all-in-one cross-platform management utility supporting **Windows**, **Linux**, and **macOS**.

```bash
# Windows
.\pmdot.bat <command>

# Linux / macOS
./pmdot <command>
```

### CLI Command Reference

| Command | Short Flag | Description |
| :--- | :--- | :--- |
| `pmdot install` | `pmdot -i` | Verifies and installs all system dependencies and initializes submodules. |
| `pmdot version` | `pmdot -v` | Displays release version info for PMDoT, Godot, and Aseprite module. |
| `pmdot update` | `pmdot -u` | Updates PMDoT codebase and all submodules. |
| `pmdot build` | `pmdot -b` | Builds the PMDoT Engine (incremental segmented build). |
| `pmdot build --force` | | Forces a full rebuild ignoring segment cache. |
| `pmdot build --only <segs>` | | Rebuilds only specific segments (comma-separated). |
| `pmdot run` | `pmdot -r` | Launches the compiled PMDoT Engine editor. |
| `pmdot doctor` | `pmdot -d` | Runs diagnostic report on tools, SDKs, and build cache. |
| `pmdot clean` | `pmdot -c` | Cleans build caches (SCons + segment hashes). |
| `pmdot --project create` | | Creates a new Godot project with scaffolding. |
| `pmdot help` | `pmdot -h` | Shows help message. |

> 📖 For complete documentation of all commands, parameters, and examples, see [`architecture-guidelines/cli_reference.md`](architecture-guidelines/cli_reference.md).

---

## 🐳 Docker Container Support

You can build PMDoT Engine inside an isolated reproducible Linux container using Docker:

```bash
# Build and run container build
docker compose run pmdot-builder
```

Or build using Docker directly:
```bash
docker build -t pmdot-engine .
```

---

## Architecture Guidelines

For complete technical details regarding Aseprite integration, the SIMD pipeline, and C++ module lifecycle, refer to the [`architecture-guidelines/`](architecture-guidelines/) directory.

- [Aseprite Architecture Guide](architecture-guidelines/aseprite_architecture_guide.md) — Internal architecture, SIMD pipeline, module lifecycle
- [CLI Reference](architecture-guidelines/cli_reference.md) — Full CLI command documentation and usage wiki
