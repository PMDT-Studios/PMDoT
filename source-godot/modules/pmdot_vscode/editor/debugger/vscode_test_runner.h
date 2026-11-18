#ifndef PMDOT_VSCODE_TEST_RUNNER_H
#define PMDOT_VSCODE_TEST_RUNNER_H

#ifdef TOOLS_ENABLED

#include "../platform/webview_backend.h"
#include "core/templates/vector.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"

class VSCodeTestRunner {
private:
	WebViewBackend *backend = nullptr;
	bool running = false;

	static void _discover_dir_tests(const String &p_dir, Array &r_tests);

public:
	void attach(WebViewBackend *p_backend);
	void discover_tests();
	void run_tests(const String &p_suite_filter);
	bool is_running() const { return running; }
};

#endif // TOOLS_ENABLED

#endif // PMDOT_VSCODE_TEST_RUNNER_H
