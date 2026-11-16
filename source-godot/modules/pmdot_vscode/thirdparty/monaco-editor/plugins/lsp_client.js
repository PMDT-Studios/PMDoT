(function () {
  window.PMDoTIDE = window.PMDoTIDE || {};

  window.PMDoTIDE.applyConfig = function (cfg) {
    const editor = window.PMDoTIDE.editor;
    if (!editor || !cfg) return;
    if (cfg.theme && typeof monaco !== "undefined") {
      monaco.editor.setTheme(cfg.theme);
    }
    editor.updateOptions({
      fontSize: cfg.fontSize || 14,
      minimap: { enabled: cfg.minimap !== false },
      fontLigatures: cfg.fontLigatures !== false
    });
  };

  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.addEventListener("message", (event) => {
      const msg = typeof event.data === "string" ? JSON.parse(event.data) : event.data;
      if (msg && msg.method === "apply_config" && msg.params) {
        window.PMDoTIDE.applyConfig(msg.params);
      }
    });
  }
})();
