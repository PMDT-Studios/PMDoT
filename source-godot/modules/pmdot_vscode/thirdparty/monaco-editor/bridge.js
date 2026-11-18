(function () {
  const ICONS = (window.PMDoTIDE && window.PMDoTIDE.icons) || {};
  const state = Object.assign(window.PMDoTIDE || {}, {
    editor: null,
    ready: false,
    currentPath: "",
    dirty: false,
    files: [],
    openTabs: new Map(),
    activeView: "explorer",
    sidebarViews: new Map(),
    bottomTabs: new Map(),
    activeBottomTab: "",
    commands: new Map(),
    messageHandlers: []
  });
  window.PMDoTIDE = state;

  function postToGodot(method, params) {
    const payload = JSON.stringify({ method, params: params || {} });
    if (window.chrome && window.chrome.webview) {
      window.chrome.webview.postMessage(payload);
    }
  }
  state.postToGodot = postToGodot;

  state.registerSidebarView = function (id, title, renderer) {
    state.sidebarViews.set(id, { title, renderer });
    if (state.activeView === id) {
      renderActiveSidebar();
    }
  };

  state.registerBottomTab = function (id, title, renderer) {
    state.bottomTabs.set(id, { title, renderer });
    renderBottomTabs();
  };

  state.openBottomTab = function (id) {
    const panel = document.getElementById("bottom-panel");
    if (panel) panel.classList.add("open");
    state.activeBottomTab = id;
    renderBottomTabs();
  };

  state.registerCommand = function (id, label, handler) {
    state.commands.set(id, { id, label, handler });
  };

  state.onMessage = function (fn) {
    state.messageHandlers.push(fn);
  };

  function hydrateSvgIcons() {
    const iconMap = {
      explorer: ICONS.explorer,
      search: ICONS.search,
      scm: ICONS.scm,
      debug: ICONS.debug,
      testing: ICONS.testing,
      extensions: ICONS.extensions,
      remote: ICONS.remote,
      "scene-graph": ICONS.sceneGraph
    };
    document.querySelectorAll(".act-btn").forEach((btn) => {
      const badge = btn.querySelector(".act-badge");
      const svg = iconMap[btn.dataset.view] || "";
      btn.innerHTML = svg;
      if (badge) btn.appendChild(badge);
    });
    const newBtn = document.getElementById("btn-new-file");
    if (newBtn) {
      newBtn.innerHTML = (ICONS.newFile || "") + "<span>New</span>";
    }
  }

  function renderTabs() {
    const bar = document.getElementById("tab-bar");
    if (!bar) return;
    bar.innerHTML = "";
    state.openTabs.forEach((tab, path) => {
      const el = document.createElement("div");
      el.className = "editor-tab" + (path === state.currentPath ? " active" : "");
      const fileName = path.split("/").pop();
      const icon = state.getFileIcon ? state.getFileIcon(tab.language) : "";
      el.innerHTML = `${icon}<span>${fileName}${tab.dirty ? " *" : ""}</span><span class="tab-close" data-close="${path}">${ICONS.close || "x"}</span>`;
      el.addEventListener("click", (e) => {
        const closeTarget = e.target.closest("[data-close]");
        if (closeTarget) {
          closeTab(closeTarget.dataset.close);
        } else {
          switchTab(path);
        }
      });
      bar.appendChild(el);
    });
  }

  function renderBreadcrumbs() {
    const bc = document.getElementById("breadcrumbs");
    if (!bc) return;
    if (!state.currentPath) {
      bc.innerHTML = "<span>No file open</span>";
      return;
    }
    const parts = state.currentPath.replace("res://", "").split("/").filter(Boolean);
    bc.innerHTML = parts
      .map((p) => `<span>${p}</span>`)
      .join(`<span class="crumb-sep">${ICONS.chevronRight || "/"}</span>`);
  }

  let outlineTimer = null;
  function scheduleOutlineUpdate() {
    clearTimeout(outlineTimer);
    outlineTimer = setTimeout(updateOutline, 200);
  }

  function updateOutline() {
    const list = document.getElementById("outline-list");
    if (!list || !state.editor) return;
    const model = state.editor.getModel();
    if (!model) {
      list.innerHTML = "";
      return;
    }
    const lines = model.getLinesContent();
    const symbols = [];
    const classRe = /\b(class|struct|interface|enum)\s+([A-Za-z_][A-Za-z0-9_]*)/;
    const methodRe = /^\s*(?:public|private|protected|internal|static|virtual|override|async|\s)+\s*[A-Za-z0-9_<>\[\]]+\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(/;

    for (let i = 0; i < lines.length; i++) {
      const line = lines[i];
      let m = classRe.exec(line);
      if (m) {
        symbols.push({ kind: "class", name: m[2], line: i + 1 });
        continue;
      }
      m = methodRe.exec(line);
      if (m && m[1] !== "if" && m[1] !== "for" && m[1] !== "while" && m[1] !== "switch") {
        symbols.push({ kind: "method", name: m[1] + "()", line: i + 1 });
      }
    }

    list.innerHTML = "";
    symbols.forEach((sym) => {
      const el = document.createElement("div");
      el.className = "tree-item";
      const icon = sym.kind === "class" ? ICONS.symbolClass : ICONS.symbolMethod;
      el.innerHTML = `${icon || ""}<span>${sym.name}</span>`;
      el.addEventListener("click", () => {
        state.editor.revealLineInCenter(sym.line);
        state.editor.setPosition({ lineNumber: sym.line, column: 1 });
        state.editor.focus();
      });
      list.appendChild(el);
    });
  }

  function switchTab(path) {
    const tab = state.openTabs.get(path);
    if (!tab || !state.editor) return;
    state.currentPath = path;
    state.dirty = tab.dirty;
    state.editor.setModel(tab.model);
    updateStatusBar();
    renderTabs();
    renderBreadcrumbs();
    scheduleOutlineUpdate();
    if (state.activeView === "explorer") renderExplorer(document.getElementById("sidebar-content"));
  }

  function closeTab(path) {
    const tab = state.openTabs.get(path);
    if (!tab) return;
    tab.model.dispose();
    state.openTabs.delete(path);
    if (state.currentPath === path) {
      const next = state.openTabs.keys().next().value || "";
      state.currentPath = next;
      if (next) {
        switchTab(next);
      } else if (state.editor) {
        state.editor.setModel(monaco.editor.createModel("", "plaintext"));
        renderBreadcrumbs();
        updateOutline();
      }
    }
    renderTabs();
  }

  function renderExplorer(container) {
    if (!container) return;
    container.innerHTML = "";
    state.files.forEach((f) => {
      const item = document.createElement("div");
      item.className = "tree-item" + (f.path === state.currentPath ? " active" : "");
      item.title = f.path;
      const icon = state.getFileIcon ? state.getFileIcon(f.language) : "";
      item.innerHTML = `${icon}<span>${f.name}</span>`;
      item.addEventListener("click", () => {
        if (state.openTabs.has(f.path)) {
          switchTab(f.path);
        } else {
          postToGodot("open_file_req", { path: f.path });
        }
      });
      container.appendChild(item);
    });
  }

  function renderSearchView(container, matches) {
    if (!container) return;
    if (!document.getElementById("search-box")) {
      container.innerHTML = `<input id="search-box" class="sidebar-input" type="text" placeholder="Search in project files..." /><div id="search-results"></div>`;
      const input = document.getElementById("search-box");
      input.addEventListener("input", () => {
        postToGodot("search_files_req", { query: input.value });
      });
      input.focus();
    }
    const resDiv = document.getElementById("search-results");
    if (!resDiv || !matches) return;
    resDiv.innerHTML = "";
    matches.forEach((m) => {
      const el = document.createElement("div");
      el.className = "tree-item";
      el.innerHTML = `<span style="color:#4ec9b0;font-weight:700;">L${m.line}</span><span>${m.name}: ${m.preview}</span>`;
      el.addEventListener("click", () => postToGodot("open_file_req", { path: m.path }));
      resDiv.appendChild(el);
    });
  }

  function renderActiveSidebar() {
    const container = document.getElementById("sidebar-content");
    const titleEl = document.getElementById("sidebar-title");
    const newBtn = document.getElementById("btn-new-file");
    if (!container || !titleEl) return;

    const custom = state.sidebarViews.get(state.activeView);
    if (custom) {
      titleEl.textContent = custom.title;
      if (newBtn) newBtn.style.display = "none";
      custom.renderer(container);
      return;
    }

    if (newBtn) newBtn.style.display = state.activeView === "explorer" ? "inline-flex" : "none";
    if (state.activeView === "explorer") {
      titleEl.textContent = "EXPLORER";
      renderExplorer(container);
    } else if (state.activeView === "search") {
      titleEl.textContent = "SEARCH";
      renderSearchView(container, []);
    } else {
      titleEl.textContent = state.activeView.toUpperCase();
      container.innerHTML = `<div style="padding:12px;color:#858585;">View '${state.activeView}' module not loaded.</div>`;
    }
  }

  function renderBottomTabs() {
    const tabList = document.getElementById("bottom-tab-list");
    const content = document.getElementById("bottom-content");
    if (!tabList || !content) return;
    tabList.innerHTML = "";
    if (!state.activeBottomTab && state.bottomTabs.size > 0) {
      state.activeBottomTab = state.bottomTabs.keys().next().value;
    }
    state.bottomTabs.forEach((tab, id) => {
      const btn = document.createElement("button");
      btn.className = "bottom-tab" + (id === state.activeBottomTab ? " active" : "");
      btn.textContent = tab.title;
      btn.addEventListener("click", () => {
        state.activeBottomTab = id;
        renderBottomTabs();
      });
      tabList.appendChild(btn);
    });
    const active = state.bottomTabs.get(state.activeBottomTab);
    if (active) {
      active.renderer(content);
    } else {
      content.innerHTML = `<div style="color:#858585;">No output.</div>`;
    }
  }

  function switchSidebarView(viewId) {
    state.activeView = viewId;
    document.querySelectorAll(".act-btn").forEach((b) => {
      b.classList.toggle("active", b.dataset.view === viewId);
    });
    renderActiveSidebar();
  }
  state.switchSidebarView = switchSidebarView;

  function updateStatusBar() {
    const right = document.getElementById("status-right");
    if (!right || !state.editor) return;
    const pos = state.editor.getPosition() || { lineNumber: 1, column: 1 };
    const model = state.editor.getModel();
    const lang = model ? model.getLanguageId() : "plaintext";
    right.textContent = `Ln ${pos.lineNumber}, Col ${pos.column} | UTF-8 | ${lang}`;
  }

  function openQuickOverlay(initialPrefix) {
    const overlay = document.getElementById("quick-open-overlay");
    const input = document.getElementById("quick-open-input");
    const list = document.getElementById("quick-open-list");
    overlay.style.display = "block";
    input.value = initialPrefix || "";
    input.focus();

    function refreshList() {
      const raw = input.value;
      list.innerHTML = "";
      if (raw.startsWith(">")) {
        const q = raw.slice(1).trim().toLowerCase();
        Array.from(state.commands.values())
          .filter((c) => c.label.toLowerCase().includes(q))
          .slice(0, 20)
          .forEach((cmd) => {
            const el = document.createElement("div");
            el.className = "tree-item";
            el.textContent = cmd.label;
            el.addEventListener("click", () => {
              overlay.style.display = "none";
              cmd.handler();
            });
            list.appendChild(el);
          });
      } else {
        const q = raw.toLowerCase();
        state.files
          .filter((f) => f.path.toLowerCase().includes(q))
          .slice(0, 20)
          .forEach((f) => {
            const el = document.createElement("div");
            el.className = "tree-item";
            const icon = state.getFileIcon ? state.getFileIcon(f.language) : "";
            el.innerHTML = `${icon}<span>${f.path}</span>`;
            el.addEventListener("click", () => {
              overlay.style.display = "none";
              postToGodot("open_file_req", { path: f.path });
            });
            list.appendChild(el);
          });
      }
    }
    input.oninput = refreshList;
    input.onkeydown = (e) => {
      if (e.key === "Escape") overlay.style.display = "none";
    };
    refreshList();
  }

  function initMonaco() {
    if (typeof monaco === "undefined") return;

    hydrateSvgIcons();

    const editor = monaco.editor.create(document.getElementById("monaco-root"), {
      value: "// Select or create a file in the Explorer (Ctrl+P: Quick Open, Ctrl+Shift+P: Commands)\n",
      language: "csharp",
      theme: "vs-dark",
      automaticLayout: true,
      minimap: { enabled: true },
      cursorSmoothCaretAnimation: "on",
      cursorBlinking: "smooth",
      smoothScrolling: true,
      fontLigatures: true,
      bracketPairColorization: { enabled: true },
      "semanticHighlighting.enabled": true,
      renderWhitespace: "selection"
    });

    state.editor = editor;
    state.ready = true;

    editor.onDidChangeCursorPosition(updateStatusBar);
    editor.onDidChangeModelContent(() => {
      scheduleOutlineUpdate();
      if (!state.currentPath) return;
      const tab = state.openTabs.get(state.currentPath);
      if (tab && !tab.dirty) {
        tab.dirty = true;
        state.dirty = true;
        renderTabs();
        postToGodot("content_changed", { path: state.currentPath, dirty: true });
      }
    });

    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, () => {
      if (!state.currentPath) return;
      const tab = state.openTabs.get(state.currentPath);
      if (tab) tab.dirty = false;
      state.dirty = false;
      renderTabs();
      postToGodot("save_script", { path: state.currentPath, content: editor.getValue() });
    });

    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyP, () => openQuickOverlay(""));
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyMod.Shift | monaco.KeyCode.KeyP, () => openQuickOverlay(">"));

    state.registerCommand("view.explorer", "View: Show Explorer", () => switchSidebarView("explorer"));
    state.registerCommand("view.search", "View: Search in Files", () => switchSidebarView("search"));
    state.registerCommand("panel.toggle", "View: Toggle Bottom Panel", () => {
      document.getElementById("bottom-panel").classList.toggle("open");
    });
    state.registerCommand("file.new", "File: New Code File", () => {
      const name = prompt("New file path (e.g. scripts/Player.cs, src/Main.cpp, README.md):", "scripts/NewScript.cs");
      if (name && name.trim()) postToGodot("create_file_req", { path: name.trim() });
    });

    document.querySelectorAll(".act-btn").forEach((btn) => {
      btn.addEventListener("click", () => switchSidebarView(btn.dataset.view));
    });

    document.getElementById("btn-new-file").addEventListener("click", () => {
      state.commands.get("file.new").handler();
    });
    document.getElementById("btn-toggle-panel").addEventListener("click", () => {
      state.commands.get("panel.toggle").handler();
    });
    document.getElementById("btn-close-bottom").addEventListener("click", () => {
      document.getElementById("bottom-panel").classList.remove("open");
    });
    document.getElementById("btn-cmd-palette").addEventListener("click", () => openQuickOverlay(">"));

    postToGodot("ready", {});
    renderBreadcrumbs();
    updateStatusBar();
  }

  state.openScript = function (path, content, language) {
    if (!state.editor || typeof monaco === "undefined") return;
    let tab = state.openTabs.get(path);
    if (!tab) {
      const model = monaco.editor.createModel(content, language || "plaintext");
      tab = { model, dirty: false, language };
      state.openTabs.set(path, tab);
    } else {
      tab.model.setValue(content);
      tab.dirty = false;
    }
    switchTab(path);
  };

  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.addEventListener("message", (event) => {
      const msg = typeof event.data === "string" ? JSON.parse(event.data) : event.data;
      if (!msg) return;
      if (msg.method === "open_script" && msg.params) {
        state.openScript(msg.params.path, msg.params.content, msg.params.language);
      } else if (msg.method === "project_tree" && msg.params) {
        state.files = msg.params.files || [];
        if (state.activeView === "explorer") renderActiveSidebar();
      } else if (msg.method === "search_results" && msg.params) {
        renderSearchView(document.getElementById("sidebar-content"), msg.params.matches || []);
      }
      state.messageHandlers.forEach((fn) => fn(msg));
    });
  }

  if (typeof require !== "undefined") {
    require.config({ paths: { vs: "vs" } });
    require(["vs/editor/editor.main"], initMonaco);
  }
})();
