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
}

void WebViewLinux::shutdown() {
}

void WebViewLinux::set_bounds(const Rect2i &p_bounds) {
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
