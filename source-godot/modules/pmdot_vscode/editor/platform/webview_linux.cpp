#ifdef LINUXBSD_ENABLED

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
	window_info.SetAsWindowless(0);
    set_position(Vector2(p_bounds.position));
    set_size(Vector2(p_bounds.size));

	client->set_view_rect(
		CefRect(0, 0, p_bounds.size.x, p_bounds.size.y)
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



void WebViewLinux::set_bounds(const Rect2i &p_bounds) {
	set_position(Vector2(p_bounds.position));
    set_size(Vector2(p_bounds.size));
	client->set_view_rect(CefRect(0, 0, p_bounds.size.x, p_bounds.size.y));

	if(browser){
		browser->GetHost()->WasResized();
	}
}

void WebViewLinux::set_visible(bool p_visible) {
	visible = p_visible;
	//set texture rect visible or not
	TextureRect::set_visible(p_visible);
}

void WebViewLinux::navigate(const String &p_url){
	if(browser){
		browser->GetMainFrame()->LoadURL(p_url);
	}
}

void WebViewLinux::set_message_callback(MessageCallback p_callback) {
    message_callback = p_callback;
}

// WE DONT HAVE THIS NOW

void WebViewLinux::execute_script(const String &p_js){
	return;
}

void WebViewLinux::post_message(const String &p_json){
	return;
}
// ----------------------

WebViewBackend *WebViewBackend::create() {
	return memnew(WebViewLinux);
}



// -------- MyCefClient --------

void MyCefClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    if (browser_ && browser_->IsSame(browser)) {
		MyCefClient::active_browser--;
        browser_ = nullptr;
    }
}

void MyCefClient::set_view_rect(const CefRect &rect){
	view_rect_ = rect;
}

bool MyCefClient::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect){
	rect = view_rect_;
	return true;
}

bool MyCefClient::is_browser_saved() const {
    return browser_ != nullptr;
}

CefRefPtr<CefBrowser> MyCefClient::get_browser() const {
    return browser_;
}

void MyCefClient::OnAfterCreated(CefRefPtr<CefBrowser> browser){
	MyCefClient::active_browser++;
    browser_ = browser;
}

#endif // LINUXBSD_ENABLED
