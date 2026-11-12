# Changes — PMDoT Modification Log

> Every structural modification to the repository (module removal, module addition, build system changes) **must** be recorded here, ordered from newest to oldest.
> Record schema: Date · ID · Status (`PLANNED` | `IN PROGRESS` | `COMPLETED` | `REVERTED`) · Affected files · Reason · Validation.

---

## 2026-11-12 · CHG-005 · Added React support for building UI in Godot · `IN PROGRESS`

**Reason:** PMDoT introduces React as a declarative UI development layer, allowing developers to build and manage user interfaces using JSX/TSX instead of manually constructing UI nodes through code. By integrating QuickJS as JavaScript runtime and implementing a custom React Renderer, React components are converting into native C++ Ui Tree managed by PMDoT.   
  
**Execution Scope (by Subsystem):**

| Subsystem | Status | Modification |
|---|---|---|
| React Base Components | ✅ | Implemented five foundational React components: `Text`, `Image`, `View`, `CustomShader`, and `Scroll`. |
| C++ Core Structure | ✅ | Created the foundational C++ source files for the UI system. |
| Third-Party Dependencies | ✅ | Integrated esbuild for TSX bundling, QuickJS as the JavaScript runtime, and React with React Reconciler. |
| Custom React Renderer | ⏳ | Implement a custom React Renderer and establish communication with the native C++ layer. |
| Native UI Tree Implementation | ⏳ | Implement the C++ logic responsible for creating and managing the native UI tree. |
| Godot Rendering Integration | ⏳ | Integrate the UI tree with Godot's `TextServer` and `RenderingServer` APIs. |

---

## 2026-11-07 · CHG-004 · Complete Removal of Mobile (Android, iOS) & Web Platforms · `COMPLETED`

**Reason:** PMDoT is strictly a desktop-oriented game engine optimized for high-performance Pixel Art development. Mobile targets (Android, iOS) and Web (HTML5/WebAssembly) added extreme maintenance overhead, thousands of platform files, external toolchain dependencies (Gradle, NDK, Xcode, Emscripten), and complex preprocessor branches across core and rendering drivers. Their elimination dramatically reduces codebase surface area, cuts compilation time, and simplifies reviewer cognitive load.

**Execution Scope (by Subsystem):**

| Subsystem | Status | Modification |
|---|---|---|
| Platform Purge | ✅ | Deleted `source-godot/platform/android/` (Java/Gradle wrappers, JNI bindings, OpenSL audio, export plugin, permissions, ~5,000 files). |
| Platform Purge | ✅ | Deleted `source-godot/platform/ios/` (Xcode project templates, CocoaTouch display server, CoreAudio driver, export plugin, codesigning scripts). |
| Platform Purge | ✅ | Deleted `source-godot/platform/web/` (Emscripten helpers, JS runtime libraries, AudioWorklet, WebGL2 glue, HTTP server, ~22,000 lines). |
| Modules Purge | ✅ | Deleted `source-godot/modules/webxr/` (WebXR OpenXR translation layer). Cleaned `openxr` Android extension references. |
| Core Preprocessors | ✅ | Purged `ANDROID_ENABLED`, `IOS_ENABLED`, and `WEB_ENABLED` across `core/`, `drivers/gles3/`, `servers/`, `modules/mono/`, `modules/websocket/`. |
| Project Creation | ✅ | `source-godot/editor/project_manager/project_dialog.cpp`: Removed the "Mobile" renderer checkbox option. Renderers for new projects are strictly desktop (**Forward+** and **Compatibility**). Cleaned descriptions to remove references to mobile and web platforms. |
| Editor Settings | ✅ | `editor_settings.cpp`: Enum `project_manager/default_renderer` narrowed to `"forward_plus,gl_compatibility"`. Removed `web_macos` and `web_ios` shortcut hacks. |
| Editor UI & Restarts | ✅ | `editor_node.cpp`: Purged restart dialog strings and tooltips referencing mobile/web platforms. Stripped `.mobile` override synchronization. |
| Debugger & DAP | ✅ | `editor/debugger/debug_adapter/debug_adapter_parser.cpp`: Generic export platform lookup, removing hardcoded Android and Web special casing. Cleaned tooltips in `debugger_editor_plugin.cpp`. |
| GDExtension Export | ✅ | `editor/plugins/gdextension_export_plugin.h`: Removed Android `.aar` plugin checks, `wasm32` arch tag, and iOS `.xcframework` dynamic linker wrappers. |
| ProjectSettings & Main | ✅ | `main/main.cpp`: Removed `--install-android-build-template`, `rendering/renderer/rendering_method.mobile`, `rendering_method.web`, `driver.android`, `driver.ios`, `driver.web`, `display/window/ios/*`, and `input_devices/pointing/android/*`. |
| ProjectSettings Core | ✅ | `core/config/project_settings.cpp`: Removed `default_playback_type.web` and `audio/general/ios/*`. Added C#, C++, Java, Markdown to default script search extensions. |
| SCons Build Tools | ✅ | `SConstruct`, `methods.py`, `core/SCsub`: Cleaned platform tool detection for android/web, darwin SDK paths, and removed obsolete `iphone` and `javascript` aliases. |
| Metadata & CI | ✅ | Purged mobile and web workflows from CI (`.github/workflows/`), updated `.github/CODEOWNERS`, and refreshed READMEs. |

