#ifndef PMDOT_WEBVIEW_LINUX_H
#define PMDOT_WEBVIEW_LINUX_H

#ifdef LINUXBSD_ENABLED

#include "webview_backend.h"

class MyCefClient : public CefClient, public CefRenderHandler, public CefLifeSpanHandler {
private:
    CefRefPtr<CefBrowser> browser_;

public:
    CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return this;
    }

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override {
        browser_ = browser;
    }

    bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;

    void OnPaint(
        CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList &dirtyRects,
        const void *buffer,
        int width,
        int height
    ) override;

    bool is_browser_saved() const {
        return browser_ != nullptr;
    }

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    CefRefPtr<CefBrowser> get_browser() const {
        return browser_;
    }

    IMPLEMENT_REFCOUNTING(MyCefClient);
};

class WebViewLinux : public WebViewBackend, public TextureRect {
private:
    CefSettings settings;
    CefBrowserSettings browser_settings;

    CefRect current_bounds;

    CefRefPtr<MyCefClient> client = new MyCefClient();
    CefRefPtr<CefBrowser> browser;

    CefWindowInfo window_info; //OSR

	bool ready = false;
	bool visible = false;

	String pending_url;
	MessageCallback message_callback;

protected:
    void _notification(int p_what);

public:
	virtual ~WebViewLinux();

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

#endif // PMDOT_WEBVIEW_LINUX_H
