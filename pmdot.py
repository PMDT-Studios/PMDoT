#!/usr/bin/env python3
"""
PMDoT Engine CLI & Installation Manager
Supports Windows, Linux, and macOS.

Commands:
  install, -i         Verify and install all system requirements and submodules
  version, -v         Check PMDoT and Godot release version information
  update, -u          Update PMDoT repositories and submodules to latest release
  build, -b           Build PMDoT Engine (incremental segmented build)
  run, -r             Launch the compiled PMDoT Engine editor
  doctor, -d          Run diagnostic report on tools and SDKs
  clean, -c           Clean temporary build cache and logs
  --project create    Create a new Godot project with architecture pattern & pixel art preset
  help, -h            Show help message
"""

import sys
import os
import platform
import subprocess
import shutil
import json
import hashlib
import time

# Force UTF-8 stdout if possible on Windows
if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass

VERSION_PMDOT = "1.1.0-custom"
VERSION_GODOT = "4.3-stable"
VERSION_ASEPRITE_MODULE = "1.0.0"

ROOT_DIR = os.path.abspath(os.path.dirname(__file__))
CACHE_DIR = os.path.join(ROOT_DIR, ".pmdot_cache")
CACHE_FILE = os.path.join(CACHE_DIR, "cache.json")
BUILD_SEGMENTS_FILE = os.path.join(CACHE_DIR, "build_segments.json")
GODOT_DIR = os.path.join(ROOT_DIR, "source-godot")
MODULE_DIR = os.path.join(GODOT_DIR, "modules", "pmdot_aseprite")


# =============================================================================
# Terminal Colors
# =============================================================================

class Colors:
    GREEN = "\033[92m"
    CYAN = "\033[96m"
    YELLOW = "\033[93m"
    RED = "\033[91m"
    BOLD = "\033[1m"
    DIM = "\033[2m"
    MAGENTA = "\033[95m"
    END = "\033[0m"


def print_header(text):
    print(f"\n{Colors.BOLD}{Colors.CYAN}=== {text} ==={Colors.END}")


def print_success(text):
    print(f"{Colors.GREEN}[OK] {text}{Colors.END}")


def print_warning(text):
    print(f"{Colors.YELLOW}[WARN] {text}{Colors.END}")


def print_error(text):
    print(f"{Colors.RED}[FAIL] {text}{Colors.END}")


def print_info(text):
    print(f"{Colors.DIM}[INFO] {text}{Colors.END}")


def get_os_platform():
    system = platform.system().lower()
    if "windows" in system or "cygwin" in system:
        return "windows"
    elif "darwin" in system:
        return "macos"
    elif "linux" in system:
        return "linux"
    return system


def ensure_cache():
    if not os.path.exists(CACHE_DIR):
        os.makedirs(CACHE_DIR, exist_ok=True)

    gitignore_path = os.path.join(ROOT_DIR, ".gitignore")
    entry = ".pmdot_cache/"
    if os.path.exists(gitignore_path):
        with open(gitignore_path, "r", encoding="utf-8") as f:
            content = f.read()
        if entry not in content:
            with open(gitignore_path, "a", encoding="utf-8") as f:
                f.write(f"\n{entry}\n")
    else:
        with open(gitignore_path, "w", encoding="utf-8") as f:
            f.write(f"{entry}\n")

    os_info = {
        "os": get_os_platform(),
        "arch": platform.machine(),
        "python": sys.version.split()[0],
    }
    with open(CACHE_FILE, "w", encoding="utf-8") as f:
        json.dump(os_info, f, indent=2)

    return os_info


def run_command(cmd, cwd=None, check=True):
    try:
        res = subprocess.run(cmd, cwd=cwd, check=check)
        return res.returncode == 0
    except Exception as e:
        print_error(f"Failed executing {' '.join(cmd)}: {e}")
        return False


# =============================================================================
# Feature 1: --project create
# =============================================================================