---

## 2026-11-07 · CHG-003 · Multi-Language Script Creation Defaults (C#, C++, Java, Markdown) · `COMPLETED`

**Reason:** Following the removal of GDScript (CHG-002), script authoring in the PMDoT editor must provide seamless out-of-the-box workflows for C# and C++ (GDExtension), early template scaffolding for roadmap-targeted Java, and project notes via Markdown, without requiring any legacy `.gd` language definitions.

**Execution Scope:**

| Subsystem | Status | Modification |
|---|---|---|
| Script Create Dialog | ✅ | `editor/script_create_dialog.cpp`: Added C#, C++, Java, and Markdown as default selectable language options in `language_menu`. |
| Language Templates | ✅ | Built-in standard class boilerplate provided for C# (`Godot.Node`), C++ (`godot::Node`), Java (with development/roadmap warning banner), and Markdown (`.md` notes). |
| Editor Docks | ✅ | `scene_tree_dock.cpp` & `filesystem_dock.cpp`: Removed gating checks that required registered script language count > 0 to allow script creation. |
| Script File Extensions | ✅ | Proper casing and file extension generation: `.cs` (PascalCase), `.cpp` (snake_case), `.java` (PascalCase), `.md` (snake_case). |

---

## 2026-11-06 · CHG-002 · Complete Removal of GDScript & JSON-RPC · `COMPLETED`

**Reason:** GDScript is no longer used for game development within PMDoT. The repository runtime is focused on **C#** and **C++ (GDExtension and native modules)**, with future roadmap planning for **Java**. The removal wiped 1,343 versioned GDScript files and 9 JSON-RPC files (~71 MB on disk), eliminating heavy parser/compiler overhead and significantly reducing engine compilation time.

**Execution Scope (by Phase):**

| Phase | Status | Modification |
|---|---|---|
| F0 | ✅ | Baseline: verification of work branch `GDSCRIPT-KILLING`. |
| F1 | ✅ | `pmdot.py` & `build_pmdot.py`: added compilation flags `module_gdscript_enabled=no` and `module_jsonrpc_enabled=no`. |
| F2 | ✅ | Deleted `source-godot/modules/gdscript/` (1,343 versioned files + compiled object artifacts). |
| F2 | ✅ | Deleted `source-godot/modules/jsonrpc/` (direct dependency used solely by GDScript LSP). |
| F2 | ✅ | Deleted `misc/dist/document_icons/gdscript*.svg` and `misc/dist/macos_tools.app/Contents/Resources/GDScript.icns`. |
| F2 | ✅ | `scu_builders.py` (lines 327–329 removed); `.github/CODEOWNERS` (lines 119–120 removed); `.gd` MIME types removed from Linux XML and macOS plists. |
| F3 | ✅ | `main/main.cpp`: cleaned all `#ifdef MODULE_GDSCRIPT_ENABLED` blocks (LSP, `--lsp-port`, `--gdscript-docs`, `get_files_with_extension`). |
| F3 | ✅ | `editor/editor_help.cpp/.h` & `editor/editor_node.cpp`: stripped `LANGUAGE_GDSCRIPT` enum and GDScript headers, retaining C# (`MODULE_MONO_ENABLED`). |
| F3 | ✅ | `editor/code_editor.cpp`: clean fallback for string and editor theme colors (removed obsolete calls to `text_editor/theme/highlighting/gdscript/*`). |
| F3 | ✅ | `editor/plugins/script_text_editor.cpp`: safe fallback lookup for `debug/gdscript/warnings/enable` via `ProjectSettings::get_setting()`. |
| F4 | ✅ | `tests/core/config/test_project_settings.h`: decoupled static dependency on `"GDScript Integration Test Suite"` by assigning dynamically during runtime tests. |
| F4 | ✅ | `doc/classes/ProjectSettings.xml`: deleted 50+ orphaned documentation nodes under `<member name="debug/gdscript/*">`. |
| F5 | ✅ | SCons and codebase validation (SConscript parsing passes cleanly without orphaned module references). |

**Retained Components (Preserved for Compatibility):**
- `TextServer::STRUCTURED_TEXT_GDSCRIPT` (public enum API exposed across GDExtension bindings).
- Example `[gdscript]` doc tags within internal engine documentation XMLs.
- `project_converter_3_to_4` and generic script architecture bindings (`ScriptServer`/`ScriptLanguage`) required by C# and GDExtension.

**Decisions and Target Runtimes:**
- Currently supported gameplay languages: **C#** and **C++**.
- Future roadmap: **Java** support.
- `modules/jsonrpc`: Cleaned up along with GDScript.

---

## 2026-11-06 · CHG-001 · Creation of the *PMDoT Architechture Guide* · `COMPLETED`

**Created Files:**
- `PMDoT Architechture Guide/Changes.md`
- `PMDoT Architechture Guide/Folders Path creation guide.md`
- `PMDoT Architechture Guide/Test guide.md`
- `PMDoT Architechture Guide/Architecture.md`
- `PMDoT Architechture Guide/Modules.md`

**Reason:** Centralize architectural specifications, module inventories, testing standards, and historical modifications into an authoritative guide.
**Note:** `architecture-guidelines/` (Aseprite SIMD and CLI reference) remains active and is cross-referenced from this guide.
