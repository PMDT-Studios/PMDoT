#ifndef WINDOWS_ENABLED

#include "webview_backend.h"

class WebViewPosix : public WebViewBackend {
private:
	bool ready = false;
	bool visible = false;
	Rect2i current_bounds;
	String pending_url;
	MessageCallback message_callback;

public:
	virtual bool initialize(int64_t p_native_window_handle, const Rect2i &p_bounds) override {
		current_bounds = p_bounds;
		ready = (p_native_window_handle != 0);
		return ready;
	}

	virtual void shutdown() override {
		ready = false;
	}

	virtual void set_bounds(const Rect2i &p_bounds) override {
		current_bounds = p_bounds;
	}

	virtual void set_visible(bool p_visible) override {
		visible = p_visible;
	}

	virtual void navigate(const String &p_url) override {
		pending_url = p_url;
	}

	virtual void execute_script(const String &p_js) override {}

	virtual void post_message(const String &p_json) override {}

	virtual void set_message_callback(MessageCallback p_callback) override {
		message_callback = p_callback;
	}

	virtual bool is_ready() const override {
		return ready;
	}
};

WebViewBackend *WebViewBackend::create() {
	return memnew(WebViewPosix);
}

#endif // !WINDOWS_ENABLED
