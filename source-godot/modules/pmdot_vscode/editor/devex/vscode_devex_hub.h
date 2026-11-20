#ifndef PMDOT_VSCODE_DEVEX_HUB_H
#define PMDOT_VSCODE_DEVEX_HUB_H

#ifdef TOOLS_ENABLED

#include "../platform/webview_backend.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"

class VSCodeDevExHub {
private:
	WebViewBackend *backend = nullptr;
	Array log_buffer;
	int max_logs = 200;

	static void _scan_scenes_recursive(const String &p_dir, Array &r_scenes);
	static void _scan_todos_recursive(const String &p_dir, Array &r_todos);

public:
	void attach(WebViewBackend *p_backend);
	void push_log(const String &p_level, const String &p_text);
	void clear_logs();
	void open_external_url(const String &p_url);
	void scan_scene_bindings();
	void scan_todos();
};

#endif // TOOLS_ENABLED

#endif // PMDOT_VSCODE_DEVEX_HUB_H
