# Test Guide — Testes na PMDoT Engine

A engine usa o framework **doctest** integrado ao Godot (`source-godot/tests/`).

## 1. Compilar com testes

```powershell
cd source-godot
scons platform=windows target=editor tests=yes disable_exceptions=no custom_modules=modules/pmdot_aseprite -j8
```
> Hoje `.scons_env.json` registra `tests: false`, então o build padrão do `pmdot build` **não** inclui testes.

## 2. Executar

```powershell
bin\godot.windows.editor.x86_64.console.exe --test                       # tudo
bin\godot.windows.editor.x86_64.console.exe --test --test-case="*Pixel*"  # filtro por nome
bin\godot.windows.editor.x86_64.console.exe --test --test-suite="[ProjectSettings]*"
```

## 3. Onde ficam os testes

| Tipo | Caminho | Descoberta |
|---|---|---|
| Engine (core/scene/servers) | `source-godot/tests/<área>/test_*.h` | Incluídos em `tests/test_main.cpp` |
| Módulo | `source-godot/modules/<modulo>/tests/*.h` | Automática: `modules/SCsub` gera `modules_tests.gen.h` |

Template de teste de módulo:
```cpp
// modules/pmdot_<nome>/tests/test_<assunto>.h
#pragma once
#include "tests/test_macros.h"
#include "../<nome>.h"

namespace TestPMDoT<Nome> {
TEST_CASE("[PMDoT][<Nome>] descreve o comportamento") {
	CHECK(...);
}
} // namespace TestPMDoT<Nome>
```
Convenção: o nome da suíte começa com `[PMDoT]` para facilitar o filtro `--test-case="[PMDoT]*"`.

## 4. Impacto do CHG-002 (remoção do GDScript) nos testes

| Item | Ação |
|---|---|
| `modules/gdscript/tests/` (`gdscript_test_runner.*`, `test_gdscript.*`, `test_completion.h`, `test_lsp.h`, `scripts/` com cerca de 1.260 arquivos `.gd`/`.out`) | **Apagar** junto com o módulo |
| `modules/jsonrpc/tests/test_jsonrpc.*` | Apagar se o `jsonrpc` for removido |
| `tests/core/config/test_project_settings.h` (2 casos que esperam `"GDScript Integration Test Suite"`) | **Corrigir**: definir `application/config/name` dentro do teste e restaurar no final. Hoje esse valor vem de `modules/gdscript/tests/scripts/project.godot` |
| `tests/scene/test_primitives.h:749` (`STRUCTURED_TEXT_GDSCRIPT`) | Manter: o enum é API pública e continua existindo |

## 5. Checklist de validação de uma mudança estrutural
- [ ] `pmdot build --force` compila
- [ ] Build com `tests=yes` compila
- [ ] `--test` 100% verde
- [ ] Editor abre sem `ERROR`/`WARNING` no console (`*.console.exe`)
- [ ] Script Editor, Editor Settings e Editor Help abrem sem erros
- [ ] Workspace Aseprite (`pmdot_aseprite`) funciona
- [ ] Resultado registrado em `Changes.md`

## 6. Módulos PMDoT sem cobertura (pendência)
- `pmdot_aseprite`: ainda **sem** `tests/`. Sugestão: testes de `PMDoTPixelPipeline` (expansão de paleta, swizzle BGRA→RGBA, premultiplicação de alpha) comparando a saída SIMD com uma implementação escalar de referência.

