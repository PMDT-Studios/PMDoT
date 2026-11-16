(function () {
  let bpDecorations = [];

  function renderBreakpoints(lines) {
    const editor = window.PMDoTIDE && window.PMDoTIDE.editor;
    if (!editor || typeof monaco === "undefined") return;

    const newDecos = (lines || []).map((line) => ({
      range: new monaco.Range(line, 1, line, 1),
      options: {
        isWholeLine: false,
        glyphMarginClassName: "pmdot-breakpoint-glyph"
      }
    }));
    bpDecorations = editor.deltaDecorations(bpDecorations, newDecos);
  }

  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.addEventListener("message", (event) => {
      const msg = typeof event.data === "string" ? JSON.parse(event.data) : event.data;
      if (msg && msg.method === "dap_breakpoints_updated" && msg.params) {
        renderBreakpoints(msg.params.lines);
      }
    });
  }
})();
