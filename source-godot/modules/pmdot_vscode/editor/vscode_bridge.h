#ifndef PMDOT_VSCODE_BRIDGE_H
#define PMDOT_VSCODE_BRIDGE_H

#ifdef TOOLS_ENABLED

#include "core/object/object.h"
#include "core/io/json.h"
#include "core/variant/dictionary.h"
#include "platform/webview_backend.h"

#include <functional>

class VSCodeBridge {
public:
	using DirtyChangedCallback = std::function<void(const String &, bool)>;

private:
	WebViewBackend *backend = nullptr;
	String active_file_path;
	bool dirty = false;
	DirtyChangedCallback dirty_callback;

	void _send_payload(const String &p_method, const Dictionary &p_params);

public:
	void attach(WebViewBackend *p_backend);
	void set_dirty_callback(DirtyChangedCallback p_callback);

	bool open_file(const String &p_res_path);
	bool save_file(const String &p_res_path, const String &p_content);
	void handle_js_message(const String &p_json_payload);

	static String detect_language_id(const String &p_path);
	String get_active_file_path() const { return active_file_path; }
	bool is_dirty() const { return dirty; }
};

#endif // TOOLS_ENABLED

#endif // PMDOT_VSCODE_BRIDGE_H
