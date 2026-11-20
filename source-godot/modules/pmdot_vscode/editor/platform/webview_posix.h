#ifndef PMDOT_WEBVIEW_POSIX_H
#define PMDOT_WEBVIEW_POSIX_H

#ifdef LINUX_ENABLED

#include "webview_backend.h"
#include "core/math/rect2i.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

class WebViewLinux : public WebViewBackend {
private:

	unsigned long host_xid = 0;
	unsigned long parent_xid = 0;

	void *xdisplay = nullptr;

	GdkWindow *host_gdk = nullptr;
	GtkWidget *webview = nullptr;

	GtkWidget *plug = nullptr;

	bool ready = false;
	bool visible = false;
	Rect2i current_bounds;
	String pending_url;

	Vector<String> queued_scripts;
	Vector<String> queued_messages;

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

#endif // lINUXBSD_ENABLED

#endif
