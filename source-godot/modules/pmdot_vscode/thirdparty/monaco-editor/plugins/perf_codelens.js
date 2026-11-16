(function () {
  let activeMetrics = [];
  let registered = false;

  function ensureProvider() {
    if (registered || typeof monaco === "undefined") return;
    registered = true;

    const provider = {
      provideCodeLenses: function (model) {
        const lenses = [];
        const lineCount = model.getLineCount();
        for (let i = 1; i <= lineCount; i++) {
          const text = model.getLineContent(i);
          for (const m of activeMetrics) {
            if (text.includes(m.method + "(")) {
              lenses.push({
                range: { startLineNumber: i, startColumn: 1, endLineNumber: i, endColumn: 1 },
                command: {
                  id: "",
                  title: `${m.avgMs.toFixed(2)} ms avg | ${m.callsPerSec} calls/s`
                }
              });
            }
          }
        }
        return { lenses, dispose: () => {} };
      }
    };

    monaco.languages.registerCodeLensProvider("csharp", provider);
    monaco.languages.registerCodeLensProvider("cpp", provider);
  }

  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.addEventListener("message", (event) => {
      const msg = typeof event.data === "string" ? JSON.parse(event.data) : event.data;
      if (msg && msg.method === "perf_codelens_update" && msg.params) {
        activeMetrics = msg.params.metrics || [];
        ensureProvider();
      }
    });
  }
})();
