(function () {
  const SVG_NS = "http://www.w3.org/2000/svg";

  function svg(paths, size = 18, viewBox = "0 0 16 16") {
    return `<svg xmlns="${SVG_NS}" width="${size}" height="${size}" viewBox="${viewBox}" fill="currentColor" aria-hidden="true">${paths}</svg>`;
  }

  const ICONS = {
    explorer: svg('<path d="M14.5 3H7.71l-.85-.85L6.51 2h-5l-.5.5v11l.5.5h13l.5-.5v-10L14.5 3zm-.51 8.49V13h-12V7h4.49l.35-.15.86-.86H14v5.5zm0-6.49H7.5l-.35.15-.86.86H2v-3h4.29l.85.85.36.15H14l-.01 1.99z"/>', 20),
    search: svg('<path d="M15.25 14.19l-3.9-3.9a5.5 5.5 0 1 0-1.06 1.06l3.9 3.9 1.06-1.06zM6.5 10.5a4 4 0 1 1 0-8 4 4 0 0 1 0 8z"/>', 20),
    scm: svg('<path d="M13 3.5a1.5 1.5 0 1 0-1.99 1.415V6.5a1.5 1.5 0 0 1-1.5 1.5H6a2.5 2.5 0 0 0-2 1V4.915a1.5 1.5 0 1 0-1 0V11.085a1.5 1.5 0 1 0 1 0V10.5A1.5 1.5 0 0 1 5.5 9h4A2.5 2.5 0 0 0 12 6.5V4.915A1.5 1.5 0 0 0 13 3.5z"/>', 20),
    debug: svg('<path d="M13.5 8h-1.29a4.5 4.5 0 0 0-.42-1.02l.91-.91-.7-.71-1 .99A4.46 4.46 0 0 0 9.5 5.58V4h-1v1.58A4.46 4.46 0 0 0 7 6.35l-1-.99-.7.71.91.91A4.5 4.5 0 0 0 5.79 8H4.5v1h1.21a4.48 4.48 0 0 0 .35 1.15l-.95.95.71.71.91-.91A4.5 4.5 0 0 0 8 11.79V13h1v-1.21a4.5 4.5 0 0 0 1.27-.89l.91.91.71-.71-.95-.95A4.48 4.48 0 0 0 11.29 9H13.5V8zM8.5 10.8a2.3 2.3 0 1 1 0-4.6 2.3 2.3 0 0 1 0 4.6zM4 2l3 2-3 2V2z"/>', 20),
    testing: svg('<path d="M11 2V1H5v1h1v3.29L2.27 11.5A1.5 1.5 0 0 0 3.56 13.8h8.88a1.5 1.5 0 0 0 1.29-2.3L10 5.29V2h1zM9 2v3.6l3.87 6.45a.5.5 0 0 1-.43.75H3.56a.5.5 0 0 1-.43-.75L7 5.6V2h2z"/>', 20),
    extensions: svg('<path d="M13.5 6H10V2.5L9.5 2h-5l-.5.5V6H2.5L2 6.5v5l.5.5H6v1.5l.5.5h5l.5-.5V12h1.5l.5-.5v-5l-.5-.5zM9 3v3H5V3h4zm-6 8V7h3v4H3zm8 2H7v-1h2.5l.5-.5V7h3v4h-2v2z"/>', 20),
    remote: svg('<path d="M8 1a7 7 0 1 0 0 14A7 7 0 0 0 8 1zm5.93 6.5h-2.46a10.4 10.4 0 0 0-.96-4.18A6.02 6.02 0 0 1 13.93 7.5zM8 14c-.83 0-1.95-1.56-2.39-4.5h4.78C9.95 12.44 8.83 14 8 14zm-2.48-5.5c-.07-.66-.07-1.34 0-2h4.96c.07.66.07 1.34 0 2H5.52zM5.61 6.5C6.05 3.56 7.17 2 8 2s1.95 1.56 2.39 4.5H5.61zM5.49 2.32A10.4 10.4 0 0 0 4.53 6.5H2.07a6.02 6.02 0 0 1 3.42-4.18zM2.07 8.5h2.46c.16 1.54.5 2.97.96 4.18A6.02 6.02 0 0 1 2.07 8.5zm8.44 4.18c.46-1.21.8-2.64.96-4.18h2.46a6.02 6.02 0 0 1-3.42 4.18z"/>', 20),
    sceneGraph: svg('<path d="M6.5 2h-4l-.5.5v3l.5.5h4l.5-.5v-3L6.5 2zM6 5H3V3h3v2zm7.5 4h-4l-.5.5v3l.5.5h4l.5-.5v-3l-.5-.5zM13 12h-3v-2h3v2zm.5-10h-4l-.5.5v3l.5.5h4l.5-.5v-3L13.5 2zM13 5h-3V3h3v2zM7 4h2V3H7v1zm-2.5 6.5V6h-1v5l.5.5H9v-1H4.5z"/>', 20),
    newFile: svg('<path d="M9.5 1.1l3.4 3.5.1.4v2h-1V5H8V1H3v13h5v1H2.5l-.5-.5v-14l.5-.5h6.7l.3.1zM9 2v3h3L9 2zm4 8v2h2v1h-2v2h-1v-2h-2v-1h2v-2h1z"/>', 14),
    close: svg('<path d="M8 8.71l3.65 3.64.7-.7L8.71 8l3.64-3.65-.7-.7L8 7.29 4.35 3.65l-.7.7L7.29 8l-3.64 3.65.7.7L8 8.71z"/>', 12),
    chevronRight: svg('<path d="M5.7 13.7L5 13l4.6-5L5 3l.7-.7L11 8l-5.3 5.7z"/>', 12),
    symbolClass: svg('<path d="M13 3H7L6 4v2H3L2 7v6l1 1h6l1-1v-2h3l1-1V4l-1-1zM9 13H3V7h6v6zm4-3h-3V7L9 6H7V4h6v6z" fill="#EE9D28"/>', 14),
    symbolMethod: svg('<path d="M13.5 5.2l-5-3h-1l-5 3-.5.8v4l.5.8 5 3h1l5-3 .5-.8V6l-.5-.8zM8 3.2l4.2 2.5L8 8.2 3.8 5.7 8 3.2zM3 6.5l4.5 2.7v4.6L3 11.1V6.5zm5.5 7.3V9.2L13 6.5v4.6l-4.5 2.7z" fill="#B180D7"/>', 14),
    fileCs: svg('<path d="M13 2H5L4 3v10l1 1h8l1-1V3l-1-1zm0 11H5V3h8v10z" fill="#519ABA"/><text x="5.3" y="10.8" font-size="6.2" font-family="monospace" font-weight="bold" fill="#519ABA">C#</text>', 15),
    fileCpp: svg('<path d="M13 2H5L4 3v10l1 1h8l1-1V3l-1-1zm0 11H5V3h8v10z" fill="#659AD2"/><text x="5.1" y="10.5" font-size="5.5" font-family="monospace" font-weight="bold" fill="#659AD2">C+</text>', 15),
    fileJava: svg('<path d="M13 2H5L4 3v10l1 1h8l1-1V3l-1-1zm0 11H5V3h8v10z" fill="#E37933"/><text x="6.5" y="10.8" font-size="6.5" font-family="monospace" font-weight="bold" fill="#E37933">J</text>', 15),
    fileMd: svg('<path d="M13 2H5L4 3v10l1 1h8l1-1V3l-1-1zm0 11H5V3h8v10z" fill="#CCCCCC"/><text x="5.5" y="10.8" font-size="6.2" font-family="monospace" font-weight="bold" fill="#CCCCCC">M</text>', 15),
    fileDefault: svg('<path d="M9.5 1H3.5L3 1.5v13l.5.5h9l.5-.5V4.5L9.5 1zM12 14H4V2h5v3h3v9z" fill="#CCCCCC"/>', 15)
  };

  window.PMDoTIDE = window.PMDoTIDE || {};
  window.PMDoTIDE.icons = ICONS;
  window.PMDoTIDE.getFileIcon = function (lang) {
    if (lang === "csharp") return ICONS.fileCs;
    if (lang === "cpp") return ICONS.fileCpp;
    if (lang === "java") return ICONS.fileJava;
    if (lang === "markdown") return ICONS.fileMd;
    return ICONS.fileDefault;
  };
})();