# --- Pixel Art Presets ---
PIXEL_PRESETS = {
    "px128": {
        "name": "Ultra Retro (128x72)",
        "viewport_width": 128,
        "viewport_height": 72,
        "window_width": 1280,
        "window_height": 720,
        "scale": 10,
    },
    "px256": {
        "name": "Retro Low (256x144)",
        "viewport_width": 256,
        "viewport_height": 144,
        "window_width": 1280,
        "window_height": 720,
        "scale": 5,
    },
    "px320": {
        "name": "Retro Standard (320x180)",
        "viewport_width": 320,
        "viewport_height": 180,
        "window_width": 1280,
        "window_height": 720,
        "scale": 4,
    },
    "px480": {
        "name": "Retro HD (480x270)",
        "viewport_width": 480,
        "viewport_height": 270,
        "window_width": 1440,
        "window_height": 810,
        "scale": 3,
    },
    "px640": {
        "name": "Modern Pixel Art (640x360)",
        "viewport_width": 640,
        "viewport_height": 360,
        "window_width": 1920,
        "window_height": 1080,
        "scale": 3,
    },
}

# --- Architecture Patterns ---
ARCHITECTURE_PATTERNS = {
    "mvc": {
        "name": "Model-View-Controller",
        "description": "Separação por responsabilidade: models, views (scenes), controllers (scripts).",
        "folders": [
            "assets/sprites",
            "assets/sprites/characters",
            "assets/sprites/environment",
            "assets/sprites/ui",
            "assets/audio/sfx",
            "assets/audio/music",
            "assets/fonts",
            "assets/shaders",
            "models",
            "views/scenes",
            "views/ui",
            "controllers",
            "autoload",
        ],
    },
    "feature": {
        "name": "Feature-Based",
        "description": "Tudo organizado por feature/entidade. Cada feature contém seus próprios assets, scenes e scripts.",
        "folders": [
            "features/player/scenes",
            "features/player/scripts",
            "features/player/sprites",
            "features/enemy/scenes",
            "features/enemy/scripts",
            "features/enemy/sprites",
            "features/ui/scenes",
            "features/ui/scripts",
            "features/ui/sprites",
            "features/world/scenes",
            "features/world/scripts",
            "features/world/tilesets",
            "shared/assets/audio/sfx",
            "shared/assets/audio/music",
            "shared/assets/fonts",
            "shared/assets/shaders",
            "shared/scripts",
            "autoload",
        ],
    },
    "layer": {
        "name": "Layered Architecture",
        "description": "Camadas: presentation (UI/scenes), domain (game logic), data (save/load/config).",
        "folders": [
            "presentation/scenes",
            "presentation/ui",
            "presentation/sprites",
            "presentation/shaders",
            "domain/entities",
            "domain/systems",
            "domain/events",
            "data/save",
            "data/config",
            "data/resources",
            "assets/audio/sfx",
            "assets/audio/music",
            "assets/fonts",
            "autoload",
        ],
    },
    "ecs": {
        "name": "Entity-Component-System",
        "description": "Estrutura para padrão ECS: entities, components, systems separados.",
        "folders": [
            "entities",
            "components/physics",
            "components/rendering",
            "components/input",
            "components/ai",
            "components/stats",
            "systems",
            "resources/sprites",
            "resources/sprites/characters",
            "resources/sprites/environment",
            "resources/audio/sfx",
            "resources/audio/music",
            "resources/fonts",
            "resources/shaders",
            "scenes/levels",
            "scenes/ui",
            "autoload",
        ],
    },
    "minimal": {
        "name": "Minimal",
        "description": "Mínimo viável para protótipos rápidos e game jams.",
        "folders": [
            "src",
            "assets/sprites",
            "assets/audio",
            "assets/fonts",
            "scenes",
        ],
    },
    "modular": {
        "name": "Modular",
        "description": "Módulos independentes e reutilizáveis. Cada módulo é autocontido.",
        "folders": [
            "modules/core/scripts",
            "modules/core/scenes",
            "modules/gameplay/scripts",
            "modules/gameplay/scenes",
            "modules/ui/scripts",
            "modules/ui/scenes",
            "modules/ui/themes",
            "modules/audio/scripts",
            "modules/audio/resources",
            "assets/sprites",
            "assets/sprites/characters",
            "assets/sprites/environment",
            "assets/fonts",
            "assets/shaders",
            "autoload",
        ],
    },
    "standard": {
        "name": "Standard (Godot Convention)",
        "description": "Padrão amplamente adotado pela comunidade Godot.",
        "folders": [
            "scenes/actors",
            "scenes/levels",
            "scenes/ui",
            "scripts/global",
            "scripts/components",
            "scripts/resources",
            "assets/sprites/characters",
            "assets/sprites/environment",
            "assets/sprites/ui",
            "assets/audio/sfx",
            "assets/audio/music",
            "assets/fonts",
            "assets/shaders",
            "assets/themes",
            "autoload",
        ],
    },
}

