# Changes — Registro de Alterações da PMDoT

> Toda alteração estrutural no repositório (remoção, criação de módulo, mudança de build) **deve** ser registrada aqui, da mais recente para a mais antiga.
> Formato de cada entrada: data · ID · status (`PLANEJADO` | `EM ANDAMENTO` | `CONCLUÍDO` | `REVERTIDO`) · arquivos afetados · motivo · validação.

---

## 2026-11-06 · CHG-002 · Remoção completa do GDScript · `CONCLUÍDO`

**Motivo:** O GDScript deixou de ser usado nos jogos da PMDoT. O repositório passa a focar prioritariamente em **C#** e **C++ (GDExtension/módulos)**, com planejamento futuro para **Java**. A remoção reduziu 1.343 arquivos versionados de gdscript e 9 de jsonrpc (~71 MB em disco), acelerando significativamente os tempos de compilação da engine.

**Escopo executado (por fase):**

| Fase | Status | Alteração |
|---|---|---|
| F0 | ✅ | Baseline: branch de trabalho `GDSCRIPT-KILLING` criada e conferida. |
| F1 | ✅ | `pmdot.py` e `build_pmdot.py`: adicionadas as flags `module_gdscript_enabled=no` e `module_jsonrpc_enabled=no`. |
| F2 | ✅ | Apagado `source-godot/modules/gdscript/` (1.343 arquivos versionados + objetos compilados). |
| F2 | ✅ | Apagado `source-godot/modules/jsonrpc/` (dependência direta do LSP do GDScript). |
| F2 | ✅ | Apagados `misc/dist/document_icons/gdscript*.svg` e `misc/dist/macos_tools.app/Contents/Resources/GDScript.icns`. |
| F2 | ✅ | `scu_builders.py` (linhas 327-329 removidas); `.github/CODEOWNERS` (linhas 119-120 removidas); MIME `.gd` removido de Linux XML e macOS plist. |
| F3 | ✅ | `main/main.cpp`: removidos blocos `#ifdef MODULE_GDSCRIPT_ENABLED` (LSP, `--lsp-port`, `--gdscript-docs`, `get_files_with_extension`). |
| F3 | ✅ | `editor/editor_help.cpp/.h` e `editor/editor_node.cpp`: limpo o enum e ramos de `LANGUAGE_GDSCRIPT` e headers do gdscript, mantendo apenas C# (`MODULE_MONO_ENABLED`). |
| F3 | ✅ | `editor/code_editor.cpp:1037-1038`: fallback limpo para as cores de string e fonte do editor (removidas chamadas inexistentes a `text_editor/theme/highlighting/gdscript/*`). |
| F3 | ✅ | `editor/plugins/script_text_editor.cpp:580`: chamada segura com fallback via `ProjectSettings::get_singleton()->get_setting()`. |
| F4 | ✅ | `tests/core/config/test_project_settings.h`: eliminada dependência estática de `"GDScript Integration Test Suite"` atribuindo dinamicamente durante o teste. |
| F4 | ✅ | `doc/classes/ProjectSettings.xml`: removidos os 50+ nós `<member name="debug/gdscript/*">`. |
| F5 | ✅ | Verificação estrutural e scripts SCons validados (SConscript parsing limpo sem dependência dos módulos removidos). |

**Não removido (preservado conforme regras de compatibilidade):**
- `TextServer::STRUCTURED_TEXT_GDSCRIPT` (constante de enum da API pública exposta a bindings GDExtension).
- Blocos `[gdscript]` de exemplo na documentação interna dos nós Godot.
- `project_converter_3_to_4` e bindings genéricos de `ScriptServer`/`ScriptLanguage` (necessários para C# e bindings GDExtension).

**Decisões e Linguagens Alvo:**
- Gameplay momentaneamente suportado: **C#** e **C++**.
- Futuro roadmap: Suporte a **Java**.
- `modules/jsonrpc`: Removido juntamente com o GDScript.

---

## 2026-11-06 · CHG-001 · Criação do *PMDoT Architechture Guide* · `CONCLUÍDO`

**Arquivos criados:**
- `PMDoT Architechture Guide/Changes.md`
- `PMDoT Architechture Guide/Folders Path creation guide.md`
- `PMDoT Architechture Guide/Test guide.md`
- `PMDoT Architechture Guide/Architecture.md`
- `PMDoT Architechture Guide/Modules.md`

**Motivo:** centralizar a documentação de arquitetura e o histórico de mudanças estruturais.
**Observação:** `architecture-guidelines/` (guia do Aseprite e referência da CLI) continua existindo. Os documentos dela são referenciados a partir deste guia.

