#ifndef PMDOT_WEBVIEW_BACKEND_H
#define PMDOT_WEBVIEW_BACKEND_H

#include "core/string/ustring.h"
#include "core/math/rect2i.h"
#include <functional>

class WebViewBackend {
public:
	using MessageCallback = std::function<void(const String &)>;

	virtual ~WebViewBackend() = default;

	virtual bool initialize(int64_t p_native_window_handle, const Rect2i &p_bounds) = 0;
	virtual void shutdown() = 0;
	virtual void set_bounds(const Rect2i &p_bounds) = 0;
	virtual void set_visible(bool p_visible) = 0;
	virtual void navigate(const String &p_url) = 0;
	virtual void execute_script(const String &p_js) = 0;
	virtual void post_message(const String &p_json) = 0;
	virtual void set_message_callback(MessageCallback p_callback) = 0;
	virtual bool is_ready() const = 0;

	static WebViewBackend *create();
};

#endif // PMDOT_WEBVIEW_BACKEND_H
