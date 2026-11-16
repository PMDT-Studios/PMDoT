#ifdef TOOLS_ENABLED

#include "vscode_diagnostic_hub.h"

#include "core/io/json.h"

void VSCodeDiagnosticHub::attach(WebViewBackend *p_backend) {
	backend = p_backend;
}

void VSCodeDiagnosticHub::clear_diagnostics(const String &p_path) {
	file_diagnostics.erase(p_path);
	flush(p_path);
}

void VSCodeDiagnosticHub::add_diagnostic(const String &p_path, int p_line, int p_column, Severity p_severity, const String &p_message) {
	DiagnosticEntry entry;
	entry.line = MAX(1, p_line);
	entry.column = MAX(1, p_column);
	entry.severity = p_severity;
	entry.message = p_message;
	file_diagnostics[p_path].push_back(entry);
}

void VSCodeDiagnosticHub::flush(const String &p_path) {
	if (!backend || !backend->is_ready()) {
		return;
	}
	Array items;
	if (file_diagnostics.has(p_path)) {
		for (const DiagnosticEntry &e : file_diagnostics[p_path]) {
			Dictionary d;
			d["line"] = e.line;
			d["column"] = e.column;
			d["severity"] = int(e.severity);
			d["message"] = e.message;
			items.push_back(d);
		}
	}

	Dictionary params;
	params["path"] = p_path;
	params["diagnostics"] = items;

	Dictionary msg;
	msg["method"] = "diagnostics_update";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

Vector<VSCodeDiagnosticHub::DiagnosticEntry> VSCodeDiagnosticHub::get_diagnostics(const String &p_path) const {
	if (file_diagnostics.has(p_path)) {
		return file_diagnostics[p_path];
	}
	return Vector<DiagnosticEntry>();
}

#endif // TOOLS_ENABLED
