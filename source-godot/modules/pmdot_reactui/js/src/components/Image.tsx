export type ImageProps = {
    source: string;

    width?: number;
    height?: number;

    fit?: "contain" | "cover" | "stretch";

    opacity?: number;

    children?: never;
};

export function Image({
    source,
    width,
    height,
    fit,
    opacity
}: ImageProps) {
    return (
        <ui-image
            source={source}
            width={width}
            height={height}
            fit={fit}
            opacity={opacity}
        />
    );
}