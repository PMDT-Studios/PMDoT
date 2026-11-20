#ifndef PMDOT_VSCODE_DIAGNOSTIC_HUB_H
#define PMDOT_VSCODE_DIAGNOSTIC_HUB_H

#ifdef TOOLS_ENABLED

#include "core/templates/hash_map.h"
#include "core/templates/vector.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "platform/webview_backend.h"

class VSCodeDiagnosticHub {
public:
	enum Severity {
		SEV_ERROR = 8,
		SEV_WARNING = 4,
		SEV_INFO = 2,
		SEV_HINT = 1
	};

	struct DiagnosticEntry {
		int line = 1;
		int column = 1;
		Severity severity = SEV_ERROR;
		String message;
	};

private:
	WebViewBackend *backend = nullptr;
	HashMap<String, Vector<DiagnosticEntry>> file_diagnostics;

public:
	void attach(WebViewBackend *p_backend);
	void clear_diagnostics(const String &p_path);
	void add_diagnostic(const String &p_path, int p_line, int p_column, Severity p_severity, const String &p_message);
	void flush(const String &p_path);
	Vector<DiagnosticEntry> get_diagnostics(const String &p_path) const;
};

#endif // TOOLS_ENABLED

#endif // PMDOT_VSCODE_DIAGNOSTIC_HUB_H
