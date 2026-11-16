#ifdef TOOLS_ENABLED

#include "vscode_dap_client.h"

#include "core/io/json.h"

void VSCodeDAPClient::attach(WebViewBackend *p_backend) {
	backend = p_backend;
}

void VSCodeDAPClient::_push_breakpoints_to_js(const String &p_path) {
	if (!backend || !backend->is_ready()) {
		return;
	}
	Array lines;
	if (breakpoints.has(p_path)) {
		for (int line : breakpoints[p_path]) {
			lines.push_back(line);
		}
	}
	Dictionary params;
	params["path"] = p_path;
	params["lines"] = lines;

	Dictionary msg;
	msg["method"] = "dap_breakpoints_updated";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

void VSCodeDAPClient::toggle_breakpoint(const String &p_path, int p_line) {
	Vector<int> &list = breakpoints[p_path];
	int idx = list.find(p_line);
	if (idx >= 0) {
		list.remove_at(idx);
	} else {
		list.push_back(p_line);
	}
	_push_breakpoints_to_js(p_path);
}

Vector<int> VSCodeDAPClient::get_breakpoints(const String &p_path) const {
	if (breakpoints.has(p_path)) {
		return breakpoints[p_path];
	}
	return Vector<int>();
}

void VSCodeDAPClient::notify_stopped(const String &p_path, int p_line, const Array &p_call_stack) {
	paused = true;
	if (!backend || !backend->is_ready()) {
		return;
	}
	Dictionary params;
	params["path"] = p_path;
	params["line"] = p_line;
	params["stack"] = p_call_stack;

	Dictionary msg;
	msg["method"] = "dap_stopped";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

void VSCodeDAPClient::continue_execution() {
	paused = false;
}

void VSCodeDAPClient::step_over() {
	// Forwarded to active Godot DAP session.
}

void VSCodeDAPClient::step_into() {
	// Forwarded to active Godot DAP session.
}

void VSCodeDAPClient::step_out() {
	// Forwarded to active Godot DAP session.
}

#endif // TOOLS_ENABLED
