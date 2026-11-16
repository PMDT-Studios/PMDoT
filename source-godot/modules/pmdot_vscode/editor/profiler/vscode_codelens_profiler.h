#ifndef PMDOT_VSCODE_CODELENS_PROFILER_H
#define PMDOT_VSCODE_CODELENS_PROFILER_H

#ifdef TOOLS_ENABLED

#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include "platform/webview_backend.h"

class VSCodeCodeLensProfiler {
public:
	struct MethodMetric {
		String method_name;
		double avg_ms = 0.0;
		int calls_per_sec = 0;
	};

private:
	WebViewBackend *backend = nullptr;
	uint64_t last_poll_msec = 0;
	uint64_t poll_interval_msec = 1000;

public:
	void attach(WebViewBackend *p_backend);
	void poll_and_push(const String &p_active_path, bool p_ide_visible);
	void push_metrics(const String &p_path, const Vector<MethodMetric> &p_metrics);
};

#endif // TOOLS_ENABLED

#endif // PMDOT_VSCODE_CODELENS_PROFILER_H
