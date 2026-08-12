# PMDoT CLI Reference

Complete command reference for the **PMDoT Engine CLI** (`pmdot`). This document covers every command, parameter, flag, and provides usage examples.

---

## Overview

The PMDoT CLI is a Python-based cross-platform tool that manages the entire lifecycle of the PMDoT Engine: from installing dependencies, building the engine, creating game projects, to launching the editor.

### How to Use

```bash
# Windows (CMD / PowerShell)
.\pmdot.bat <command> [options]
.\pmdot.ps1 <command> [options]
python pmdot.py <command> [options]

# Linux / macOS
./pmdot <command> [options]
python3 pmdot.py <command> [options]
```

---

## Core Commands

### `install` (alias: `-i`)

Verifies all system requirements and initializes Git submodules.

```bash
pmdot install
```

**What it does:**
1. Detects OS platform (Windows / Linux / macOS)
2. Checks for: Git, Python, SCons, CMake, Ninja
3. Checks for C++ compiler (MSVC on Windows, GCC on Linux, Clang on macOS)
4. Clones `source-godot` (Godot 4.3-stable) if not present
5. Clones Aseprite thirdparty submodules if not present
6. Creates `.pmdot_cache/` directory with environment info

---

### `version` (alias: `-v`)

Displays version information for all components.

```bash
pmdot version
```

**Output example:**
```
=== PMDoT Engine Version Info ===
PMDoT Version:            1.1.0-custom
Godot Base Release:       4.3-stable
Aseprite Module:          pmdot_aseprite v1.0.0
Detected System OS:       WINDOWS (AMD64)
```

---

### `update` (alias: `-u`)

Pulls latest changes for PMDoT and all submodules.

```bash
pmdot update
```

**What it does:**
1. `git pull` on the PMDoT repository
2. `git submodule update --init --recursive` on source-godot
3. `git submodule update --init --recursive` on Aseprite thirdparty

---

### `doctor` (alias: `-d`)

Runs a full system diagnostic report.

```bash
pmdot doctor
```

**Output includes:**
- OS platform and version
- Python executable and version
- Paths to Git, SCons, CMake, Ninja
- Status of Godot source directory
- Status of PMDoT module
- Build segment cache status (which segments are cached)

---

## Build Commands

### `build` (alias: `-b`)

Builds the PMDoT Engine using the **incremental segmented build system**.

```bash
pmdot build                # Incremental build (default)
pmdot build --force        # Force full rebuild, ignore cache
pmdot build --only <segs>  # Rebuild specific segments only
```

#### Incremental Build System

Before invoking SCons, the CLI scans the source tree divided into **10 logical segments** and computes SHA256 hashes of all source files (`.cpp`, `.c`, `.h`, `.hpp`, `.py`, `.inc`). These hashes are compared against the cache stored in `.pmdot_cache/build_segments.json`.

**Build segments:**

| Segment | Directory | Description |
|:---|:---|:---|
| `godot_core` | `source-godot/core/` | Godot Engine Core |
| `godot_scene` | `source-godot/scene/` | Scene System |
| `godot_servers` | `source-godot/servers/` | Rendering, Physics, Audio Servers |
| `godot_editor` | `source-godot/editor/` | Editor UI |
| `godot_drivers` | `source-godot/drivers/` | GPU / Audio Drivers |
| `godot_platform` | `source-godot/platform/` | Platform-Specific Code |
| `godot_main` | `source-godot/main/` | Main Entry Point |
| `godot_thirdparty` | `source-godot/thirdparty/` | Third-Party Libraries |
| `godot_modules` | `source-godot/modules/` (excl. pmdot_*) | Godot Built-in Modules |
| `pmdot_aseprite` | `source-godot/modules/pmdot_aseprite/` | PMDoT Aseprite Module |

**Behavior:**
- If **no segments changed**: prints "Build is up to date" and exits immediately (no SCons invocation).
- If **segments changed**: reports which ones, then invokes SCons for a full build. SCons internally handles per-file incremental compilation.
- After **successful build**: updates the segment hash cache.
- After **failed build**: does NOT update cache, so the next `pmdot build` will detect the same changes.

#### Flags

| Flag | Description |
|:---|:---|
| `--force` | Ignores the segment cache and forces SCons to run regardless of changes. |
| `--only <segments>` | Comma-separated list of segments to check. Only these segments are evaluated for changes. |

#### Examples

```bash
# Standard incremental build
pmdot build

# Force full rebuild
pmdot build --force

# Only check and rebuild if pmdot_aseprite or godot_editor changed
pmdot build --only pmdot_aseprite,godot_editor

# Pass extra flags to SCons
pmdot build vsprops=yes
```

---

### `clean` (alias: `-c`)

Cleans all build caches and temporary files.

```bash
pmdot clean
```

**What it removes:**
- `.sconsign.dblite` (SCons dependency database)
- `.sconf_temp/` (SCons configuration cache)
- `.pmdot_cache/build_segments.json` (PMDoT segment hash cache)

> **Note:** This does NOT remove compiled `.obj` or `.lib` files. For a truly clean rebuild, run `pmdot clean` followed by `pmdot build --force`.

---

## Project Commands

### `--project create`

Creates a new Godot project with a pre-configured folder structure, pixel art resolution preset, and automatic `project.godot` generation.

```bash
pmdot --project create <type> <px_preset> <arch_pattern> [--path <dir>]
pmdot --project create help    # Show detailed help
```

#### Parameters

