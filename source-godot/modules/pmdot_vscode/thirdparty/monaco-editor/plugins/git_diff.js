(function () {
  const IDE = (window.PMDoTIDE = window.PMDoTIDE || {});
  let gitDecorations = [];
  let diffEditor = null;
  const gitState = {
    branch: "main",
    changes: [],
    commits: [],
    commitsLoaded: false
  };

  function renderGutter(hunks) {
    const editor = IDE.editor;
    if (!editor || typeof monaco === "undefined") return;
    const decos = (hunks || []).map((h) => {
      let cls = "pmdot-git-modified";
      if (h.type === 0) cls = "pmdot-git-added";
      else if (h.type === 2) cls = "pmdot-git-deleted";
      return {
        range: new monaco.Range(h.startLine, 1, h.endLine, 1),
        options: { isWholeLine: true, linesDecorationsClassName: cls }
      };
    });
    gitDecorations = editor.deltaDecorations(gitDecorations, decos);
  }

  function openSideBySideDiff(params) {
    if (typeof monaco === "undefined") return;
    const root = document.getElementById("monaco-root");
    if (!diffEditor) {
      diffEditor = monaco.editor.createDiffEditor(root, {
        theme: "pmdot-vscode-dark",
        automaticLayout: true,
        renderSideBySide: true
      });
    }
    const origModel = monaco.editor.createModel(params.original || "", params.language || "plaintext");
    const modModel = monaco.editor.createModel(params.modified || "", params.language || "plaintext");
    diffEditor.setModel({ original: origModel, modified: modModel });
  }

  function updateBadges() {
    const badge = document.getElementById("badge-scm");
    if (badge) {
      badge.textContent = String(gitState.changes.length);
      badge.style.display = gitState.changes.length > 0 ? "flex" : "none";
    }
    const branchEl = document.getElementById("status-branch");
    if (branchEl) branchEl.textContent = gitState.branch || "main";
  }

  function renderSCMSidebar(container) {
    container.innerHTML = `
      <div style="padding:8px;border-bottom:1px solid #1e1e1e;">
        <input id="scm-msg" class="sidebar-input" style="margin:0 0 6px 0;width:100%;" placeholder="Commit message..." />
        <div style="display:flex;gap:6px;">
          <button id="scm-commit" class="icon-btn" style="flex:1;justify-content:center;">Commit</button>
          <button id="scm-graph" class="icon-btn secondary">Graph</button>
          <button id="scm-refresh" class="icon-btn secondary">Sync</button>
        </div>
      </div>
      <div class="section-title">Changes (${gitState.changes.length})</div>
      <div id="scm-list"></div>
    `;

    container.querySelector("#scm-commit").addEventListener("click", () => {
      const msg = container.querySelector("#scm-msg").value;
      if (msg.trim()) {
        IDE.postToGodot("git_commit_req", { message: msg.trim() });
        container.querySelector("#scm-msg").value = "";
      }
    });
    container.querySelector("#scm-graph").addEventListener("click", () => {
      IDE.postToGodot("git_graph_req", {});
      IDE.openBottomTab("git-graph");
    });
    container.querySelector("#scm-refresh").addEventListener("click", () => {
      IDE.postToGodot("git_status_req", {});
    });

    const list = container.querySelector("#scm-list");
    if (!gitState.changes.length) {
      list.innerHTML = `<div class="tree-item" style="color:#858585;">Working tree clean</div>`;
      return;
    }

    gitState.changes.forEach((c) => {
      const isStaged = c.index !== " " && c.index !== "?";
      const statusChar = c.worktree !== " " ? c.worktree : c.index;
      const color = statusChar === "?" || statusChar === "A" ? "#2ea043" : statusChar === "D" ? "#f85149" : "#e2c08d";
      const row = document.createElement("div");
      row.className = "tree-item";
      row.style.justifyContent = "space-between";
      row.innerHTML = `
        <span class="scm-file" style="overflow:hidden;text-overflow:ellipsis;">${c.path}</span>
        <span style="display:flex;align-items:center;gap:6px;">
          <button class="icon-btn secondary scm-act" style="padding:1px 5px;">${isStaged ? "-" : "+"}</button>
          <span style="color:${color};font-weight:700;">${statusChar}</span>
        </span>
      `;
      row.querySelector(".scm-file").addEventListener("click", () => {
        IDE.postToGodot("git_diff_req", { path: c.path });
      });
      row.querySelector(".scm-act").addEventListener("click", (e) => {
        e.stopPropagation();
        IDE.postToGodot(isStaged ? "git_unstage_req" : "git_stage_req", { path: c.path });
      });
      list.appendChild(row);
    });
  }

  function renderGitGraphTab(el) {
    if (!gitState.commitsLoaded) {
      IDE.postToGodot("git_graph_req", {});
      el.innerHTML = `<div style="color:#858585;">Loading Git Graph...</div>`;
      return;
    }
    if (!gitState.commits.length) {
      el.innerHTML = `<div style="color:#858585;">No git commits found.</div>`;
      return;
    }
    const rowH = 24;
    const svgH = gitState.commits.length * rowH + 10;
    const colors = ["#007acc", "#2ea043", "#c586c0", "#dcdcaa"];
    let svgLines = "";
    let rowsHtml = "";

    gitState.commits.forEach((c, idx) => {
      const y = idx * rowH + 12;
      const lane = c.parents && c.parents.includes(" ") ? 1 : 0;
      const x = 14 + lane * 16;
      const color = colors[lane % colors.length];
      if (idx < gitState.commits.length - 1) {
        svgLines += `<line x1="${x}" y1="${y}" x2="14" y2="${y + rowH}" stroke="${color}" stroke-width="2" />`;
      }
      svgLines += `<circle cx="${x}" cy="${y}" r="4.5" fill="${color}" stroke="#1e1e1e" stroke-width="1.5" />`;
      rowsHtml += `
        <div style="height:${rowH}px;display:flex;align-items:center;gap:10px;font-size:12px;white-space:nowrap;">
          <span style="color:#4ec9b0;font-family:monospace;">${c.hash}</span>
          <span style="color:#ffffff;flex:1;overflow:hidden;text-overflow:ellipsis;">${c.message}</span>
          <span style="color:#858585;">${c.author}</span>
          <span style="color:#6a9955;">${c.date}</span>
        </div>
      `;
    });

    el.innerHTML = `
      <div style="display:flex;gap:8px;">
        <svg width="48" height="${svgH}" style="flex-shrink:0;">${svgLines}</svg>
        <div style="flex:1;min-width:0;">${rowsHtml}</div>
      </div>
    `;
  }

  function boot() {
    if (IDE.registerSidebarView) {
      IDE.registerSidebarView("scm", "SOURCE CONTROL", renderSCMSidebar);
    }
    if (IDE.registerBottomTab) {
      IDE.registerBottomTab("git-graph", "Git Graph", renderGitGraphTab);
    }
    if (IDE.registerCommand) {
      IDE.registerCommand("git.status", "Git: Show Source Control", () => IDE.switchSidebarView("scm"));
      IDE.registerCommand("git.graph", "Git: Open Commit Graph", () => {
        IDE.postToGodot("git_graph_req", {});
        IDE.openBottomTab("git-graph");
      });
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
      if (msg.method === "git_gutter_update") {
        renderGutter(msg.params.hunks);
      } else if (msg.method === "git_open_diff") {
        openSideBySideDiff(msg.params);
      } else if (msg.method === "git_status_update") {
        gitState.branch = msg.params.branch || "main";
        gitState.changes = msg.params.changes || [];
        updateBadges();
        if (IDE.activeView === "scm") IDE.switchSidebarView("scm");
      } else if (msg.method === "git_graph_update") {
        gitState.commitsLoaded = true;
        gitState.commits = msg.params.commits || [];
        if (IDE.activeBottomTab === "git-graph") IDE.openBottomTab("git-graph");
      }
    });
  }
})();
