# Changes — PMDoT Modification Log

> Every structural modification to the repository (module removal, module addition, build system changes) **must** be recorded here, ordered from newest to oldest.
> Record schema: Date · ID · Status (`PLANNED` | `IN PROGRESS` | `COMPLETED` | `REVERTED`) · Affected files · Reason · Validation.

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
