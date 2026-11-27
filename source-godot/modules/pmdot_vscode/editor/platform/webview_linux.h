#ifndef PMDOT_WEBVIEW_LINUX_H
#define PMDOT_WEBVIEW_LINUX_H

#ifdef LINUXBSD_ENABLED

#include "webview_backend.h"
#include <cef_app.h>
#include <cef_browser.h>
#include <cef_client.h>
#include <cef_render_handler.h>
#include <cef_life_span_handler.h>

class MyCefClient : public CefClient, public CefRenderHandler, public CefLifeSpanHandler {
private:
    CefRefPtr<CefBrowser> browser_;
    CefRect view_rect_;
    //We need browser count to count how many browser we have when close godot
    //In CEF doc in life cicle we have to run CefRunMessageloop
    //But we dont use that, so when we close godot
    //We need to close the browsers that are still open to not crash
    //This we do with while in main();
    static int active_browser = 0;

public:
    CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;

    void OnPaint(
        CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList &dirtyRects,
        const void *buffer,
        int width,
        int height
    ) override;

    bool is_browser_saved() const;

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    CefRefPtr<CefBrowser> get_browser() const;

    void set_view_rect(const CefRect &rect);

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
