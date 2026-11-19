import Reconciler from 'react-reconciler';

const HostConfig = {
    createInstance(type: string, props: any){
        const serializedProps = JSON.stringify(props);

        globalThis.nativeCreateElement(type, serializedProps);

        return {
            type,
            props,
            children: []
        };
    },

};

const MyRenderer = Reconciler(HostConfig);

interface RendererPublicAPI {
    render(
        element: React.ReactNode,
        container: unknown,
        callback?: () => void
    ): void;
}

const RendererPublicAPI: RendererPublicAPI = {
    render(element, container, callback) {

    }
};

export default RendererPublicAPI;