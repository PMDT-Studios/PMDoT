# Test Guide — Engine & Module Verification

PMDoT uses the Godot-integrated **doctest** test framework located under `source-godot/tests/`.

---

## 1. Compiling with Test Suite Support

```powershell
cd source-godot
scons platform=windows target=editor tests=yes disable_exceptions=no custom_modules=modules/pmdot_aseprite module_gdscript_enabled=no module_jsonrpc_enabled=no -j8
```

> By default, standard builds via `pmdot build` have `tests=false` to maximize build velocity.

---

## 2. Running Engine Tests

```powershell
bin\godot.windows.editor.x86_64.console.exe --test                       # Run all test cases
bin\godot.windows.editor.x86_64.console.exe --test --test-case="*Pixel*"  # Filter by case name
bin\godot.windows.editor.x86_64.console.exe --test --test-suite="[ProjectSettings]*"
```

---

## 3. Test File Conventions and Layout

| Scope | Directory | Discovery Mechanism |
|---|---|---|
| Core Engine (core/scene/servers) | `source-godot/tests/<subsystem>/test_*.h` | Statically linked in `tests/test_main.cpp` |
| Modules | `source-godot/modules/<module>/tests/*.h` | Dynamically scanned: `modules/SCsub` generates `modules_tests.gen.h` |

### Module Test Template:
```cpp
// modules/pmdot_<name>/tests/test_<feature>.h
#pragma once
#include "tests/test_macros.h"
#include "../<name>.h"

namespace TestPMDoT<Name> {
TEST_CASE("[PMDoT][<Name>] Describe feature behavior") {
	CHECK(...);
}
} // namespace TestPMDoT<Name>
```

> **Convention:** All test suites must prefix their test cases with `[PMDoT]` so they can be isolated using `--test-case="[PMDoT]*"`.

---

## 4. Impact of GDScript Removal (CHG-002) on Tests

| Target | Action Taken |
|---|---|
| `modules/gdscript/tests/` (`gdscript_test_runner.*`, `test_gdscript.*`, `test_completion.h`, `test_lsp.h`, and `scripts/` with ~1,260 scripts) | **Deleted** along with the module. |
| `modules/jsonrpc/tests/test_jsonrpc.*` | **Deleted** along with JSON-RPC. |
| `tests/core/config/test_project_settings.h` | **Decoupled**: dynamically initializes `application/config/name` within test cases, eliminating reliance on the removed `GDScript Integration Test Suite` project file. |
| `tests/scene/test_primitives.h:749` (`STRUCTURED_TEXT_GDSCRIPT`) | **Retained**: preserved as public enum constant. |

---

## 5. Structural Change Verification Checklist
- [ ] Clean build passes: `pmdot build --force`
- [ ] Test build passes: SCons with `tests=yes`
- [ ] Engine tests pass (`--test`)
- [ ] Editor executable starts with zero console `ERROR` / `WARNING` logs
- [ ] Embedded Aseprite Workspace (`pmdot_aseprite`) mounts cleanly in editor
- [ ] Changes recorded in `Changes.md`