PROJECT_TYPES = ["2d", "3d", "2d-top", "2d-plat"]


def generate_project_godot(project_name, project_type, preset):
    """Generate a project.godot file as plain text with pixel art settings."""
    p = PIXEL_PRESETS[preset]

    # Determine renderer based on project type
    if project_type == "3d":
        renderer = "forward_plus"
    else:
        renderer = "gl_compatibility"

    contents = f"""; Engine configuration file.
; It's best edited using the editor UI and not directly,
; since the parameters that go here are not all obvious.
;
; Format:
;   [section] ; section goes between []
;   param=value ; assign values to parameters

config_version=5

[application]

config/name="{project_name}"
config/features=PackedStringArray("4.3", "GL Compatibility")

[display]

window/size/viewport_width={p['viewport_width']}
window/size/viewport_height={p['viewport_height']}
window/size/window_width_override={p['window_width']}
window/size/window_height_override={p['window_height']}
window/stretch/mode="canvas_items"
window/stretch/aspect="keep"

[rendering]

renderer/rendering_method="{renderer}"
textures/canvas_textures/default_texture_filter=0
"""
    return contents


def generate_default_env():
    """Generate a minimal default_env.tres."""
    return """[gd_resource type="Environment" format=3]

[resource]
background_mode = 0
"""


def cmd_project_create(args):
    """Handle: pmdot --project create <type> <px_preset> <arch_pattern> [--path <dir>]"""
    print_header("PMDoT Project Scaffolding")

    if not args or args[0] == "help":
        print_project_create_help()
        return

    if len(args) < 3:
        print_error("Usage: pmdot --project create <type> <px_preset> <architecture_pattern> [--path <dir>]")
        print(f"\n  Types:    {', '.join(PROJECT_TYPES)}")
        print(f"  Presets:  {', '.join(PIXEL_PRESETS.keys())}")
        print(f"  Patterns: {', '.join(ARCHITECTURE_PATTERNS.keys())}")
        sys.exit(1)

    project_type = args[0].lower()
    px_preset = args[1].lower()
    arch_pattern = args[2].lower()

    # Parse optional --path
    project_path = os.getcwd()
    if "--path" in args:
        idx = args.index("--path")
        if idx + 1 < len(args):
            project_path = os.path.abspath(args[idx + 1])
        else:
            print_error("--path requires a directory argument.")
            sys.exit(1)

    # Validate inputs
    if project_type not in PROJECT_TYPES:
        print_error(f"Unknown project type: '{project_type}'. Valid: {', '.join(PROJECT_TYPES)}")
        sys.exit(1)

    if px_preset not in PIXEL_PRESETS:
        print_error(f"Unknown pixel preset: '{px_preset}'. Valid: {', '.join(PIXEL_PRESETS.keys())}")
        sys.exit(1)

    if arch_pattern not in ARCHITECTURE_PATTERNS:
        print_error(f"Unknown architecture pattern: '{arch_pattern}'. Valid: {', '.join(ARCHITECTURE_PATTERNS.keys())}")
        sys.exit(1)

    preset_info = PIXEL_PRESETS[px_preset]
    pattern_info = ARCHITECTURE_PATTERNS[arch_pattern]
    project_name = os.path.basename(project_path)

    print(f"  Project Name:    {Colors.BOLD}{project_name}{Colors.END}")
    print(f"  Project Type:    {Colors.BOLD}{project_type}{Colors.END}")
    print(f"  Pixel Preset:    {Colors.BOLD}{preset_info['name']}{Colors.END}")
    print(f"    Base:          {preset_info['viewport_width']}x{preset_info['viewport_height']}")
    print(f"    Window:        {preset_info['window_width']}x{preset_info['window_height']} ({preset_info['scale']}x upscale)")
    print(f"  Architecture:    {Colors.BOLD}{pattern_info['name']}{Colors.END}")
    print(f"    {Colors.DIM}{pattern_info['description']}{Colors.END}")
    print(f"  Path:            {Colors.BOLD}{project_path}{Colors.END}")
    print()

    # Create project directory
    os.makedirs(project_path, exist_ok=True)

    # Create folder structure
    folders_created = 0
    for folder in pattern_info["folders"]:
        full_path = os.path.join(project_path, folder)
        os.makedirs(full_path, exist_ok=True)
        # Create .gdkeep so Git tracks empty directories
        gdkeep = os.path.join(full_path, ".gdkeep")
        if not os.path.exists(gdkeep):
            with open(gdkeep, "w", encoding="utf-8") as f:
                f.write("")
        folders_created += 1

    # Generate project.godot
    project_godot_path = os.path.join(project_path, "project.godot")
    if not os.path.exists(project_godot_path):
        with open(project_godot_path, "w", encoding="utf-8") as f:
            f.write(generate_project_godot(project_name, project_type, px_preset))
        print_success(f"Created project.godot ({preset_info['name']})")
    else:
        print_warning("project.godot already exists, skipping.")

    # Generate default_env.tres
    env_path = os.path.join(project_path, "default_env.tres")
    if not os.path.exists(env_path):
        with open(env_path, "w", encoding="utf-8") as f:
            f.write(generate_default_env())
        print_success("Created default_env.tres")

    # Generate .gitignore for the game project
    gitignore_path = os.path.join(project_path, ".gitignore")
    if not os.path.exists(gitignore_path):
        with open(gitignore_path, "w", encoding="utf-8") as f:
            f.write(
                "# Godot\n"
                ".godot/\n"
                "*.import\n"
                "\n"
                "# Export\n"
                "export/\n"
                "*.pck\n"
                "\n"
                "# OS\n"
                ".DS_Store\n"
                "Thumbs.db\n"
            )
        print_success("Created .gitignore")

    print_success(f"Created {folders_created} directories ({pattern_info['name']} pattern)")

    # Print folder tree summary
    print(f"\n{Colors.BOLD}Project Structure:{Colors.END}")
    print(f"  {project_name}/")
    for folder in pattern_info["folders"]:
        depth = folder.count("/")
        indent = "  " * (depth + 1)
        name = folder.split("/")[-1]
        connector = "├── " if folder != pattern_info["folders"][-1] else "└── "
        # Simple tree: show full relative path
        print(f"    {Colors.DIM}{folder}/{Colors.END}")

    print(f"\n{Colors.GREEN}{Colors.BOLD}Project '{project_name}' created successfully!{Colors.END}")
    print(f"Open with: {Colors.CYAN}.\\pmdot run -- --path \"{project_path}\"{Colors.END}")


