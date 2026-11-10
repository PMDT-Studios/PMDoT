export type TextProps = {
    color?: string;
    fontSize?: number;

    fontFamily?: string;
    fontWeight?: "normal" | "bold";

    textAlign?: "left" | "center" | "right";

    children?: string;
};

export function Text({
    color,
    fontSize,
    fontFamily,
    fontWeight,
    textAlign,
    children
}: TextProps) {
    return (
        <ui-text
            color={color}
            fontSize={fontSize}
            fontFamily={fontFamily}
            fontWeight={fontWeight}
            textAlign={textAlign}
        >
            {children}
        </ui-text>
    );
}