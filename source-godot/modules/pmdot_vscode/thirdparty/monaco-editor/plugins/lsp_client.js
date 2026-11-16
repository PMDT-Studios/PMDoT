(function () {
  window.PMDoTIDE = window.PMDoTIDE || {};

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

    // C# (.NET / PMDoT API) IntelliSense
    monaco.languages.registerCompletionItemProvider("csharp", {
      triggerCharacters: [".", " "],
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
            { label: "Node", kind: K.Class, insertText: "Node", detail: "Godot.Node (C# Base Node)", range },
            { label: "Node2D", kind: K.Class, insertText: "Node2D", detail: "Godot.Node2D (2D Spatial Node)", range },
            { label: "CharacterBody2D", kind: K.Class, insertText: "CharacterBody2D", detail: "Godot.CharacterBody2D", range },
            { label: "Vector2", kind: K.Struct, insertText: "new Vector2(${1:0}, ${2:0})", insertTextRules: R, detail: "Godot.Vector2(float x, float y)", range },
            { label: "_Ready", kind: K.Method, insertText: "public override void _Ready()\n{\n\t$0\n}", insertTextRules: R, detail: "Lifecycle: called when node enters scene tree", range },
            { label: "_Process", kind: K.Method, insertText: "public override void _Process(double delta)\n{\n\t$0\n}", insertTextRules: R, detail: "Lifecycle: called every frame", range },
            { label: "_PhysicsProcess", kind: K.Method, insertText: "public override void _PhysicsProcess(double delta)\n{\n\t$0\n}", insertTextRules: R, detail: "Lifecycle: fixed physics step", range },
            { label: "GD.Print", kind: K.Function, insertText: "GD.Print(${1:\"message\"});", insertTextRules: R, detail: "void GD.Print(params object[] what)", range },
            { label: "Console.WriteLine", kind: K.Function, insertText: "Console.WriteLine(${1:\"\"});", insertTextRules: R, detail: "System.Console.WriteLine", range },
            { label: "GetNode<T>", kind: K.Method, insertText: "GetNode<${1:Node}>(\"${2:Path}\")", insertTextRules: R, detail: "T GetNode<T>(NodePath path)", range }
          ]
        };
      }
    });

    // C++ (GDExtension / PMDoT Native) IntelliSense
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
            { label: "GDCLASS", kind: K.Snippet, insertText: "GDCLASS(${1:ClassName}, ${2:Node});", insertTextRules: R, detail: "PMDoT/Godot Class Binding Macro", range },
            { label: "_bind_methods", kind: K.Method, insertText: "static void _bind_methods();", detail: "ClassDB method binder", range }
          ]
        };
      }
    });

    // Hover Documentation linked to Language Docs (C#, C++, Java)
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

  window.PMDoTIDE.applyConfig = function (cfg) {
    const editor = window.PMDoTIDE.editor;
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

  if (typeof monaco !== "undefined") {
    defineVSCodeDarkPlusTheme();
    registerLanguageCompletions();
  } else {
    window.addEventListener("load", () => {
      setTimeout(() => {
        defineVSCodeDarkPlusTheme();
        registerLanguageCompletions();
      }, 300);
    });
  }

  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.addEventListener("message", (event) => {
      const msg = typeof event.data === "string" ? JSON.parse(event.data) : event.data;
      if (msg && msg.method === "apply_config" && msg.params) {
        window.PMDoTIDE.applyConfig(msg.params);
      }
    });
  }
})();
