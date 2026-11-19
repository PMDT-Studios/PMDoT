import { build } from "esbuild";

await build({
    entryPoints: ["src/base.ts"],
    bundle: true,
    format: "iife",
    plataform: "browser",
    target: "es2020",
    define: { "process.env.NODE_ENV": '"production"' },
    outfile: "dist/base.js",
});