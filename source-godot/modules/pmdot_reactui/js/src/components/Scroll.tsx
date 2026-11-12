import type { ReactNode } from "react";

export type ScrollProps = {
    width?: number;
    height?: number;

    direction?: "vertical" | "horizontal";

    scrollX?: number;
    scrollY?: number;

    scrollbarVisible?: boolean;

    children?: ReactNode;
};

export function Scroll({
    width,
    height,
    direction,
    scrollX,
    scrollY,
    scrollbarVisible,
    children
}: ScrollProps) {
    return (
        <ui-scroll
            width={width}
            height={height}
            direction={direction}
            scrollX={scrollX}
            scrollY={scrollY}
            scrollbarVisible={scrollbarVisible}
        >
            {children}
        </ui-scroll>
    );
}