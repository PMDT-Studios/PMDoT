import * as React from "react";
import * as jsx from "react/jsx-runtime";
import * as ui from "./index";
import Renderer from "./reconciller";

(globalThis as any).__pmdot = { React, jsx, ui, render: Renderer.render };