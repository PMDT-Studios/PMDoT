import Reconciler from 'react-reconciler';

const HostConfig = {
    
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