def print_project_create_help():
    """Print detailed help for --project create."""
    print(f"\n{Colors.BOLD}Usage:{Colors.END} pmdot --project create <type> <px_preset> <arch_pattern> [--path <dir>]\n")

    print(f"{Colors.BOLD}Project Types:{Colors.END}")
    for t in PROJECT_TYPES:
        print(f"  {Colors.CYAN}{t:12s}{Colors.END}")

    print(f"\n{Colors.BOLD}Pixel Art Presets:{Colors.END}")
    print(f"  {'Preset':<10} {'Base Resolution':<18} {'Window Override':<18} {'Scale'}")
    print(f"  {'─'*10} {'─'*18} {'─'*18} {'─'*5}")
    for key, p in PIXEL_PRESETS.items():
        base = f"{p['viewport_width']}x{p['viewport_height']}"
        window = f"{p['window_width']}x{p['window_height']}"
        print(f"  {Colors.CYAN}{key:<10}{Colors.END} {base:<18} {window:<18} {p['scale']}x")

    print(f"\n{Colors.BOLD}Architecture Patterns:{Colors.END}")
    for key, p in ARCHITECTURE_PATTERNS.items():
        print(f"  {Colors.CYAN}{key:<12}{Colors.END} {p['name']}")
        print(f"  {' '*12} {Colors.DIM}{p['description']}{Colors.END}")

    print(f"\n{Colors.BOLD}Examples:{Colors.END}")
    print(f"  pmdot --project create 2d px320 mvc")
    print(f"  pmdot --project create 2d-plat px640 feature --path D:\\MyGame")
    print(f"  pmdot --project create 2d px128 ecs")
    print(f"  pmdot --project create 2d-top px256 standard")


# =============================================================================
# Feature 2: Incremental Segmented Build
# =============================================================================

