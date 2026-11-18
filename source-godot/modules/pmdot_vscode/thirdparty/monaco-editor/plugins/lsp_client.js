(function () {
  const IDE = (window.PMDoTIDE = window.PMDoTIDE || {});

  IDE.extensions = IDE.extensions || {
    errorLens: { name: "Error Lens", desc: "Inline diagnostics at end of line", enabled: true },
    perfCodeLens: { name: "Performance CodeLens", desc: "1Hz CPU frame-time metrics above methods", enabled: true },
    colorSwatches: { name: "Color Swatches", desc: "Inline color pickers for #hex and Color()", enabled: true },
    bracketGuides: { name: "Bracket Pair Colorizer", desc: "Colorized matching brackets & guides", enabled: true },
    todoTree: { name: "Todo Tree", desc: "Scan TODO/FIXME/HACK across project", enabled: true }
  };

  function defineVSCodeDarkPlusTheme() {
    if (typeof monaco === "undefined") return;
    monaco.editor.defineTheme("pmdot-vscode-dark", {
      base: "vs-dark",
      inherit: true,
      rules: [
        { token: "keyword", foreground: "569CD6" },
        { token: "type", foreground: "4EC9B0" },
        { token: "type.identifier", foreground: "4EC9B0" },
        { token: "class", foreground: "4EC9B0" },
        { token: "interface", foreground: "B8D7A3" },
        { token: "function", foreground: "DCDCAA" },
        { token: "method", foreground: "DCDCAA" },
        { token: "variable", foreground: "9CDCFE" },
        { token: "parameter", foreground: "9CDCFE" },
        { token: "property", foreground: "9CDCFE" },
        { token: "string", foreground: "CE9178" },
        { token: "number", foreground: "B5CEA8" },
        { token: "comment", foreground: "6A9955", fontStyle: "italic" }
      ],
      colors: {
        "editor.background": "#1E1E1E",
        "editor.foreground": "#D4D4D4",
        "editorCursor.foreground": "#AEAFAD",
        "editor.lineHighlightBackground": "#2A2D2E",
        "editorLineNumber.foreground": "#858585",
        "editor.selectionBackground": "#264F78"
      }
    });
    monaco.editor.setTheme("pmdot-vscode-dark");
  }

  function registerLanguageCompletions() {
    if (typeof monaco === "undefined") return;

    monaco.languages.registerCompletionItemProvider("csharp", {
      triggerCharacters: [".", " ", "["],
      provideCompletionItems: function (model, position) {
        const word = model.getWordUntilPosition(position);
        const range = {
          startLineNumber: position.lineNumber,
          endLineNumber: position.lineNumber,
          startColumn: word.startColumn,
          endColumn: word.endColumn
        };
        const K = monaco.languages.CompletionItemKind;
        const R = monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet;
        return {
          suggestions: [
            { label: "Node", kind: K.Class, insertText: "Node", detail: "Godot.Node", range },
            { label: "Node2D", kind: K.Class, insertText: "Node2D", detail: "Godot.Node2D", range },
            { label: "CharacterBody2D", kind: K.Class, insertText: "CharacterBody2D", detail: "2D Physics Character", range },
            { label: "Sprite2D", kind: K.Class, insertText: "Sprite2D", detail: "2D Sprite Node", range },
            { label: "AudioStreamPlayer", kind: K.Class, insertText: "AudioStreamPlayer", detail: "Audio Playback Node", range },
            { label: "ShaderMaterial", kind: K.Class, insertText: "ShaderMaterial", detail: "Custom Shader Material", range },
            { label: "Vector2", kind: K.Struct, insertText: "new Vector2(${1:0}, ${2:0})", insertTextRules: R, detail: "Vector2(x, y)", range },
            { label: "Color", kind: K.Color, insertText: "new Color(${1:1f}, ${2:1f}, ${3:1f}, ${4:1f})", insertTextRules: R, detail: "RGBA Color", range },
            { label: "Export", kind: K.Property, insertText: "[Export]\npublic ${1:float} ${2:Speed} { get; set; } = ${3:100f};", insertTextRules: R, detail: "Expose property to PMDoT Inspector", range },
            { label: "Signal", kind: K.Event, insertText: "[Signal]\npublic delegate void ${1:MySignal}EventHandler(${2:});", insertTextRules: R, detail: "Declare PMDoT Signal", range },
            { label: "Input.IsActionPressed", kind: K.Method, insertText: "Input.IsActionPressed(\"${1:ui_accept}\")", insertTextRules: R, detail: "bool Input.IsActionPressed(StringName action)", range },
            { label: "Input.GetVector", kind: K.Method, insertText: "Input.GetVector(\"${1:move_left}\", \"${2:move_right}\", \"${3:move_up}\", \"${4:move_down}\")", insertTextRules: R, detail: "Vector2 4-way input", range },
            { label: "_Ready", kind: K.Method, insertText: "public override void _Ready()\n{\n\t$0\n}", insertTextRules: R, detail: "Node tree entry lifecycle", range },
            { label: "_Process", kind: K.Method, insertText: "public override void _Process(double delta)\n{\n\t$0\n}", insertTextRules: R, detail: "Per-frame update lifecycle", range },
            { label: "_PhysicsProcess", kind: K.Method, insertText: "public override void _PhysicsProcess(double delta)\n{\n\t$0\n}", insertTextRules: R, detail: "Fixed timestep physics lifecycle", range },
            { label: "GetNode<T>", kind: K.Method, insertText: "GetNode<${1:Node}>(\"${2:Path}\")", insertTextRules: R, detail: "T GetNode<T>(NodePath path)", range },
            { label: "GD.Print", kind: K.Function, insertText: "GD.Print(${1:\"message\"});", insertTextRules: R, detail: "Log message to PMDoT Output", range }
          ]
        };
      }
    });

    monaco.languages.registerCompletionItemProvider("cpp", {
      triggerCharacters: [":", ".", ">"],
      provideCompletionItems: function (model, position) {
        const word = model.getWordUntilPosition(position);
        const range = {
          startLineNumber: position.lineNumber,
          endLineNumber: position.lineNumber,
          startColumn: word.startColumn,
          endColumn: word.endColumn
        };
        const K = monaco.languages.CompletionItemKind;
        const R = monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet;
        return {
          suggestions: [
            { label: "std::vector", kind: K.Class, insertText: "std::vector<${1:int}>", insertTextRules: R, detail: "STL Dynamic Array", range },
            { label: "std::string", kind: K.Class, insertText: "std::string", detail: "STL String", range },
            { label: "GDCLASS", kind: K.Snippet, insertText: "GDCLASS(${1:ClassName}, ${2:Node});", insertTextRules: R, detail: "PMDoT Class Macro", range },
            { label: "_bind_methods", kind: K.Method, insertText: "static void _bind_methods();", detail: "ClassDB method binder", range },
            { label: "ClassDB::bind_method", kind: K.Function, insertText: "ClassDB::bind_method(D_METHOD(\"${1:method}\"), &${2:Class}::${1:method});", insertTextRules: R, detail: "Expose C++ method", range }
          ]
        };
      }
    });

    // Inline Color Swatches for #RRGGBB
    ["csharp", "cpp", "glsl"].forEach((lang) => {
      monaco.languages.registerColorProvider(lang, {
        provideColorPresentations: (model, colorInfo) => {
          const c = colorInfo.color;
          const toHex = (v) => Math.round(v * 255).toString(16).padStart(2, "0");
          return [{ label: `#${toHex(c.red)}${toHex(c.green)}${toHex(c.blue)}` }];
        },
        provideDocumentColors: (model) => {
          if (!IDE.extensions.colorSwatches.enabled) return [];
          const matches = model.findMatches("#([0-9a-fA-F]{6})\\b", false, true, false, null, true);
          return matches.map((m) => {
            const hex = m.matches[1];
            return {
              range: m.range,
              color: {
                red: parseInt(hex.slice(0, 2), 16) / 255,
                green: parseInt(hex.slice(2, 4), 16) / 255,
                blue: parseInt(hex.slice(4, 6), 16) / 255,
                alpha: 1
              }
            };
          });
        }
      });
    });

    const docLinks = {
      csharp: { title: "C# / .NET Documentation", url: "https://learn.microsoft.com/en-us/dotnet/csharp/" },
      cpp: { title: "C++ Reference (cppreference)", url: "https://en.cppreference.com/w/" },
      java: { title: "Java SE Documentation", url: "https://docs.oracle.com/en/java/" }
    };

    ["csharp", "cpp", "java"].forEach((lang) => {
      monaco.languages.registerHoverProvider(lang, {
        provideHover: function (model, position) {
          const word = model.getWordAtPosition(position);
          if (!word) return null;
          const doc = docLinks[lang];
          return {
            contents: [
              { value: `**${word.word}** (\`${lang}\`)` },
              { value: `[${doc.title}](${doc.url})` }
            ]
          };
        }
      });
    });
  }

  function renderExtensionsSidebar(container) {
    container.innerHTML = `<div class="section-title">PMDoT GameDev Extensions</div>`;
    Object.keys(IDE.extensions).forEach((key) => {
      const ext = IDE.extensions[key];
      const row = document.createElement("div");
      row.className = "tree-item";
      row.style.justifyContent = "space-between";
      row.innerHTML = `
        <div style="overflow:hidden;text-overflow:ellipsis;">
          <div style="font-weight:600;color:#fff;">${ext.name}</div>
          <div style="font-size:11px;color:#8c8c8c;">${ext.desc}</div>
        </div>
        <input type="checkbox" ${ext.enabled ? "checked" : ""} />
      `;
      const cb = row.querySelector("input");
      cb.addEventListener("change", () => {
        ext.enabled = cb.checked;
        if (key === "bracketGuides" && IDE.editor) {
          IDE.editor.updateOptions({ bracketPairColorization: { enabled: ext.enabled } });
        }
      });
      container.appendChild(row);
    });
  }

  IDE.applyConfig = function (cfg) {
    const editor = IDE.editor;
    if (!editor || !cfg) return;
    if (cfg.theme && typeof monaco !== "undefined") {
      monaco.editor.setTheme(cfg.theme === "vs-dark" ? "pmdot-vscode-dark" : cfg.theme);
    }
    editor.updateOptions({
      fontSize: cfg.fontSize || 14,
      minimap: { enabled: cfg.minimap !== false },
      fontLigatures: cfg.fontLigatures !== false,
      "semanticHighlighting.enabled": true
    });
  };

  function boot() {
    defineVSCodeDarkPlusTheme();
    registerLanguageCompletions();
    if (IDE.registerSidebarView) {
      IDE.registerSidebarView("extensions", "EXTENSIONS", renderExtensionsSidebar);
    }
    if (IDE.registerCommand) {
      IDE.registerCommand("view.extensions", "View: Show Extensions", () => IDE.switchSidebarView("extensions"));
    }
  }

  if (typeof monaco !== "undefined") {
    boot();
  } else {
    window.addEventListener("load", () => setTimeout(boot, 300));
  }

  if (IDE.onMessage) {
    IDE.onMessage((msg) => {
      if (msg && msg.method === "apply_config" && msg.params) {
        IDE.applyConfig(msg.params);
      }
    });
  }
})();
