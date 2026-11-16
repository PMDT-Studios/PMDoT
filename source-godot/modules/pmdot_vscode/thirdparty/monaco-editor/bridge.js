(function () {
  const state = {
    editor: null,
    ready: false,
    currentPath: "",
    dirty: false,
    files: [],
    openTabs: new Map(), // path -> { model, dirty, language }
    activeView: "explorer"
  };
  window.PMDoTIDE = state;

  function postToGodot(method, params) {
    const payload = JSON.stringify({ method, params });
    if (window.chrome && window.chrome.webview) {
      window.chrome.webview.postMessage(payload);
    }
  }
  window.PMDoTIDE.postToGodot = postToGodot;

  function langShort(lang) {
    if (lang === "csharp") return "C#";
    if (lang === "cpp") return "C++";
    if (lang === "java") return "JV";
    if (lang === "markdown") return "MD";
    return "TXT";
  }

  function renderTabs() {
    const bar = document.getElementById("tab-bar");
    if (!bar) return;
    bar.innerHTML = "";
    state.openTabs.forEach((tab, path) => {
      const el = document.createElement("div");
      el.className = "editor-tab" + (path === state.currentPath ? " active" : "");
      const fileName = path.split("/").pop();
      el.innerHTML = `<span>${fileName}${tab.dirty ? " ●" : ""}</span><span class="tab-close" data-close="${path}">✕</span>`;
      el.addEventListener("click", (e) => {
        if (e.target.dataset.close) {
          closeTab(e.target.dataset.close);
        } else {
          switchTab(path);
        }
      });
      bar.appendChild(el);
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
    renderExplorer();
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
      }
    }
    renderTabs();
  }

  function renderExplorer() {
    if (state.activeView !== "explorer") return;
    const container = document.getElementById("sidebar-content");
    if (!container) return;
    container.innerHTML = "";
    state.files.forEach((f) => {
      const item = document.createElement("div");
      item.className = "tree-item" + (f.path === state.currentPath ? " active" : "");
      item.title = f.path;
      item.innerHTML = `<span class="lang-badge">${langShort(f.language)}</span><span>${f.name}</span>`;
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

  function renderSearchView(matches) {
    if (state.activeView !== "search") return;
    const container = document.getElementById("sidebar-content");
    if (!container) return;
    if (!document.getElementById("search-box")) {
      container.innerHTML = `<input id="search-box" type="text" placeholder="Search in project files..." /><div id="search-results"></div>`;
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
      el.innerHTML = `<span class="lang-badge">L${m.line}</span><span>${m.name}: ${m.preview}</span>`;
      el.addEventListener("click", () => {
        postToGodot("open_file_req", { path: m.path });
      });
      resDiv.appendChild(el);
    });
  }

  function updateStatusBar() {
    const right = document.getElementById("status-right");
    if (!right || !state.editor) return;
    const pos = state.editor.getPosition() || { lineNumber: 1, column: 1 };
    const model = state.editor.getModel();
    const lang = model ? model.getLanguageId() : "plaintext";
    right.textContent = `Ln ${pos.lineNumber}, Col ${pos.column} | UTF-8 | ${lang}`;
  }

  function initMonaco() {
    if (typeof monaco === "undefined") return;

    const editor = monaco.editor.create(document.getElementById("monaco-root"), {
      value: "// Select or create a file in the Explorer (Ctrl+P for Quick Open)\n",
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

    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyP, openQuickOpen);

    document.querySelectorAll(".act-btn").forEach((btn) => {
      btn.addEventListener("click", () => {
        document.querySelectorAll(".act-btn").forEach((b) => b.classList.remove("active"));
        btn.classList.add("active");
        state.activeView = btn.dataset.view;
        document.getElementById("sidebar-title").textContent =
          state.activeView === "search" ? "SEARCH" : "EXPLORER";
        if (state.activeView === "explorer") renderExplorer();
        else renderSearchView([]);
      });
    });

    document.getElementById("btn-new-file").addEventListener("click", () => {
      const name = prompt("New file path (e.g. scripts/Player.cs, src/Main.cpp, README.md):", "scripts/NewScript.cs");
      if (name && name.trim()) {
        postToGodot("create_file_req", { path: name.trim() });
      }
    });

    postToGodot("ready", {});
    updateStatusBar();
  }

  function openQuickOpen() {
    const overlay = document.getElementById("quick-open-overlay");
    const input = document.getElementById("quick-open-input");
    const list = document.getElementById("quick-open-list");
    overlay.style.display = "block";
    input.value = "";
    input.focus();

    function refreshList() {
      const q = input.value.toLowerCase();
      list.innerHTML = "";
      state.files
        .filter((f) => f.path.toLowerCase().includes(q))
        .slice(0, 20)
        .forEach((f) => {
          const el = document.createElement("div");
          el.className = "tree-item";
          el.innerHTML = `<span class="lang-badge">${langShort(f.language)}</span><span>${f.path}</span>`;
          el.addEventListener("click", () => {
            overlay.style.display = "none";
            postToGodot("open_file_req", { path: f.path });
          });
          list.appendChild(el);
        });
    }
    input.oninput = refreshList;
    input.onkeydown = (e) => {
      if (e.key === "Escape") overlay.style.display = "none";
    };
    refreshList();
  }

  window.PMDoTIDE.openScript = function (path, content, language) {
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
      if (!msg || !msg.params) return;
      if (msg.method === "open_script") {
        window.PMDoTIDE.openScript(msg.params.path, msg.params.content, msg.params.language);
      } else if (msg.method === "project_tree") {
        state.files = msg.params.files || [];
        renderExplorer();
      } else if (msg.method === "search_results") {
        renderSearchView(msg.params.matches || []);
      }
    });
  }

  if (typeof require !== "undefined") {
    require.config({ paths: { vs: "vs" } });
    require(["vs/editor/editor.main"], initMonaco);
  }
})();
