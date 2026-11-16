(function () {
  function postToGodot(method, params) {
    const payload = JSON.stringify({ method, params });
    if (window.chrome && window.chrome.webview) {
      window.chrome.webview.postMessage(payload);
    }
  }

  function initMonaco() {
    if (typeof monaco === "undefined") {
      return;
    }

    const editor = monaco.editor.create(document.getElementById("monaco-root"), {
      value: "",
      language: "csharp",
      theme: "vs-dark",
      automaticLayout: true,
      minimap: { enabled: true },
      cursorSmoothCaretAnimation: "on",
      cursorBlinking: "smooth",
      smoothScrolling: true,
      fontLigatures: true,
      bracketPairColorization: { enabled: true },
      renderWhitespace: "selection"
    });

    window.PMDoTIDE.editor = editor;
    window.PMDoTIDE.ready = true;

    editor.onDidChangeModelContent(() => {
      if (!window.PMDoTIDE.dirty) {
        window.PMDoTIDE.dirty = true;
        postToGodot("content_changed", {
          path: window.PMDoTIDE.currentPath,
          dirty: true
        });
      }
    });

    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, () => {
      postToGodot("save_script", {
        path: window.PMDoTIDE.currentPath,
        content: editor.getValue()
      });
      window.PMDoTIDE.dirty = false;
    });

    postToGodot("ready", {});
  }

  window.PMDoTIDE = window.PMDoTIDE || {};
  window.PMDoTIDE.openScript = function (path, content, language) {
    window.PMDoTIDE.currentPath = path;
    window.PMDoTIDE.dirty = false;
    const editor = window.PMDoTIDE.editor;
    if (!editor) {
      return;
    }
    const model = monaco.editor.createModel(content, language || "plaintext");
    editor.setModel(model);
  };

  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.addEventListener("message", (event) => {
      const msg = typeof event.data === "string" ? JSON.parse(event.data) : event.data;
      if (msg && msg.method === "open_script" && msg.params) {
        window.PMDoTIDE.openScript(
          msg.params.path,
          msg.params.content,
          msg.params.language
        );
      }
    });
  }

  if (typeof require !== "undefined") {
    require.config({ paths: { vs: "vs" } });
    require(["vs/editor/editor.main"], initMonaco);
  }
})();
