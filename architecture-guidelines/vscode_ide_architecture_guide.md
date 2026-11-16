# Architecture Guidelines: Embedded VS Code (Monaco) IDE (`pmdot_vscode`)

Technical reference for the `pmdot_vscode` module in **PMDoT Engine (Godot 4.3-stable)**.

---

## 1. High-Level Architecture

```mermaid
flowchart TD
    Dock["VSCodeEditorDock (Control)"] --> WebView["WebViewBackend (Win32 WebView2 / POSIX)"]
    Dock --> Bridge["VSCodeBridge (JSON IPC)"]
    Dock --> LSP["VSCodeLSPManager (C# / C++ / Java)"]
    Dock --> DAP["VSCodeDAPClient (Breakpoints / Stack)"]
    Dock --> Diag["VSCodeDiagnosticHub (Error Lens)"]
    Dock --> Perf["VSCodeCodeLensProfiler (1Hz CPU)"]
    Dock --> Git["VSCodeGitTracker (Gutter / Diff)"]

    WebView <--> Monaco["thirdparty/monaco-editor (index.html + bridge.js + plugins/)"]
```

---

## 2. Performance Design

1. **Pre-warmed WebView**: `VSCodeEditorDock::prewarm()` initializes the native WebView container on `NOTIFICATION_READY`, ensuring sub-frame (`<16ms`) tab switching.
2. **Debounced Diagnostics (Error Lens)**: `error_lens.js` debounces updates by `150ms` and uses `editor.deltaDecorations()` to preserve 60+ FPS typing.
3. **1 Hz Telemetry Polling**: `VSCodeCodeLensProfiler` throttles `Performance::get_singleton()` queries to `1000ms` and pauses when the IDE tab is hidden.
4. **Zero-Binary Windows Overhead**: `WebViewWindows` dynamically binds `WebView2Loader.dll` and uses `#1e1e1e` native background erase to prevent white flicker.

---

## 3. IPC Message Protocol (`VSCodeBridge`)

| Method | Direction | Payload |
|---|---|---|
| `open_script` | C++ -> JS | `{ path, content, language }` |
| `save_script` | JS -> C++ | `{ path, content }` |
| `content_changed` | JS -> C++ | `{ path, dirty }` |
| `apply_config` | C++ -> JS | `{ theme, fontSize, minimap, fontLigatures }` |
| `dap_breakpoints_updated` | C++ -> JS | `{ path, lines }` |
| `diagnostics_update` | C++ -> JS | `{ path, diagnostics }` |
| `perf_codelens_update` | C++ -> JS | `{ path, metrics }` |
| `git_gutter_update` | C++ -> JS | `{ path, hunks }` |
| `git_open_diff` | C++ -> JS | `{ path, original, modified, language }` |

