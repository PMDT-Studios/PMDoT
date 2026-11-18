(function () {
  const IDE = (window.PMDoTIDE = window.PMDoTIDE || {});
  const devexState = {
    logs: [],
    scenes: [],
    scenesLoaded: false,
    todos: [],
    todosLoaded: false
  };

  const DOC_LINKS = [
    { label: "C# / .NET API Docs", url: "https://learn.microsoft.com/en-us/dotnet/csharp/" },
    { label: "Godot / PMDoT C# API", url: "https://docs.godotengine.org/en/4.3/tutorials/scripting/c_sharp/index.html" },
    { label: "C++ Reference (cppreference)", url: "https://en.cppreference.com/w/" },
    { label: "Java SE Docs", url: "https://docs.oracle.com/en/java/" }
  ];

  function renderRemoteSidebar(container) {
    container.innerHTML = `
      <div style="padding:8px;border-bottom:1px solid #1e1e1e;">
        <input id="web-search-input" class="sidebar-input" style="margin:0 0 6px 0;width:100%;" placeholder="Search Google / StackOverflow..." />
        <div style="display:flex;gap:6px;">
          <button id="btn-google" class="icon-btn" style="flex:1;justify-content:center;">Google</button>
          <button id="btn-so" class="icon-btn secondary" style="flex:1;justify-content:center;">StackOverflow</button>
        </div>
      </div>
      <div class="section-title">Documentation</div>
      <div id="docs-links"></div>
      <div class="section-title">
        <span>Runtime Logs (${devexState.logs.length})</span>
        <button id="btn-open-logs" class="icon-btn secondary" style="padding:1px 6px;">Panel</button>
      </div>
      <div id="mini-logs" style="max-height:160px;overflow-y:auto;"></div>
    `;

    const queryInput = container.querySelector("#web-search-input");
    container.querySelector("#btn-google").addEventListener("click", () => {
      const q = encodeURIComponent(queryInput.value.trim() || "Godot 4 C# documentation");
      IDE.postToGodot("open_url_req", { url: `https://www.google.com/search?q=${q}` });
    });
    container.querySelector("#btn-so").addEventListener("click", () => {
      const q = encodeURIComponent(queryInput.value.trim() || "godot c#");
      IDE.postToGodot("open_url_req", { url: `https://stackoverflow.com/search?q=${q}` });
    });
    container.querySelector("#btn-open-logs").addEventListener("click", () => {
      IDE.openBottomTab("logs");
    });

    const docsEl = container.querySelector("#docs-links");
    DOC_LINKS.forEach((d) => {
      const el = document.createElement("div");
      el.className = "tree-item";
      el.innerHTML = `<span style="color:#4ec9b0;">↗</span><span>${d.label}</span>`;
      el.addEventListener("click", () => IDE.postToGodot("open_url_req", { url: d.url }));
      docsEl.appendChild(el);
    });

    const miniLogs = container.querySelector("#mini-logs");
    devexState.logs.slice(-10).forEach((l) => {
      const el = document.createElement("div");
      el.className = "tree-item";
      const color = l.level === "error" ? "#f48771" : l.level === "warn" ? "#cca700" : "#cccccc";
      el.innerHTML = `<span style="color:${color};">[${l.level}]</span><span>${l.text}</span>`;
      miniLogs.appendChild(el);
    });
  }

  function renderSceneGraphSidebar(container) {
    container.innerHTML = `
      <div style="padding:8px;display:flex;gap:6px;border-bottom:1px solid #1e1e1e;">
        <button id="btn-scan-scenes" class="icon-btn" style="flex:1;justify-content:center;">Scan Scenes</button>
        <button id="btn-scan-todos" class="icon-btn secondary" style="flex:1;justify-content:center;">Scan TODOs</button>
      </div>
      <div class="section-title">Scene Node Tree</div>
      <div id="scene-tree-list"></div>
      <div class="section-title">Todo Tree (${devexState.todos.length})</div>
      <div id="todo-tree-list"></div>
    `;

    container.querySelector("#btn-scan-scenes").addEventListener("click", () => {
      IDE.postToGodot("scene_graph_req", {});
    });
    container.querySelector("#btn-scan-todos").addEventListener("click", () => {
      IDE.postToGodot("todos_req", {});
    });

    const sceneList = container.querySelector("#scene-tree-list");
    if (!devexState.scenesLoaded) {
      IDE.postToGodot("scene_graph_req", {});
      sceneList.innerHTML = `<div class="tree-item" style="color:#858585;">Scanning .tscn files...</div>`;
    } else if (!devexState.scenes.length) {
      sceneList.innerHTML = `<div class="tree-item" style="color:#858585;">No .tscn scenes found.</div>`;
    } else {
      devexState.scenes.forEach((s) => {
        const hdr = document.createElement("div");
        hdr.className = "tree-item";
        hdr.style.fontWeight = "600";
        hdr.style.color = "#4ec9b0";
        hdr.textContent = s.name;
        sceneList.appendChild(hdr);
        (s.nodes || []).forEach((n) => {
          const item = document.createElement("div");
          item.className = "tree-item";
          item.style.paddingLeft = n.parent === "root" ? "14px" : "24px";
          item.innerHTML = `<span style="color:#569cd6;">◇</span><span>${n.name}</span>`;
          sceneList.appendChild(item);
        });
      });
    }

    const todoList = container.querySelector("#todo-tree-list");
    if (!devexState.todosLoaded) {
      IDE.postToGodot("todos_req", {});
      todoList.innerHTML = `<div class="tree-item" style="color:#858585;">Scanning TODOs...</div>`;
    } else if (!devexState.todos.length) {
      todoList.innerHTML = `<div class="tree-item" style="color:#858585;">No TODO/FIXME/HACK comments found.</div>`;
    } else {
      devexState.todos.forEach((t) => {
        const row = document.createElement("div");
        row.className = "tree-item";
        row.innerHTML = `<span style="color:#dcdcaa;">L${t.line}</span><span>${t.name}: ${t.text}</span>`;
        row.addEventListener("click", () => IDE.postToGodot("open_file_req", { path: t.path }));
        todoList.appendChild(row);
      });
    }
  }

  function renderLogsBottomTab(el) {
    el.innerHTML = `
      <div style="margin-bottom:6px;display:flex;gap:8px;align-items:center;">
        <button id="btn-clear-logs" class="icon-btn secondary">Clear Logs</button>
      </div>
      <div id="logs-stream" style="font-family:monospace;font-size:12px;"></div>
    `;
    el.querySelector("#btn-clear-logs").addEventListener("click", () => {
      IDE.postToGodot("logs_clear_req", {});
    });
    const stream = el.querySelector("#logs-stream");
    if (!devexState.logs.length) {
      stream.innerHTML = `<div style="color:#858585;">No runtime logs captured yet.</div>`;
      return;
    }
    devexState.logs.forEach((l) => {
      const color = l.level === "error" ? "#f48771" : l.level === "warn" ? "#cca700" : "#cccccc";
      const row = document.createElement("div");
      row.style.color = color;
      row.textContent = `[${l.level.toUpperCase()}] ${l.text}`;
      stream.appendChild(row);
    });
  }

  function boot() {
    if (IDE.registerSidebarView) {
      IDE.registerSidebarView("remote", "REMOTE & DOCS", renderRemoteSidebar);
      IDE.registerSidebarView("scene-graph", "SCENE & DEVEX", renderSceneGraphSidebar);
    }
    if (IDE.registerBottomTab) {
      IDE.registerBottomTab("logs", "Logs", renderLogsBottomTab);
    }
    if (IDE.registerCommand) {
      IDE.registerCommand("remote.open", "View: Show Remote Explorer & Docs", () => IDE.switchSidebarView("remote"));
      IDE.registerCommand("scene.graph", "View: Show Scene Node & Todo Graph", () => IDE.switchSidebarView("scene-graph"));
      IDE.registerCommand("logs.open", "View: Open Runtime Logs", () => IDE.openBottomTab("logs"));
    }
  }

  if (IDE.onReady) {
    IDE.onReady(boot);
  } else {
    window.addEventListener("load", () => setTimeout(boot, 350));
  }

  if (IDE.onMessage) {
    IDE.onMessage((msg) => {
      if (!msg || !msg.params) return;
      if (msg.method === "logs_update") {
        devexState.logs = msg.params.logs || [];
        if (IDE.activeView === "remote") IDE.switchSidebarView("remote");
        if (IDE.activeBottomTab === "logs") IDE.openBottomTab("logs");
      } else if (msg.method === "scene_graph_update") {
        devexState.scenesLoaded = true;
        devexState.scenes = msg.params.scenes || [];
        if (IDE.activeView === "scene-graph") IDE.switchSidebarView("scene-graph");
      } else if (msg.method === "todos_update") {
        devexState.todosLoaded = true;
        devexState.todos = msg.params.todos || [];
        if (IDE.activeView === "scene-graph") IDE.switchSidebarView("scene-graph");
      }
    });
  }
})();
