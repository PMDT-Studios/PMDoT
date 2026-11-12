import type { ReactNode } from "react";
import type {
    ViewProps,
    TextProps,
    ImageProps,
    ScrollProps,
    CustomShaderProps
} from "./index";

declare module "react" {
    namespace JSX {
        interface IntrinsicElements {

            "ui-view": ViewProps;

            "ui-text": TextProps;

            "ui-image": ImageProps;

            "ui-scroll": ScrollProps;

            "ui-custom-shader": CustomShaderProps;
        }
    }
}