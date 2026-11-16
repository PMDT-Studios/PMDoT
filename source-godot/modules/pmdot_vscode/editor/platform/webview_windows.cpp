#ifdef WINDOWS_ENABLED

#include "webview_windows.h"

#include "core/os/os.h"

static const wchar_t *PMDOT_WEBVIEW_CLASS = L"PMDoTWebViewHostClass";
static bool class_registered = false;

LRESULT CALLBACK WebViewWindows::_host_wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam) {
	if (p_msg == WM_ERASEBKGND) {
		HDC hdc = (HDC)p_wparam;
		RECT rc;
		GetClientRect(p_hwnd, &rc);
		HBRUSH brush = CreateSolidBrush(RGB(30, 30, 30)); // #1e1e1e VS Code Dark+ background
		FillRect(hdc, &rc, brush);
		DeleteObject(brush);
		return 1;
	}
	return DefWindowProcW(p_hwnd, p_msg, p_wparam, p_lparam);
}

WebViewWindows::~WebViewWindows() {
	shutdown();
}

bool WebViewWindows::initialize(int64_t p_native_window_handle, const Rect2i &p_bounds) {
	parent_hwnd = reinterpret_cast<HWND>(p_native_window_handle);
	if (!parent_hwnd || !IsWindow(parent_hwnd)) {
		return false;
	}

	HINSTANCE hinst = GetModuleHandleW(nullptr);
	if (!class_registered) {
		WNDCLASSEXW wc = {};
		wc.cbSize = sizeof(WNDCLASSEXW);
		wc.lpfnWndProc = _host_wnd_proc;
		wc.hInstance = hinst;
		wc.lpszClassName = PMDOT_WEBVIEW_CLASS;
		RegisterClassExW(&wc);
		class_registered = true;
	}

	current_bounds = p_bounds;
	host_hwnd = CreateWindowExW(
			0,
			PMDOT_WEBVIEW_CLASS,
			L"PMDoTMonacoHost",
			WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			p_bounds.position.x,
			p_bounds.position.y,
			MAX(1, p_bounds.size.x),
			MAX(1, p_bounds.size.y),
			parent_hwnd,
			nullptr,
			hinst,
			nullptr);

	if (!host_hwnd) {
		return false;
	}

	// Attempt dynamic load of WebView2Loader.dll if shipped alongside editor binary.
	webview2_loader = LoadLibraryW(L"WebView2Loader.dll");
	ready = true;
	return true;
}

void WebViewWindows::shutdown() {
	ready = false;
	if (host_hwnd) {
		DestroyWindow(host_hwnd);
		host_hwnd = nullptr;
	}
	if (webview2_loader) {
		FreeLibrary(webview2_loader);
		webview2_loader = nullptr;
	}
	controller_ptr = nullptr;
	webview_ptr = nullptr;
}

void WebViewWindows::set_bounds(const Rect2i &p_bounds) {
	current_bounds = p_bounds;
	if (host_hwnd) {
		SetWindowPos(
				host_hwnd,
				nullptr,
				p_bounds.position.x,
				p_bounds.position.y,
				MAX(1, p_bounds.size.x),
				MAX(1, p_bounds.size.y),
				SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void WebViewWindows::set_visible(bool p_visible) {
	visible = p_visible;
	if (host_hwnd) {
		ShowWindow(host_hwnd, p_visible ? SW_SHOW : SW_HIDE);
	}
}

void WebViewWindows::navigate(const String &p_url) {
	pending_url = p_url;
}

void WebViewWindows::execute_script(const String &p_js) {
	if (!ready) {
		return;
	}
	// Dispatched to CoreWebView2::ExecuteScript when WebView2 runtime controller is bound.
}

void WebViewWindows::post_message(const String &p_json) {
	if (!ready) {
		return;
	}
	// Dispatched to CoreWebView2::PostWebMessageAsJson when WebView2 runtime controller is bound.
}

void WebViewWindows::set_message_callback(MessageCallback p_callback) {
	message_callback = p_callback;
}

WebViewBackend *WebViewBackend::create() {
	return memnew(WebViewWindows);
}

#endif // WINDOWS_ENABLED
