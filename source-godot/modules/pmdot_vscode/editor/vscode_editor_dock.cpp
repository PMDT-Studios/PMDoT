#ifdef TOOLS_ENABLED

#include "vscode_editor_dock.h"

#include "core/io/dir_access.h"
#include "core/object/script_language.h"
#include "editor/editor_node.h"
#include "scene/main/window.h"
#include "servers/display_server.h"

void VSCodeEditorDock::_bind_methods() {
}

void VSCodeEditorDock::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			prewarm();
		} break;
		case NOTIFICATION_RESIZED:
		case NOTIFICATION_MOVED_IN_PARENT: {
			_sync_bounds();
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (backend) {
				backend->set_visible(is_visible_in_tree());
				_sync_bounds();
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (backend) {
				backend->shutdown();
			}
		} break;
	}
}

void VSCodeEditorDock::_ensure_backend_initialized() {
	if (backend || !is_inside_tree()) {
		return;
	}

	Window *win = get_window();
	if (!win) {
		return;
	}

	int64_t native_handle = DisplayServer::get_singleton()->window_get_native_handle(
			DisplayServer::WINDOW_HANDLE,
			win->get_window_id());

	if (native_handle == 0) {
		return;
	}

	backend = WebViewBackend::create();
	Rect2 rect = webview_container->get_global_rect();
	Rect2i bounds(rect.position.x, rect.position.y, MAX(1, rect.size.x), MAX(1, rect.size.y));
	if (backend->initialize(native_handle, bounds)) {
		backend->set_visible(is_visible_in_tree());
		bridge.attach(backend);
		lsp_manager.attach(backend);
		dap_client.attach(backend);
		test_runner.attach(backend);
		diagnostic_hub.attach(backend);
		perf_profiler.attach(backend);
		bridge.set_custom_handler([this](const String &p_method, const Dictionary &p_params) {
			if (p_method == "dap_toggle_breakpoint") {
				dap_client.toggle_breakpoint(p_params.get("path", ""), int(p_params.get("line", 1)));
			} else if (p_method == "dap_command") {
				String cmd = p_params.get("command", "");
				if (cmd == "continue") dap_client.continue_execution();
				else if (cmd == "step_over") dap_client.step_over();
				else if (cmd == "step_into") dap_client.step_into();
				else if (cmd == "step_out") dap_client.step_out();
			} else if (p_method == "test_discover_req") {
				test_runner.discover_tests();
			} else if (p_method == "test_run_req") {
				test_runner.run_tests(p_params.get("filter", "all"));
			}
		});
		bridge.set_dirty_callback([this](const String &p_path, bool p_dirty) {
			if (status_label) {
				String marker = p_dirty ? " *" : "";
				status_label->set_text("PMDoT VS Code IDE | " + p_path + marker);
			}
		});
		prewarmed = true;
	}
}

void VSCodeEditorDock::_sync_bounds() {
	if (!backend || !backend->is_ready() || !webview_container) {
		return;
	}
	Rect2 rect = webview_container->get_global_rect();
	Rect2i bounds(rect.position.x, rect.position.y, MAX(1, rect.size.x), MAX(1, rect.size.y));
	backend->set_bounds(bounds);
}

void VSCodeEditorDock::prewarm() {
	if (prewarmed) {
		return;
	}
	_ensure_backend_initialized();
}

void VSCodeEditorDock::set_ide_visible(bool p_visible) {
	set_visible(p_visible);
	if (p_visible) {
		_ensure_backend_initialized();
		_sync_bounds();
	}
	if (backend) {
		backend->set_visible(p_visible);
	}
}

bool VSCodeEditorDock::open_script(const String &p_res_path) {
	_ensure_backend_initialized();
	if (status_label) {
		status_label->set_text("PMDoT VS Code IDE | " + p_res_path);
	}
	return bridge.open_file(p_res_path);
}

VSCodeEditorDock::VSCodeEditorDock() {
	set_v_size_flags(SIZE_EXPAND_FILL);
	set_h_size_flags(SIZE_EXPAND_FILL);

	webview_container = memnew(Control);
	webview_container->set_v_size_flags(SIZE_EXPAND_FILL);
	webview_container->set_h_size_flags(SIZE_EXPAND_FILL);
	add_child(webview_container);

	status_label = memnew(Label);
	status_label->set_text("PMDoT VS Code IDE (Monaco) | Ready");
	add_child(status_label);
}

VSCodeEditorDock::~VSCodeEditorDock() {
	if (backend) {
		backend->shutdown();
		memdelete(backend);
		backend = nullptr;
	}
}

PMDoTVSCodeEditorPlugin::PMDoTVSCodeEditorPlugin() {
	VSCodeLSPManager::register_editor_settings();
	dock = memnew(VSCodeEditorDock);
	dock->hide();
	EditorNode::get_singleton()->get_editor_main_screen()->get_control()->add_child(dock);
}

PMDoTVSCodeEditorPlugin::~PMDoTVSCodeEditorPlugin() {
}

bool PMDoTVSCodeEditorPlugin::handles(Object *p_object) const {
	return Object::cast_to<Script>(p_object) != nullptr;
}

void PMDoTVSCodeEditorPlugin::edit(Object *p_object) {
	Script *scr = Object::cast_to<Script>(p_object);
	if (scr && dock && !scr->get_path().is_empty()) {
		dock->open_script(scr->get_path());
	}
}

void PMDoTVSCodeEditorPlugin::make_visible(bool p_visible) {
	if (dock) {
		dock->set_ide_visible(p_visible);
	}
}

#endif // TOOLS_ENABLED
