#ifndef PMDOT_VSCODE_DAP_CLIENT_H
#define PMDOT_VSCODE_DAP_CLIENT_H

#ifdef TOOLS_ENABLED

#include "core/templates/hash_map.h"
#include "core/templates/vector.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "platform/webview_backend.h"

class VSCodeDAPClient {
private:
	WebViewBackend *backend = nullptr;
	HashMap<String, Vector<int>> breakpoints;
	bool paused = false;

	void _push_breakpoints_to_js(const String &p_path);

public:
	void attach(WebViewBackend *p_backend);
	void toggle_breakpoint(const String &p_path, int p_line);
	Vector<int> get_breakpoints(const String &p_path) const;

	void notify_stopped(const String &p_path, int p_line, const Array &p_call_stack);
	void continue_execution();
	void step_over();
	void step_into();
	void step_out();
	bool is_paused() const { return paused; }
};

#endif // TOOLS_ENABLED

#endif // PMDOT_VSCODE_DAP_CLIENT_H
