(function () {
  let bpDecorations = [];
  let wired = false;

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

  function wireGutterClick() {
    if (wired) return;
    const editor = window.PMDoTIDE && window.PMDoTIDE.editor;
    if (!editor || typeof monaco === "undefined") return;
    wired = true;
    editor.updateOptions({ glyphMargin: true });
    editor.onMouseDown((e) => {
      if (e.target.type === monaco.editor.MouseTargetType.GUTTER_GLYPH_MARGIN && e.target.position) {
        if (window.PMDoTIDE.postToGodot) {
          window.PMDoTIDE.postToGodot("dap_toggle_breakpoint", {
            path: window.PMDoTIDE.currentPath,
            line: e.target.position.lineNumber
          });
        }
      }
    });
  }

  window.addEventListener("load", () => setTimeout(wireGutterClick, 400));

  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.addEventListener("message", (event) => {
      const msg = typeof event.data === "string" ? JSON.parse(event.data) : event.data;
      if (msg && msg.method === "dap_breakpoints_updated" && msg.params) {
        wireGutterClick();
        renderBreakpoints(msg.params.lines);
      }
    });
  }
})();