BUILD_SEGMENTS = {
    "godot_core": {
        "path": "core",
        "description": "Godot Engine Core",
    },
    "godot_scene": {
        "path": "scene",
        "description": "Scene System",
    },
    "godot_servers": {
        "path": "servers",
        "description": "Rendering, Physics, Audio Servers",
    },
    "godot_editor": {
        "path": "editor",
        "description": "Editor UI",
    },
    "godot_drivers": {
        "path": "drivers",
        "description": "GPU / Audio Drivers",
    },
    "godot_platform": {
        "path": "platform",
        "description": "Platform-Specific Code",
    },
    "godot_main": {
        "path": "main",
        "description": "Main Entry Point",
    },
    "godot_thirdparty": {
        "path": "thirdparty",
        "description": "Third-Party Libraries",
    },
    "godot_modules": {
        "path": "modules",
        "description": "Godot Built-in Modules (excl. pmdot_*)",
        "exclude_prefix": "pmdot_",
    },
    "pmdot_aseprite": {
        "path": os.path.join("modules", "pmdot_aseprite"),
        "description": "PMDoT Aseprite Module",
    },
}

SOURCE_EXTENSIONS = {".cpp", ".c", ".h", ".hpp", ".py", ".inc"}


def compute_segment_hash(segment_key):
    """Compute a combined SHA256 hash for all source files in a segment."""
    segment = BUILD_SEGMENTS[segment_key]
    segment_path = os.path.join(GODOT_DIR, segment["path"])
    exclude_prefix = segment.get("exclude_prefix", None)

    if not os.path.exists(segment_path):
        return None

    hasher = hashlib.sha256()
    file_count = 0

    for root, dirs, files in os.walk(segment_path):
        # If this segment has an exclude prefix, skip matching subdirectories
        if exclude_prefix:
            dirs[:] = [d for d in dirs if not d.startswith(exclude_prefix)]

        # Sort for deterministic ordering
        for filename in sorted(files):
            ext = os.path.splitext(filename)[1].lower()
            if ext in SOURCE_EXTENSIONS:
                filepath = os.path.join(root, filename)
                try:
                    with open(filepath, "rb") as f:
                        content = f.read()
                    hasher.update(filepath.encode("utf-8"))
                    hasher.update(content)
                    file_count += 1
                except (OSError, IOError):
                    pass

    if file_count == 0:
        return None

    return hasher.hexdigest()


def load_build_cache():
    """Load the build segments cache from disk."""
    if os.path.exists(BUILD_SEGMENTS_FILE):
        try:
            with open(BUILD_SEGMENTS_FILE, "r", encoding="utf-8") as f:
                return json.load(f)
        except (json.JSONDecodeError, OSError):
            pass
    return {}


def save_build_cache(cache):
    """Save the build segments cache to disk."""
    ensure_cache()
    with open(BUILD_SEGMENTS_FILE, "w", encoding="utf-8") as f:
        json.dump(cache, f, indent=2)


def detect_changed_segments(force=False, only=None):
    """
    Compare current segment hashes against cached hashes.
    Returns (changed_segments, current_hashes).
    """
    cached = load_build_cache()
    current_hashes = {}
    changed = []

    segments_to_check = BUILD_SEGMENTS.keys()
    if only:
        segments_to_check = [s.strip() for s in only if s.strip() in BUILD_SEGMENTS]

    for key in segments_to_check:
        h = compute_segment_hash(key)
        current_hashes[key] = h
        if force or h is None or h != cached.get(key):
            changed.append(key)

    return changed, current_hashes


