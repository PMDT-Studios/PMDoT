#ifdef LINUX_ENABLED

#include "webview_posix.h"

#include "core/io/dir_access.h"
#include "core/os/os.h"

#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <X11/Xlib.h>

static String _resolve_monaco_directory() {
	String exe_dir = OS::get_singleton()->get_executable_path().get_base_dir();
	String packaged = exe_dir.path_join("pmdot_vscode_ide");
	if (DirAccess::exists(packaged)) {
		return packaged;
	}
	String dev_path = exe_dir.get_base_dir().path_join("modules/pmdot_vscode/thirdparty/monaco-editor");
	if (DirAccess::exists(dev_path)) {
		return dev_path;
	}
	String cwd_path = OS::get_singleton()->get_resource_dir().path_join("../source-godot/modules/pmdot_vscode/thirdparty/monaco-editor");
	if (DirAccess::exists(cwd_path)) {
		return cwd_path;
	}
	return dev_path;
}

WebViewLinux::~WebViewLinux(){
	shutdown();
}

bool WebViewLinux::initialize(int64_t p_native_window_handle, const Rect2i &p_bounds) {
 	Window parent = static_cast<Window>(p_native_window_handle);
	
	if(!parent) return false;
	
	parent_xid = parent;
	current_bounds = p_bounds;
	
	GdkDisplay *gdk_display = gdk_display_get_default();

	if(!gdk_display || !GDK_IS_X11_DISPLAY(gdk_display)) return false;

	xdisplay = gdk_x11_display_get_xdisplay(gdk_display);

	if(!parent_gdk) return false;

	
	plug = gtk_plug_new(parent_xid);

	if(!plug) return false;

	webview = webkit_web_view_new();

    if (!webview) {
        gtk_widget_destroy(plug);
        plug = nullptr;
        return false;
    }

	gtk_container_add(GTK_CONTAINER(plug), webview);

	gtk_widget_set_size_request(
        webview,
        MAX(1, p_bounds.size.x),
        MAX(1, p_bounds.size.y)
    );

	g_signal_connect(
        webview,
        "load-changed",
        G_CALLBACK(+[](WebKitWebView *, WebKitLoadEvent event, gpointer user_data) {
            WebViewLinux *self = static_cast<WebViewLinux *>(user_data);

            if (event == WEBKIT_LOAD_FINISHED) {
                self->ready = true;
            }
        }),
        this
    );

	ready = false;

	entra aqui 
	
	gtk_widget_show_all(plug);

	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), "https://pmdot.ide/index.html");

	return ready;
}

void WebViewLinux::shutdown() {
}

void WebViewLinux::set_bounds(const Rect2i &p_bounds) {
	current_bounds = p_bounds;

    if (host_gdk) {
        gdk_window_move_resize(
            host_gdk,
            p_bounds.position.x,
            p_bounds.position.y,
            MAX(1, p_bounds.size.x),
            MAX(1, p_bounds.size.y)
        );
    }
}

void WebViewLinux::set_visible(bool p_visible) {
}

void WebViewLinux::navigate(const String &p_url) {
}

void WebViewLinux::execute_script(const String &p_js) {
}

void WebViewLinux::post_message(const String &p_json) {
}

void WebViewLinux::set_message_callback(MessageCallback p_callback) {
}

WebViewBackend *WebViewBackend::create() {
	return memnew(WebViewLinux);
}

#endif //lINUX_ENABLED
