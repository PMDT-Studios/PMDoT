#ifdef LINUXBSD_ENABLED

#include <cef_app.h>
#include <cef_browser.h>
#include <cef_client.h>
#include <cef_render_handler.h>
#include <cef_life_span_handler.h>

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

bool WebViewLinux::initialize(int64_t p_native_window_handle, const Rect2i &p_bounds){
    set_position(Vector2(p_bounds.position));
    set_size(Vector2(p_bounds.size));
	
	current_bounds.Set(
		p_bounds.position.x,
		p_bounds.position.y,
		p_bounds.size.x,
		p_bounds.size.y
	);

	if(!CefBrowserHost::CreateBrowser(window_info, client, "https://www.google.com", browser_settings, nullptr, nullptr)){
		//Google to test, after we'll put out vscode .html
		return false;
	}


	return true;
}

void WebViewLinux::_notification(int p_what){
	switch (p_what)
	{
	case NOTIFICATION_PROCESS:
		CefDoMessageLoopWork();
		if(!ready && client->is_browser_saved()){
			browser = client->get_browser();
			ready = true;
		}
		break;
	
	default:
		break;
	}
}


void MyCefClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    if (browser_ && browser_->IsSame(browser)) {
        browser_ = nullptr;
    }
}

#endif // LINUXBSD_ENABLED