| Parameter | Required | Description |
|:---|:---|:---|
| `type` | Yes | Game type: `2d`, `3d`, `2d-top`, `2d-plat` |
| `px_preset` | Yes | Pixel art resolution preset |
| `arch_pattern` | Yes | Folder architecture pattern |
| `--path <dir>` | No | Directory to create project in (default: current directory) |

#### Pixel Art Presets

All presets configure: Nearest texture filtering, `canvas_items` stretch mode, `keep` aspect ratio.

| Preset | Base Resolution | Window Override | Scale | Best For |
|:---|:---|:---|:---|:---|
| `px128` | 128×72 | 1280×720 | 10x | NES/Game Boy style |
| `px256` | 256×144 | 1280×720 | 5x | SNES/GBA style |
| `px320` | 320×180 | 1280×720 | 4x | Classic retro (recommended) |
| `px480` | 480×270 | 1440×810 | 3x | High-detail retro |
| `px640` | 640×360 | 1920×1080 | 3x | Modern pixel art |

#### Architecture Patterns

##### `mvc` — Model-View-Controller

```
project/
├── assets/sprites/{characters,environment,ui}/
├── assets/audio/{sfx,music}/
├── assets/fonts/
├── assets/shaders/
├── models/
├── views/{scenes,ui}/
├── controllers/
└── autoload/
```

##### `feature` — Feature-Based

```
project/
├── features/player/{scenes,scripts,sprites}/
├── features/enemy/{scenes,scripts,sprites}/
├── features/ui/{scenes,scripts,sprites}/
├── features/world/{scenes,scripts,tilesets}/
├── shared/assets/{audio,fonts,shaders}/
├── shared/scripts/
└── autoload/
```

##### `layer` — Layered Architecture

```
project/
├── presentation/{scenes,ui,sprites,shaders}/
├── domain/{entities,systems,events}/
├── data/{save,config,resources}/
├── assets/audio/{sfx,music}/
├── assets/fonts/
└── autoload/
```

##### `ecs` — Entity-Component-System

```
project/
├── entities/
├── components/{physics,rendering,input,ai,stats}/
├── systems/
├── resources/{sprites,audio,fonts,shaders}/
├── scenes/{levels,ui}/
└── autoload/
```

##### `minimal` — Minimal

```
project/
├── src/
├── assets/{sprites,audio,fonts}/
└── scenes/
```

##### `modular` — Modular

```
project/
├── modules/core/{scripts,scenes}/
├── modules/gameplay/{scripts,scenes}/
├── modules/ui/{scripts,scenes,themes}/
├── modules/audio/{scripts,resources}/
├── assets/{sprites,fonts,shaders}/
└── autoload/
```

##### `standard` — Standard (Godot Convention)

```
project/
├── scenes/{actors,levels,ui}/
├── scripts/{global,components,resources}/
├── assets/sprites/{characters,environment,ui}/
├── assets/audio/{sfx,music}/
├── assets/{fonts,shaders,themes}/
└── autoload/
```

#### Generated Files

Each `--project create` command generates:

| File | Description |
|:---|:---|
| `project.godot` | Godot project file with pixel art settings pre-configured |
| `default_env.tres` | Default environment resource |
| `.gitignore` | Git ignore rules for Godot projects |
| `.gdkeep` (in each folder) | Empty files so Git tracks empty directories |

#### Examples

```bash
# Create a 2D platformer with 320x180 resolution and MVC architecture
pmdot --project create 2d-plat px320 mvc

# Create in a specific directory
pmdot --project create 2d px640 feature --path D:\Projects\MyPixelGame

# Create a minimal prototype with ultra-retro resolution
pmdot --project create 2d px128 minimal

# Create a top-down game with ECS pattern
pmdot --project create 2d-top px256 ecs

# Show detailed help
pmdot --project create help
```

---

## Run Commands

### `run` (alias: `-r`)

Launches the compiled PMDoT Engine editor.

```bash
pmdot run
```

**What it does:**
1. Looks for the compiled executable in `source-godot/bin/`
2. Platform-specific binary names:
   - Windows: `godot.windows.editor.x86_64.exe`
   - Linux: `godot.linuxbsd.editor.x86_64`
   - macOS: `godot.macos.editor.x86_64`
3. Launches with `--editor` flag

> **Note:** You must run `pmdot build` at least once before `pmdot run`.

---

## Help

### `help` (alias: `-h`, `--help`)

Shows the command overview and usage examples.

```bash
pmdot help
```

---

## Troubleshooting

### Build fails with `_Mtx_init_in_situ` error (MSVC)

This occurs with MSVC toolset 14.44+ (Visual Studio 2022 17.14+). The fix (`_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR`) is already applied in `platform/windows/detect.py`. If you still see this error:
1. Run `pmdot clean`
2. Run `pmdot build --force`

### Build says "up to date" but I changed a file

The segment cache tracks `.cpp`, `.c`, `.h`, `.hpp`, `.py`, and `.inc` files. If you changed a different file type (e.g., `.gdshader`, `.xml`), use:
```bash
pmdot build --force
```

### SCons not found

```bash
pip install scons
# or
pmdot install
```

### `--project create` won't overwrite existing project.godot

This is intentional. The scaffolding command will NOT overwrite an existing `project.godot`. Delete it manually first if you want to regenerate.

### Segment hash scan is slow

The first scan of `godot_thirdparty` can take a few seconds due to the large number of files. Subsequent scans are faster because the OS caches the file system metadata. If this is a concern, use `--only` to scan specific segments.

---

## Configuration Files

| File | Location | Description |
|:---|:---|:---|
| `cache.json` | `.pmdot_cache/cache.json` | OS/Python environment info |
| `build_segments.json` | `.pmdot_cache/build_segments.json` | SHA256 hashes for each build segment |
