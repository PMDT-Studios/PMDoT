(function () {
  let lensDecorations = [];
  let debounceTimer = null;

  function applyDiagnostics(diagnostics) {
    const editor = window.PMDoTIDE && window.PMDoTIDE.editor;
    if (!editor || typeof monaco === "undefined") return;
    const model = editor.getModel();
    if (!model) return;

    const markers = (diagnostics || []).map((d) => ({
      startLineNumber: d.line,
      startColumn: d.column || 1,
      endLineNumber: d.line,
      endColumn: model.getLineMaxColumn(d.line),
      message: d.message,
      severity: d.severity || monaco.MarkerSeverity.Error
    }));
    monaco.editor.setModelMarkers(model, "pmdot-diagnostics", markers);

    const newDecos = (diagnostics || []).map((d) => {
      const isErr = d.severity === 8;
      return {
        range: new monaco.Range(d.line, 1, d.line, 1),
        options: {
          isWholeLine: true,
          className: isErr ? "pmdot-lens-error-line" : "pmdot-lens-warn-line",
          after: {
            content: "  " + (isErr ? "✖ " : "⚠ ") + d.message,
            inlineClassName: isErr ? "pmdot-lens-error-text" : "pmdot-lens-warn-text"
          }
        }
      };
    });
    lensDecorations = editor.deltaDecorations(lensDecorations, newDecos);
  }

  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.addEventListener("message", (event) => {
      const msg = typeof event.data === "string" ? JSON.parse(event.data) : event.data;
      if (msg && msg.method === "diagnostics_update" && msg.params) {
        clearTimeout(debounceTimer);
        debounceTimer = setTimeout(() => applyDiagnostics(msg.params.diagnostics), 150);
      }
    });
  }
})();
