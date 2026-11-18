(function () {
  const IDE = (window.PMDoTIDE = window.PMDoTIDE || {});
  let bpDecorations = [];
  let wired = false;
  const debugState = {
    paused: false,
    breakpoints: new Map(), // path -> [lines]
    callStack: [],
    variables: [],
    tests: [],
    testOutput: "No tests run yet."
  };

  function renderBreakpointsInEditor(lines) {
    const editor = IDE.editor;
    if (!editor || typeof monaco === "undefined") return;
    const newDecos = (lines || []).map((line) => ({
      range: new monaco.Range(line, 1, line, 1),
      options: { isWholeLine: false, glyphMarginClassName: "pmdot-breakpoint-glyph" }
    }));
    bpDecorations = editor.deltaDecorations(bpDecorations, newDecos);
  }

  function wireGutterClick() {
    if (wired) return;
    const editor = IDE.editor;
    if (!editor || typeof monaco === "undefined") return;
    wired = true;
    editor.updateOptions({ glyphMargin: true });
    editor.onMouseDown((e) => {
      if (e.target.type === monaco.editor.MouseTargetType.GUTTER_GLYPH_MARGIN && e.target.position) {
        const line = e.target.position.lineNumber;
        const path = IDE.currentPath;
        if (!path) return;
        const list = debugState.breakpoints.get(path) || [];
        const idx = list.indexOf(line);
        if (idx >= 0) list.splice(idx, 1);
        else list.push(line);
        debugState.breakpoints.set(path, list);
        renderBreakpointsInEditor(list);
        if (IDE.activeView === "debug") IDE.switchSidebarView("debug");
        IDE.postToGodot("dap_toggle_breakpoint", { path, line });
      }
    });
  }

  function renderDebugSidebar(container) {
    container.innerHTML = `
      <div style="padding:8px;display:flex;gap:4px;flex-wrap:wrap;border-bottom:1px solid #1e1e1e;">
        <button class="icon-btn" data-dap="continue">Continue</button>
        <button class="icon-btn secondary" data-dap="step_over">Step Over</button>
        <button class="icon-btn secondary" data-dap="step_into">Step Into</button>
        <button class="icon-btn secondary" data-dap="step_out">Step Out</button>
      </div>
      <div class="section-title">Call Stack</div>
      <div id="dbg-stack">${debugState.callStack.length ? "" : '<div class="tree-item" style="color:#858585;">Not paused</div>'}</div>
      <div class="section-title">Breakpoints</div>
      <div id="dbg-bps"></div>
    `;
    container.querySelectorAll("[data-dap]").forEach((b) => {
      b.addEventListener("click", () => IDE.postToGodot("dap_command", { command: b.dataset.dap }));
    });
    const bpsEl = container.querySelector("#dbg-bps");
    debugState.breakpoints.forEach((lines, path) => {
      lines.forEach((ln) => {
        const row = document.createElement("div");
        row.className = "tree-item";
        row.innerHTML = `<span style="color:#e51400;">●</span><span>${path.split("/").pop()}:${ln}</span>`;
        row.addEventListener("click", () => IDE.postToGodot("open_file_req", { path }));
        bpsEl.appendChild(row);
      });
    });
  }

  function renderTestingSidebar(container) {
    container.innerHTML = `
      <div style="padding:8px;display:flex;gap:6px;border-bottom:1px solid #1e1e1e;">
        <button id="btn-run-all-tests" class="icon-btn">Run All</button>
        <button id="btn-run-unit" class="icon-btn secondary">Unit</button>
        <button id="btn-run-integ" class="icon-btn secondary">Integration</button>
      </div>
      <div class="section-title">Unit & Integration Suites</div>
      <div id="test-tree"></div>
    `;
    container.querySelector("#btn-run-all-tests").addEventListener("click", () => {
      IDE.postToGodot("test_run_req", { filter: "all" });
      IDE.openBottomTab("test-output");
    });
    container.querySelector("#btn-run-unit").addEventListener("click", () => {
      IDE.postToGodot("test_run_req", { filter: "unit" });
      IDE.openBottomTab("test-output");
    });
    container.querySelector("#btn-run-integ").addEventListener("click", () => {
      IDE.postToGodot("test_run_req", { filter: "integration" });
      IDE.openBottomTab("test-output");
    });

    const tree = container.querySelector("#test-tree");
    if (!debugState.tests.length) {
      IDE.postToGodot("test_discover_req", {});
      tree.innerHTML = `<div class="tree-item" style="color:#858585;">Scanning project tests...</div>`;
      return;
    }
    debugState.tests.forEach((t) => {
      const color = t.status === "passed" ? "#2ea043" : t.status === "failed" ? "#f85149" : "#858585";
      const row = document.createElement("div");
      row.className = "tree-item";
      row.innerHTML = `<span style="color:${color};font-weight:700;">[${t.type.toUpperCase()}]</span><span>${t.name}</span>`;
      row.addEventListener("click", () => IDE.postToGodot("test_run_req", { filter: t.id }));
      tree.appendChild(row);
    });
  }

  function renderTestOutputTab(el) {
    el.innerHTML = `<pre style="margin:0;white-space:pre-wrap;color:#cccccc;font-family:monospace;">${debugState.testOutput}</pre>`;
  }

  function boot() {
    wireGutterClick();
    if (IDE.registerSidebarView) {
      IDE.registerSidebarView("debug", "RUN AND DEBUG", renderDebugSidebar);
      IDE.registerSidebarView("testing", "TESTING", renderTestingSidebar);
    }
    if (IDE.registerBottomTab) {
      IDE.registerBottomTab("test-output", "Test Results", renderTestOutputTab);
    }
    if (IDE.registerCommand) {
      IDE.registerCommand("debug.open", "View: Show Run and Debug", () => IDE.switchSidebarView("debug"));
      IDE.registerCommand("test.open", "View: Show Testing", () => IDE.switchSidebarView("testing"));
      IDE.registerCommand("test.runAll", "Test: Run All Unit & Integration Tests", () => {
        IDE.postToGodot("test_run_req", { filter: "all" });
        IDE.openBottomTab("test-output");
      });
    }
  }

  window.addEventListener("load", () => setTimeout(boot, 350));

  if (IDE.onMessage) {
    IDE.onMessage((msg) => {
      if (!msg || !msg.params) return;
      if (msg.method === "dap_breakpoints_updated") {
        debugState.breakpoints.set(msg.params.path, msg.params.lines || []);
        if (msg.params.path === IDE.currentPath) {
          renderBreakpointsInEditor(msg.params.lines || []);
        }
      } else if (msg.method === "tests_discovered") {
        debugState.tests = msg.params.tests || [];
        if (IDE.activeView === "testing") IDE.switchSidebarView("testing");
      } else if (msg.method === "tests_finished") {
        debugState.testOutput = msg.params.output || "";
        IDE.openBottomTab("test-output");
      }
    });
  }
})();