def cmd_build(extra_args=None):
    """Build PMDoT Engine with incremental segment detection."""
    print_header("PMDoT Incremental Build System")

    if extra_args is None:
        extra_args = []

    os_name = get_os_platform()
    platform_target = os_name
    if os_name == "linux":
        platform_target = "linuxbsd"

    # Parse build flags
    force_build = "--force" in extra_args
    only_segments = None

    clean_args = []
    i = 0
    while i < len(extra_args):
        if extra_args[i] == "--force":
            i += 1
            continue
        elif extra_args[i] == "--only":
            if i + 1 < len(extra_args):
                only_segments = extra_args[i + 1].split(",")
                i += 2
                continue
            else:
                print_error("--only requires a comma-separated list of segments.")
                print(f"  Available segments: {', '.join(BUILD_SEGMENTS.keys())}")
                sys.exit(1)
        else:
            clean_args.append(extra_args[i])
        i += 1

    # Detect changes
    start_time = time.time()
    print_info("Scanning source segments for changes...")

    changed, current_hashes = detect_changed_segments(force=force_build, only=only_segments)

    scan_time = time.time() - start_time

    # Report segment status
    segments_to_show = only_segments if only_segments else BUILD_SEGMENTS.keys()
    print(f"\n  {'Segment':<22} {'Status':<14} Description")
    print(f"  {'─'*22} {'─'*14} {'─'*35}")
    for key in segments_to_show:
        if key not in BUILD_SEGMENTS:
            continue
        seg = BUILD_SEGMENTS[key]
        if key in changed:
            status = f"{Colors.YELLOW}CHANGED{Colors.END}"
        else:
            status = f"{Colors.GREEN}OK{Colors.END}"
        print(f"  {key:<22} {status:<23} {seg['description']}")

    print(f"\n  {Colors.DIM}Scan completed in {scan_time:.2f}s{Colors.END}")

    if not changed:
        print(f"\n{Colors.GREEN}{Colors.BOLD}Build is up to date. No segments changed.{Colors.END}")
        print(f"  Use {Colors.CYAN}pmdot build --force{Colors.END} to force a full rebuild.")
        return

    print(f"\n  {Colors.YELLOW}{len(changed)} segment(s) changed:{Colors.END} {', '.join(changed)}")

    if force_build:
        print(f"  {Colors.MAGENTA}--force{Colors.END} flag: full rebuild requested")

    # Run SCons
    print_header("Running SCons Build")

    scons_cmd = [
        "scons",
        f"platform={platform_target}",
        "target=editor",
        "disable_exceptions=no",
        "custom_modules=modules/pmdot_aseprite",
        "-j8",
    ]
    scons_cmd.extend(clean_args)

    print(f"  {Colors.DIM}$ {' '.join(scons_cmd)}{Colors.END}\n")

    build_start = time.time()
    success = run_command(scons_cmd, cwd=GODOT_DIR, check=False)
    build_time = time.time() - build_start

    if success:
        # Update cache with current hashes
        cache = load_build_cache()
        cache.update(current_hashes)
        save_build_cache(cache)
        print_success(f"PMDoT Engine build finished successfully! ({build_time:.1f}s)")
    else:
        print_error(f"Build failed after {build_time:.1f}s. Segment cache NOT updated.")
        print(f"  Fix the errors above, then run {Colors.CYAN}pmdot build{Colors.END} again.")
        sys.exit(1)


# =============================================================================
# Existing Commands (preserved from original)
# =============================================================================

def cmd_install():
    print_header("PMDoT Environment & Dependencies Installer")
    os_name = get_os_platform()
    print(f"Detected OS Platform: {Colors.BOLD}{os_name.upper()}{Colors.END} ({platform.machine()})")

    ensure_cache()

    if shutil.which("git"):
        print_success("Git is installed")
    else:
        print_error("Git not found. Please install Git: https://git-scm.com/")

    print_success(f"Python {sys.version.split()[0]} is active")

    if shutil.which("scons"):
        print_success("SCons build tool is installed")
    else:
        print_warning("SCons not found. Installing SCons via pip...")
        subprocess.run([sys.executable, "-m", "pip", "install", "scons"], check=True)
        print_success("SCons installed successfully")

    if shutil.which("cmake"):
        print_success("CMake is installed")
    else:
        print_warning("CMake not found in PATH")

    if shutil.which("ninja"):
        print_success("Ninja is installed")
    else:
        print_warning("Ninja not found in PATH")

    if os_name == "windows":
        vs_path = os.path.expandvars(
            "%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe"
        )
        if os.path.exists(vs_path):
            print_success("Visual Studio C++ Build Tools detected")
        else:
            print_warning(
                "Visual Studio C++ compiler (MSVC) recommended for Windows builds"
            )

    elif os_name == "linux":
        if shutil.which("gcc") and shutil.which("g++"):
            print_success("GCC / G++ compilers detected")
        else:
            print_warning(
                "Run: sudo apt-get install build-essential pkg-config libx11-dev libxcursor-dev libxinerama-dev libgl1-mesa-dev"
            )

    elif os_name == "macos":
        if shutil.which("clang"):
            print_success("Apple Clang / Xcode Command Line Tools detected")
        else:
            print_warning("Run: xcode-select --install")

    print_header("Submodule Initialization")
    if os.path.exists(GODOT_DIR):
        print_success("source-godot (Godot 4.3-stable) repository present")
    else:
        print("Cloning Godot 4.3-stable source...")
        run_command(
            [
                "git",
                "clone",
                "--recursive",
                "-b",
                "4.3-stable",
                "https://github.com/godotengine/godot.git",
                "source-godot",
            ],
            cwd=ROOT_DIR,
        )

    aseprite_sub_dir = os.path.join(MODULE_DIR, "thirdparty", "aseprite")
    if os.path.exists(aseprite_sub_dir):
        print_success("Aseprite thirdparty submodules present")
    else:
        print("Cloning Aseprite source and submodules...")
        os.makedirs(os.path.dirname(aseprite_sub_dir), exist_ok=True)
        run_command(
            [
                "git",
                "clone",
                "--recursive",
                "https://github.com/aseprite/aseprite.git",
                aseprite_sub_dir,
            ],
            cwd=ROOT_DIR,
        )

    print_header("Installation Check Complete!")


