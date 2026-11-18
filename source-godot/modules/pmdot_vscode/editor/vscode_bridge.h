#ifndef PMDOT_VSCODE_BRIDGE_H
#define PMDOT_VSCODE_BRIDGE_H

#ifdef TOOLS_ENABLED

#include "core/io/json.h"
#include "core/object/object.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "platform/webview_backend.h"

#include <functional>

class VSCodeBridge {
public:
	using DirtyChangedCallback = std::function<void(const String &, bool)>;
	using ReadyCallback = std::function<void()>;
	using CustomMessageHandler = std::function<void(const String &, const Dictionary &)>;

private:
	WebViewBackend *backend = nullptr;
	String active_file_path;
	bool dirty = false;
	DirtyChangedCallback dirty_callback;
	ReadyCallback ready_callback;
	CustomMessageHandler custom_handler;

	void _send_payload(const String &p_method, const Dictionary &p_params);
	void _scan_dir_recursive(const String &p_dir, Array &r_files) const;
	static bool _is_supported_code_file(const String &p_ext);
	static String _build_template(const String &p_res_path);

public:
	void attach(WebViewBackend *p_backend);
	void set_dirty_callback(DirtyChangedCallback p_callback);
	void set_ready_callback(ReadyCallback p_callback);
	void set_custom_handler(CustomMessageHandler p_handler);

	bool open_file(const String &p_res_path);
	bool save_file(const String &p_res_path, const String &p_content);
	bool create_file(const String &p_res_path);
	void refresh_project_tree();
	void search_in_files(const String &p_query);
	void handle_js_message(const String &p_json_payload);

	static String detect_language_id(const String &p_path);
	String get_active_file_path() const { return active_file_path; }
	bool is_dirty() const { return dirty; }
};

#endif // TOOLS_ENABLED

#endif // PMDOT_VSCODE_BRIDGE_H
