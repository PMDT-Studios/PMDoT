#ifdef TOOLS_ENABLED

#include "vscode_test_runner.h"

#include "core/io/dir_access.h"
#include "core/io/json.h"
#include "core/os/os.h"
#include "core/templates/list.h"

void VSCodeTestRunner::attach(WebViewBackend *p_backend) {
	backend = p_backend;
}

void VSCodeTestRunner::_discover_dir_tests(const String &p_dir, Array &r_tests) {
	Ref<DirAccess> da = DirAccess::open(p_dir);
	if (da.is_null()) {
		return;
	}
	da->list_dir_begin();
	String name = da->get_next();
	while (!name.is_empty()) {
		if (name.begins_with(".") || name == "bin" || name == "obj") {
			name = da->get_next();
			continue;
		}
		String full = p_dir.ends_with("/") ? (p_dir + name) : (p_dir + "/" + name);
		if (da->current_is_dir()) {
			_discover_dir_tests(full, r_tests);
		} else {
			String lower = name.to_lower();
			if (lower.contains("test") && (lower.ends_with(".cs") || lower.ends_with(".cpp") || lower.ends_with(".tscn"))) {
				Dictionary t;
				t["id"] = full;
				t["name"] = name;
				t["type"] = lower.ends_with(".tscn") ? "integration" : "unit";
				t["status"] = "idle";
				r_tests.push_back(t);
			}
		}
		name = da->get_next();
	}
	da->list_dir_end();
}

void VSCodeTestRunner::discover_tests() {
	if (!backend || !backend->is_ready()) {
		return;
	}
	Array tests;
	_discover_dir_tests("res://", tests);

	Dictionary engine_unit;
	engine_unit["id"] = "engine://doctest";
	engine_unit["name"] = "PMDoT C++ Core & Module Unit Tests";
	engine_unit["type"] = "unit";
	engine_unit["status"] = "idle";
	tests.push_back(engine_unit);

	Dictionary params;
	params["tests"] = tests;

	Dictionary msg;
	msg["method"] = "tests_discovered";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

void VSCodeTestRunner::run_tests(const String &p_suite_filter) {
	if (!backend || !backend->is_ready() || running) {
		return;
	}
	running = true;

	String output;
	int exit_code = 0;

	if (p_suite_filter == "unit" || p_suite_filter == "all" || p_suite_filter == "engine://doctest") {
		List<String> args;
		args.push_back("--test");
		args.push_back("--headless");
		OS::get_singleton()->execute(OS::get_singleton()->get_executable_path(), args, &output, &exit_code, true);
	}

	running = false;

	Dictionary params;
	params["filter"] = p_suite_filter;
	params["passed"] = (exit_code == 0);
	params["output"] = output.is_empty() ? String("Test run finished with exit code " + itos(exit_code)) : output;

	Dictionary msg;
	msg["method"] = "tests_finished";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

#endif // TOOLS_ENABLED