def cmd_version():
    print_header("PMDoT Engine Version Info")
    print(
        f"PMDoT Version:            {Colors.BOLD}{VERSION_PMDOT}{Colors.END}"
    )
    print(
        f"Godot Base Release:       {Colors.BOLD}{VERSION_GODOT}{Colors.END}"
    )
    print(
        f"Aseprite Module:          {Colors.BOLD}pmdot_aseprite v{VERSION_ASEPRITE_MODULE}{Colors.END}"
    )
    print(
        f"Detected System OS:       {Colors.BOLD}{get_os_platform().upper()}{Colors.END} ({platform.machine()})"
    )

    if os.path.exists(CACHE_FILE):
        with open(CACHE_FILE, "r", encoding="utf-8") as f:
            cache = json.load(f)
            print(f"Cached Config:            {cache}")


def cmd_update():
    print_header("Updating PMDoT Repositories & Submodules")
    print("Pulling PMDoT repository updates...")
    run_command(["git", "pull"], cwd=ROOT_DIR, check=False)

    if os.path.exists(GODOT_DIR):
        print("Updating Godot submodules...")
        run_command(
            ["git", "submodule", "update", "--init", "--recursive"],
            cwd=GODOT_DIR,
            check=False,
        )

    aseprite_sub_dir = os.path.join(MODULE_DIR, "thirdparty", "aseprite")
    if os.path.exists(aseprite_sub_dir):
        print("Updating Aseprite submodules...")
        run_command(
            ["git", "submodule", "update", "--init", "--recursive"],
            cwd=aseprite_sub_dir,
            check=False,
        )

    print_success("PMDoT source tree updated successfully!")


def cmd_run():
    print_header("Launching PMDoT Engine Editor")
    os_name = get_os_platform()

    bin_dir = os.path.join(GODOT_DIR, "bin")
    exec_name = None

    if os_name == "windows":
        exec_name = os.path.join(bin_dir, "godot.windows.editor.x86_64.exe")
    elif os_name == "linux":
        exec_name = os.path.join(bin_dir, "godot.linuxbsd.editor.x86_64")
    elif os_name == "macos":
        exec_name = os.path.join(bin_dir, "godot.macos.editor.x86_64")

    if exec_name and os.path.exists(exec_name):
        print(f"Running: {exec_name} --editor")
        subprocess.run([exec_name, "--editor"])
    else:
        print_error(f"Executable not found: {exec_name}")
        print(
            "Run 'python pmdot.py build' or 'pmdot build' first to compile the engine."
        )


def cmd_doctor():
    print_header("PMDoT System Doctor & Diagnostics")
    os_name = get_os_platform()
    print(f"Operating System:   {os_name.upper()} ({platform.platform()})")
    print(f"Python Executable:  {sys.executable} ({sys.version.split()[0]})")
    print(f"Git Path:           {shutil.which('git') or 'Not Found'}")
    print(f"SCons Path:         {shutil.which('scons') or 'Not Found'}")
    print(f"CMake Path:         {shutil.which('cmake') or 'Not Found'}")
    print(f"Ninja Path:         {shutil.which('ninja') or 'Not Found'}")
    print(f"Godot Directory:    {os.path.exists(GODOT_DIR)}")
    print(f"PMDoT Module:       {os.path.exists(MODULE_DIR)}")

    # Show build segment cache status
    print_header("Build Segment Cache")
    cache = load_build_cache()
    if cache:
        for key in BUILD_SEGMENTS:
            status = "cached" if key in cache else "not cached"
            print(f"  {key:<22} {status}")
    else:
        print("  No build cache found. Run 'pmdot build' to initialize.")


