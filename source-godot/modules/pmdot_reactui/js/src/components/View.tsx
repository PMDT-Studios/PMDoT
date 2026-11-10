import type { ReactNode } from "react";

export type ViewProps = {
    width?: number;
    height?: number;

    backgroundColor?: string;

    padding?: number;
    margin?: number;

    gap?: number;

    direction?: "row" | "column";

    align?: "start" | "center" | "end";
    justify?: "start" | "center" | "end" | "space-between";

    children?: ReactNode;
};

export function View({
    width,
    height,
    backgroundColor,
    padding,
    margin,
    gap,
    direction,
    align,
    justify,
    children
}: ViewProps) {
    return (
        <ui-view
            width={width}
            height={height}
            backgroundColor={backgroundColor}
            padding={padding}
            margin={margin}
            gap={gap}
            direction={direction}
            align={align}
            justify={justify}
        >
            {children}
        </ui-view>
    );
}