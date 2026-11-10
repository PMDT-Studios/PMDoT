import type { ReactNode } from "react";

export type CustomShaderProps = {
    shader: string;
    uniforms?: Record<string, number | number[]>;
    children?: ReactNode;
};

export function CustomShader({
    shader,
    uniforms,
    children
}: CustomShaderProps) {
    return (
        <ui-custom-shader
            shader={shader}
            uniforms={uniforms}
        >
            {children}
        </ui-custom-shader>
    );
}