def cmd_clean():
    print_header("Cleaning Build Cache & Artifacts")
    scons_cache = os.path.join(GODOT_DIR, ".sconsign.dblite")
    if os.path.exists(scons_cache):
        os.remove(scons_cache)
        print_success("Removed .sconsign.dblite")

    temp_sconf = os.path.join(GODOT_DIR, ".sconf_temp")
    if os.path.exists(temp_sconf):
        shutil.rmtree(temp_sconf)
        print_success("Removed .sconf_temp")

    if os.path.exists(BUILD_SEGMENTS_FILE):
        os.remove(BUILD_SEGMENTS_FILE)
        print_success("Removed build_segments.json cache")

    print_success("Clean operation finished!")


# =============================================================================
# Help & Main
# =============================================================================

def print_help():
    print(f"{Colors.BOLD}{Colors.CYAN}PMDoT Engine Management CLI v{VERSION_PMDOT}{Colors.END}")
    print(f"Usage: pmdot <command> [options]\n")

    print(f"{Colors.BOLD}Core Commands:{Colors.END}")
    print("  install, -i              Verify and install all system requirements and submodules")
    print("  version, -v              Check PMDoT and Godot release version information")
    print("  update, -u               Update PMDoT repositories and submodules to latest release")
    print("  doctor, -d               Run diagnostic report on tools and SDKs")

    print(f"\n{Colors.BOLD}Build Commands:{Colors.END}")
    print("  build, -b                Build PMDoT Engine (incremental segmented)")
    print("  build --force            Force full rebuild ignoring cache")
    print("  build --only <segments>  Rebuild only specific segments (comma-separated)")
    print("  clean, -c                Clean temporary build cache and logs")

    print(f"\n{Colors.BOLD}Project Commands:{Colors.END}")
    print("  --project create <type> <px_preset> <arch_pattern> [--path <dir>]")
    print("                           Create a new Godot project with scaffolding")
    print("  --project create help    Show detailed project creation help")

    print(f"\n{Colors.BOLD}Run Commands:{Colors.END}")
    print("  run, -r                  Launch the compiled PMDoT Engine editor")

    print(f"\n{Colors.BOLD}Help:{Colors.END}")
    print("  help, -h                 Show this help message")

    print(f"\n{Colors.BOLD}Examples:{Colors.END}")
    print(f"  {Colors.CYAN}pmdot install{Colors.END}")
    print(f"  {Colors.CYAN}pmdot build{Colors.END}")
    print(f"  {Colors.CYAN}pmdot build --force{Colors.END}")
    print(f"  {Colors.CYAN}pmdot build --only pmdot_aseprite,godot_editor{Colors.END}")
    print(f"  {Colors.CYAN}pmdot --project create 2d px320 mvc{Colors.END}")
    print(f"  {Colors.CYAN}pmdot --project create 2d-plat px640 feature --path D:\\MyGame{Colors.END}")
    print(f"  {Colors.CYAN}pmdot run{Colors.END}")
    print(f"\n  See {Colors.CYAN}architecture-guidelines/cli_reference.md{Colors.END} for full documentation.")


def main():
    if len(sys.argv) < 2:
        print_help()
        sys.exit(0)

    arg = sys.argv[1].lower()
    extra = sys.argv[2:]

    # Handle --project flag
    if arg == "--project":
        if len(extra) < 1:
            print_error("Usage: pmdot --project create <type> <px_preset> <architecture_pattern>")
            sys.exit(1)
        subcmd = extra[0].lower()
        if subcmd == "create":
            cmd_project_create(extra[1:])
        else:
            print_error(f"Unknown project subcommand: '{subcmd}'. Use 'create'.")
            sys.exit(1)
    elif arg in ["install", "-i"]:
        cmd_install()
    elif arg in ["version", "-v"]:
        cmd_version()
    elif arg in ["update", "-u"]:
        cmd_update()
    elif arg in ["build", "-b"]:
        cmd_build(extra)
    elif arg in ["run", "-r"]:
        cmd_run()
    elif arg in ["doctor", "-d"]:
        cmd_doctor()
    elif arg in ["clean", "-c"]:
        cmd_clean()
    elif arg in ["help", "-h", "--help"]:
        print_help()
    else:
        print_error(f"Unknown command: {arg}")
        print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
