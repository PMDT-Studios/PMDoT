(function () {
  let gitDecorations = [];
  let diffEditor = null;

  function renderGutter(hunks) {
    const editor = window.PMDoTIDE && window.PMDoTIDE.editor;
    if (!editor || typeof monaco === "undefined") return;

    const decos = (hunks || []).map((h) => {
      let cls = "pmdot-git-modified";
      if (h.type === 0) cls = "pmdot-git-added";
      else if (h.type === 2) cls = "pmdot-git-deleted";
      return {
        range: new monaco.Range(h.startLine, 1, h.endLine, 1),
        options: {
          isWholeLine: true,
          linesDecorationsClassName: cls
        }
      };
    });
    gitDecorations = editor.deltaDecorations(gitDecorations, decos);
  }

  function openSideBySideDiff(params) {
    if (typeof monaco === "undefined") return;
    const root = document.getElementById("monaco-root");
    if (!diffEditor) {
      diffEditor = monaco.editor.createDiffEditor(root, {
        theme: "vs-dark",
        automaticLayout: true,
        renderSideBySide: true
      });
    }
    const origModel = monaco.editor.createModel(params.original || "", params.language || "plaintext");
    const modModel = monaco.editor.createModel(params.modified || "", params.language || "plaintext");
    diffEditor.setModel({ original: origModel, modified: modModel });
  }

  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.addEventListener("message", (event) => {
      const msg = typeof event.data === "string" ? JSON.parse(event.data) : event.data;
      if (!msg || !msg.params) return;
      if (msg.method === "git_gutter_update") {
        renderGutter(msg.params.hunks);
      } else if (msg.method === "git_open_diff") {
        openSideBySideDiff(msg.params);
      }
    });
  }
})();
