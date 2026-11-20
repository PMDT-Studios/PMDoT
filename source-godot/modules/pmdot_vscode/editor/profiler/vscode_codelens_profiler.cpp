#ifdef TOOLS_ENABLED

#include "vscode_codelens_profiler.h"

#include "core/io/json.h"
#include "core/os/os.h"
#include "main/performance.h"

void VSCodeCodeLensProfiler::attach(WebViewBackend *p_backend) {
	backend = p_backend;
}

void VSCodeCodeLensProfiler::poll_and_push(const String &p_active_path, bool p_ide_visible) {
	if (!p_ide_visible || !backend || !backend->is_ready() || p_active_path.is_empty()) {
		return;
	}

	uint64_t now = OS::get_singleton()->get_ticks_msec();
	if (now - last_poll_msec < poll_interval_msec) {
		return;
	}
	last_poll_msec = now;

	Performance *perf = Performance::get_singleton();
	if (!perf) {
		return;
	}

	Vector<MethodMetric> metrics;
	MethodMetric proc;
	proc.method_name = "_Process";
	proc.avg_ms = perf->get_monitor(Performance::TIME_PROCESS) * 1000.0;
	proc.calls_per_sec = int(perf->get_monitor(Performance::TIME_FPS));
	metrics.push_back(proc);

	MethodMetric phys;
	phys.method_name = "_PhysicsProcess";
	phys.avg_ms = perf->get_monitor(Performance::TIME_PHYSICS_PROCESS) * 1000.0;
	phys.calls_per_sec = int(perf->get_monitor(Performance::TIME_FPS));
	metrics.push_back(phys);

	push_metrics(p_active_path, metrics);
}

void VSCodeCodeLensProfiler::push_metrics(const String &p_path, const Vector<MethodMetric> &p_metrics) {
	if (!backend || !backend->is_ready()) {
		return;
	}

	Array items;
	for (const MethodMetric &m : p_metrics) {
		Dictionary d;
		d["method"] = m.method_name;
		d["avgMs"] = m.avg_ms;
		d["callsPerSec"] = m.calls_per_sec;
		items.push_back(d);
	}

	Dictionary params;
	params["path"] = p_path;
	params["metrics"] = items;

	Dictionary msg;
	msg["method"] = "perf_codelens_update";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

#endif // TOOLS_ENABLED
