#ifdef WINDOWS_ENABLED

#include "webview_windows.h"

#include "core/io/dir_access.h"
#include "core/os/os.h"
#include "core/templates/vector.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WebView2.h>
#include <atomic>

static const wchar_t *PMDOT_WEBVIEW_CLASS = L"PMDoTWebViewHostClass";
static bool class_registered = false;

namespace {

template <typename TInterface, typename TFunc>
class ComCallback : public TInterface {
	std::atomic<ULONG> ref_count{ 1 };
	TFunc callback;

public:
	explicit ComCallback(TFunc p_cb) : callback(p_cb) {}
	virtual ~ComCallback() = default;

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override {
		if (!ppvObject) {
			return E_POINTER;
		}
		if (riid == IID_IUnknown || riid == __uuidof(TInterface)) {
			*ppvObject = static_cast<TInterface *>(this);
			AddRef();
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef() override {
		return ++ref_count;
	}

	ULONG STDMETHODCALLTYPE Release() override {
		ULONG count = --ref_count;
		if (count == 0) {
			delete this;
		}
		return count;
	}

	template <typename... Args>
	HRESULT STDMETHODCALLTYPE Invoke(Args... args) override {
		return callback(args...);
	}
};

struct WebView2State {
	ICoreWebView2Controller *controller = nullptr;
	ICoreWebView2 *webview = nullptr;
	bool navigation_completed = false;
	Vector<String> queued_messages;
	Vector<String> queued_scripts;
};

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

} // namespace

static LRESULT CALLBACK _pmdot_host_wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam) {
	if (p_msg == WM_ERASEBKGND) {
		HDC hdc = (HDC)p_wparam;
		RECT rc;
		GetClientRect(p_hwnd, &rc);
		HBRUSH brush = CreateSolidBrush(RGB(30, 30, 30));
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
	HWND parent = reinterpret_cast<HWND>(p_native_window_handle);
	if (!parent || !IsWindow(parent)) {
		return false;
	}
	parent_hwnd = parent;

	HINSTANCE hinst = GetModuleHandleW(nullptr);
	if (!class_registered) {
		WNDCLASSEXW wc = {};
		wc.cbSize = sizeof(WNDCLASSEXW);
		wc.lpfnWndProc = _pmdot_host_wnd_proc;
		wc.hInstance = hinst;
		wc.lpszClassName = PMDOT_WEBVIEW_CLASS;
		RegisterClassExW(&wc);
		class_registered = true;
	}

	current_bounds = p_bounds;
	HWND host = CreateWindowExW(
			0,
			PMDOT_WEBVIEW_CLASS,
			L"PMDoTMonacoHost",
			WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			p_bounds.position.x,
			p_bounds.position.y,
			MAX(1, p_bounds.size.x),
			MAX(1, p_bounds.size.y),
			parent,
			nullptr,
			hinst,
			nullptr);

	if (!host) {
		return false;
	}
	host_hwnd = host;

	WebView2State *state = new WebView2State();
	controller_ptr = state;

	String user_data_dir = OS::get_singleton()->get_user_data_dir().path_join("webview2_cache");
	Char16String user_data_w = user_data_dir.utf16();

	auto *env_handler = new ComCallback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
			std::function<HRESULT(HRESULT, ICoreWebView2Environment *)>>(
			[this, host, state](HRESULT result, ICoreWebView2Environment *env) -> HRESULT {
				if (FAILED(result) || !env || !host_hwnd) {
					return S_OK;
				}
				auto *ctrl_handler = new ComCallback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
						std::function<HRESULT(HRESULT, ICoreWebView2Controller *)>>(
						[this, host, state](HRESULT ctrl_res, ICoreWebView2Controller *controller) -> HRESULT {
							if (FAILED(ctrl_res) || !controller || !host_hwnd) {
								return S_OK;
							}
							controller->AddRef();
							state->controller = controller;

							ICoreWebView2 *webview = nullptr;
							if (SUCCEEDED(controller->get_CoreWebView2(&webview)) && webview) {
								state->webview = webview;
								webview_ptr = webview;

								RECT rc;
								GetClientRect(host, &rc);
								controller->put_Bounds(rc);
								controller->put_IsVisible(visible ? TRUE : FALSE);

								EventRegistrationToken msg_token;
								auto *msg_handler = new ComCallback<ICoreWebView2WebMessageReceivedEventHandler,
										std::function<HRESULT(ICoreWebView2 *, ICoreWebView2WebMessageReceivedEventArgs *)>>(
										[this](ICoreWebView2 *, ICoreWebView2WebMessageReceivedEventArgs *args) -> HRESULT {
											LPWSTR json_w = nullptr;
											if (args && SUCCEEDED(args->get_WebMessageAsJson(&json_w)) && json_w) {
												String msg = String::utf16(reinterpret_cast<const char16_t *>(json_w));
												CoTaskMemFree(json_w);
												// Unwrap JSON string if JS called postMessage(JSON.stringify(...))
												if (msg.begins_with("\"") && msg.ends_with("\"")) {
													msg = msg.substr(1, msg.length() - 2).c_unescape();
												}
												if (message_callback) {
													message_callback(msg);
												}
											}
											return S_OK;
										});
								webview->add_WebMessageReceived(msg_handler, &msg_token);
								msg_handler->Release();

								EventRegistrationToken nav_token;
								auto *nav_handler = new ComCallback<ICoreWebView2NavigationCompletedEventHandler,
										std::function<HRESULT(ICoreWebView2 *, ICoreWebView2NavigationCompletedEventArgs *)>>(
										[this, state](ICoreWebView2 *wv, ICoreWebView2NavigationCompletedEventArgs *) -> HRESULT {
											state->navigation_completed = true;
											ready = true;
											for (const String &js : state->queued_scripts) {
												Char16String wjs = js.utf16();
												wv->ExecuteScript(reinterpret_cast<LPCWSTR>(wjs.get_data()), nullptr);
											}
											state->queued_scripts.clear();
											for (const String &m : state->queued_messages) {
												Char16String wm = m.utf16();
												wv->PostWebMessageAsJson(reinterpret_cast<LPCWSTR>(wm.get_data()));
											}
											state->queued_messages.clear();
											return S_OK;
										});
								webview->add_NavigationCompleted(nav_handler, &nav_token);
								nav_handler->Release();

								ICoreWebView2_3 *webview3 = nullptr;
								String monaco_dir = _resolve_monaco_directory().replace("/", "\\");
								if (SUCCEEDED(webview->QueryInterface(__uuidof(ICoreWebView2_3), reinterpret_cast<void **>(&webview3))) && webview3) {
									Char16String dir_w = monaco_dir.utf16();
									webview3->SetVirtualHostNameToFolderMapping(
											L"pmdot.ide",
											reinterpret_cast<LPCWSTR>(dir_w.get_data()),
											COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);
									webview3->Release();
									webview->Navigate(L"https://pmdot.ide/index.html");
								} else {
									String fallback_url = "file:///" + monaco_dir.replace("\\", "/") + "/index.html";
									Char16String url_w = fallback_url.utf16();
									webview->Navigate(reinterpret_cast<LPCWSTR>(url_w.get_data()));
								}
							}
							return S_OK;
						});
				env->CreateCoreWebView2Controller(host, ctrl_handler);
				ctrl_handler->Release();
				return S_OK;
			});

	HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
			nullptr,
			reinterpret_cast<LPCWSTR>(user_data_w.get_data()),
			nullptr,
			env_handler);
	env_handler->Release();

	ready = SUCCEEDED(hr);
	return ready;
}

void WebViewWindows::shutdown() {
	ready = false;
	if (controller_ptr) {
		WebView2State *state = static_cast<WebView2State *>(controller_ptr);
		if (state->webview) {
			state->webview->Release();
			state->webview = nullptr;
		}
		if (state->controller) {
			state->controller->Close();
			state->controller->Release();
			state->controller = nullptr;
		}
		delete state;
		controller_ptr = nullptr;
		webview_ptr = nullptr;
	}
	if (host_hwnd) {
		DestroyWindow(static_cast<HWND>(host_hwnd));
		host_hwnd = nullptr;
	}
}

void WebViewWindows::set_bounds(const Rect2i &p_bounds) {
	current_bounds = p_bounds;
	if (host_hwnd) {
		SetWindowPos(
				static_cast<HWND>(host_hwnd),
				nullptr,
				p_bounds.position.x,
				p_bounds.position.y,
				MAX(1, p_bounds.size.x),
				MAX(1, p_bounds.size.y),
				SWP_NOZORDER | SWP_NOACTIVATE);
		if (controller_ptr) {
			WebView2State *state = static_cast<WebView2State *>(controller_ptr);
			if (state->controller) {
				RECT rc = { 0, 0, MAX(1, p_bounds.size.x), MAX(1, p_bounds.size.y) };
				state->controller->put_Bounds(rc);
			}
		}
	}
}

void WebViewWindows::set_visible(bool p_visible) {
	visible = p_visible;
	if (host_hwnd) {
		ShowWindow(static_cast<HWND>(host_hwnd), p_visible ? SW_SHOW : SW_HIDE);
	}
	if (controller_ptr) {
		WebView2State *state = static_cast<WebView2State *>(controller_ptr);
		if (state->controller) {
			state->controller->put_IsVisible(p_visible ? TRUE : FALSE);
		}
	}
}

void WebViewWindows::navigate(const String &p_url) {
	pending_url = p_url;
	if (controller_ptr) {
		WebView2State *state = static_cast<WebView2State *>(controller_ptr);
		if (state->webview) {
			Char16String wurl = p_url.utf16();
			state->webview->Navigate(reinterpret_cast<LPCWSTR>(wurl.get_data()));
		}
	}
}

void WebViewWindows::execute_script(const String &p_js) {
	if (!controller_ptr) {
		return;
	}
	WebView2State *state = static_cast<WebView2State *>(controller_ptr);
	if (state->webview && state->navigation_completed) {
		Char16String wjs = p_js.utf16();
		state->webview->ExecuteScript(reinterpret_cast<LPCWSTR>(wjs.get_data()), nullptr);
	} else {
		state->queued_scripts.push_back(p_js);
	}
}

void WebViewWindows::post_message(const String &p_json) {
	if (!controller_ptr) {
		return;
	}
	WebView2State *state = static_cast<WebView2State *>(controller_ptr);
	if (state->webview && state->navigation_completed) {
		Char16String wjson = p_json.utf16();
		state->webview->PostWebMessageAsJson(reinterpret_cast<LPCWSTR>(wjson.get_data()));
	} else {
		state->queued_messages.push_back(p_json);
	}
}

void WebViewWindows::set_message_callback(MessageCallback p_callback) {
	message_callback = p_callback;
}

WebViewBackend *WebViewBackend::create() {
	return memnew(WebViewWindows);
}

#endif // WINDOWS_ENABLED
