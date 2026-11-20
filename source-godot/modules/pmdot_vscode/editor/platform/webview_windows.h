#ifndef PMDOT_WEBVIEW_WINDOWS_H
#define PMDOT_WEBVIEW_WINDOWS_H

#ifdef WINDOWS_ENABLED

#include "webview_backend.h"

class WebViewWindows : public WebViewBackend {
private:
	void *parent_hwnd = nullptr;
	void *host_hwnd = nullptr;
	void *webview2_loader = nullptr;
	void *controller_ptr = nullptr;
	void *webview_ptr = nullptr;
	bool ready = false;
	bool visible = false;
	Rect2i current_bounds;
	String pending_url;
	MessageCallback message_callback;

public:
	virtual ~WebViewWindows();

	virtual bool initialize(int64_t p_native_window_handle, const Rect2i &p_bounds) override;
	virtual void shutdown() override;
	virtual void set_bounds(const Rect2i &p_bounds) override;
	virtual void set_visible(bool p_visible) override;
	virtual void navigate(const String &p_url) override;
	virtual void execute_script(const String &p_js) override;
	virtual void post_message(const String &p_json) override;
	virtual void set_message_callback(MessageCallback p_callback) override;
	virtual bool is_ready() const override { return ready; }
};

#endif // WINDOWS_ENABLED

#endif // PMDOT_WEBVIEW_WINDOWS_H
