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
	
	if(!parent || !IsWindow(parent)) return false;
	
	parent_xid = parent;
	current_bounds = p_bounds;
	
	GdkDisplay *gdk_display = gdk_display_get_default();

	if(!gdk_display || !GDK_IS_X11_DISPLAY(gdk_display)) return false;

	xdisplay = gdk_x11_display_get_xdisplay(gdk_display);

	GdkWindow *parent_gdk = gdk_x11_window_foreign_new_for_display(gdk_display, parent_xid);

	if(!parent_gdk) return false;

	GdkWindowAttr attr = {};
	attr.window_type = GDK_WINDOW_CHILD;
	attr.x = p_bounds.position.x;
	arrt.y = p_bounds.position.y;
	attr.width = MAX(1, p_bounds.size.x);
	attr.height = MAX(1, p_bounds.size.y);
	attr.wclass = GDK_INPUT_OUTPUT;
	attr.event_mask = GDK_EXPOSURE_MASK;

	GdkWindow *host = gdk_window_new(
		parent_gdk,
		&attr,
		GDK_WA_X | GDK_WA_Y
	);

	g_object_unref(parent_gdk);

    if (!host) {
        return false;
    }

    host_gdk = host;

    host_xid = gdk_x11_window_get_xid(host_gdk);

	

	
    gdk_window_show(host_gdk);
	ready = true;
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
