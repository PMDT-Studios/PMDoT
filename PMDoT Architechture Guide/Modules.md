# Modules — Module Inventory (`source-godot/modules/`)

Status Key: ✅ Active · 🟣 Custom PMDoT · ⛔ Disabled · 🗑️ Purged / Removed (CHG-002)

---

## 1. PMDoT Modules

| Module | Status | Description | Key Files |
|---|---|---|---|
| `pmdot_aseprite` | 🟣 ✅ | Embedded native Aseprite workspace in the editor, real-time SIMD pixel conversion pipeline, and Pixel Art presets | `register_types.cpp`, `pmdot_pixel_pipeline.cpp/.h`, `editor/aseprite_workspace.cpp/.h`, `editor/pixel_art_presets.cpp/.h`, `thirdparty/aseprite` |

For in-depth technical details, see `architecture-guidelines/aseprite_architecture_guide.md`.

---

## 2. Scripting & Language Modules

| Module | Status | Description | Notes |
|---|---|---|---|
| `gdscript` | 🗑️ | Legacy GDScript language (tokenizer, parser, analyzer, compiler, VM, LSP, templates) | Completely removed (~71 MB, 1,343 files). See `Changes.md` (CHG-002). |
| `jsonrpc` | 🗑️ | JSON-RPC protocol implementation | Removed; sole consumer was GDScript language server protocol. |
| `mono` | 🔄 | C#/.NET engine integration (`module_mono_enabled=yes`) | Primary supported scripting language alongside native C++. |

---

## 3. Retained Upstream Godot Modules

| Category | Modules |
|---|---|
| Image / Texture | `astcenc`, `basis_universal`, `bmp`, `cvtt`, `dds`, `etcpak`, `hdr`, `jpg`, `ktx`, `squish`, `svg`, `tga`, `tinyexr`, `webp` |
| Audio | `interactive_music`, `minimp3`, `ogg`, `vorbis` |
| Video | `theora` |
| Typography / Font | `freetype`, `msdfgen`, `text_server_adv`, `text_server_fb` |
| 3D Scene / Import | `csg`, `fbx`, `gltf`, `gridmap`, `lightmapper_rd`, `meshoptimizer`, `raycast`, `vhacd`, `xatlas_unwrap` |
| Navigation / Math | `navigation`, `noise` |
| Networking | `enet`, `mbedtls`, `multiplayer`, `upnp`, `webrtc`, `websocket` |
| XR | `mobile_vr`, `openxr`, `webxr` |
| System Utilities | `camera`, `glslang`, `regex`, `zip` |

> [!NOTE]
> `websocket` is **retained** despite GDScript LSP removal, as it serves multiplayer networking, WebRTC signaling, and the remote debugger.

---

## 4. Module Registration Checklist

When creating a new module, add a record to Section 1 and log the change in `Changes.md`:

```markdown
| `pmdot_<name>` | 🟣 ✅ | <short description> | <key files> |
```

Verification Checklist:
- [ ] `config.py` with `can_build()`, `configure()`, and explicit dependency declarations (`env.module_add_dependencies`).
- [ ] `SCsub` appending sources to `env.modules_sources`.
- [ ] `register_types.cpp/.h` providing `initialize_pmdot_<name>_module` and `uninitialize_pmdot_<name>_module`.
- [ ] Unit tests in `tests/test_<name>.h` (see `Test guide.md`).
- [ ] Added to `custom_modules` and `BUILD_SEGMENTS` in `pmdot.py`.
