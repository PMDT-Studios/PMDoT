# Modules — Inventário de Módulos (`source-godot/modules/`)

Legenda de status: ✅ ativo · 🟣 custom PMDoT · ⛔ desabilitado · 🗑️ remoção planejada (CHG-002)

## 1. Módulos PMDoT

| Módulo | Status | Descrição | Arquivos-chave |
|---|---|---|---|
| `pmdot_aseprite` | 🟣 ✅ | Workspace Aseprite embutido no editor, pipeline SIMD de pixels e presets de Pixel Art | `register_types.cpp`, `pmdot_pixel_pipeline.cpp/.h`, `editor/aseprite_workspace.cpp/.h`, `editor/pixel_art_presets.cpp/.h`, `thirdparty/aseprite` |

Detalhes: `architecture-guidelines/aseprite_architecture_guide.md`.

## 2. Linguagens de script

| Módulo | Status | Descrição | Observação |
|---|---|---|---|
| `gdscript` | 🗑️ | Linguagem GDScript: tokenizer, parser, analyzer, compiler, VM, editor (highlighter, docgen, templates) e LSP | Cerca de 71 MB, 1.343 arquivos versionados. Ver `Changes.md` › CHG-002 |
| `jsonrpc` | 🗑️ (decisão) | Classe `JSONRPC`. Único consumidor: o LSP do GDScript | Dependência declarada em `gdscript/config.py` |
| `mono` | 🔄 | C#/.NET (`module_mono_enabled=yes`). Linguagem de gameplay suportada juntamente com C++ | Em processo de ativação/configuração |

## 3. Módulos Godot built-in mantidos

| Categoria | Módulos |
|---|---|
| Imagem / textura | `astcenc`, `basis_universal`, `bmp`, `cvtt`, `dds`, `etcpak`, `hdr`, `jpg`, `ktx`, `squish`, `svg`, `tga`, `tinyexr`, `webp` |
| Áudio | `interactive_music`, `minimp3`, `ogg`, `vorbis` |
| Vídeo | `theora` |
| Fontes / texto | `freetype`, `msdfgen`, `text_server_adv`, `text_server_fb` |
| Cena 3D / import | `csg`, `fbx`, `gltf`, `gridmap`, `lightmapper_rd`, `meshoptimizer`, `raycast`, `vhacd`, `xatlas_unwrap` |
| Navegação / ruído | `navigation`, `noise` |
| Rede | `enet`, `mbedtls`, `multiplayer`, `upnp`, `webrtc`, `websocket` |
| XR | `mobile_vr`, `openxr`, `webxr` |
| Utilitários | `camera`, `glslang`, `regex`, `zip` |

> [!NOTE]
> `websocket` **não** sai junto com o GDScript: ele é usado também por `multiplayer`/`webrtc` e pelo debugger remoto.

## 4. Template para registrar um novo módulo

Ao criar um módulo, adicione uma linha na seção 1 e registre a criação em `Changes.md`.

```
| `pmdot_<nome>` | 🟣 ✅ | <descrição curta> | <arquivos-chave> |
```

Checklist:
- [ ] `config.py` com `can_build`, `configure` e dependências (`env.module_add_dependencies`)
- [ ] `SCsub` adicionando os fontes a `env.modules_sources`
- [ ] `register_types.cpp/.h` com `initialize_pmdot_<nome>_module` / `uninitialize_...`
- [ ] Testes em `tests/test_<nome>.h` (ver `Test guide.md`)
- [ ] Incluído em `custom_modules` e em `BUILD_SEGMENTS` do `pmdot.py`

