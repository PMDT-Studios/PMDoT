#ifdef TOOLS_ENABLED

#include "vscode_bridge.h"

#include "core/io/file_access.h"
#include "editor/editor_file_system.h"

void VSCodeBridge::_send_payload(const String &p_method, const Dictionary &p_params) {
	if (!backend || !backend->is_ready()) {
		return;
	}
	Dictionary msg;
	msg["method"] = p_method;
	msg["params"] = p_params;
	backend->post_message(JSON::stringify(msg));
}

void VSCodeBridge::attach(WebViewBackend *p_backend) {
	backend = p_backend;
	if (backend) {
		backend->set_message_callback([this](const String &p_msg) {
			handle_js_message(p_msg);
		});
	}
}

void VSCodeBridge::set_dirty_callback(DirtyChangedCallback p_callback) {
	dirty_callback = p_callback;
}

String VSCodeBridge::detect_language_id(const String &p_path) {
	String ext = p_path.get_extension().to_lower();
	if (ext == "cs") {
		return "csharp";
	}
	if (ext == "cpp" || ext == "cc" || ext == "cxx" || ext == "h" || ext == "hpp" || ext == "c") {
		return "cpp";
	}
	if (ext == "java") {
		return "java";
	}
	if (ext == "md" || ext == "markdown") {
		return "markdown";
	}
	if (ext == "json") {
		return "json";
	}
	if (ext == "glsl" || ext == "gdshader") {
		return "glsl";
	}
	return "plaintext";
}

bool VSCodeBridge::open_file(const String &p_res_path) {
	Error err = OK;
	Ref<FileAccess> fa = FileAccess::open(p_res_path, FileAccess::READ, &err);
	if (err != OK || fa.is_null()) {
		return false;
	}

	String content = fa->get_as_utf8_string();
	active_file_path = p_res_path;
	dirty = false;

	Dictionary params;
	params["path"] = p_res_path;
	params["content"] = content;
	params["language"] = detect_language_id(p_res_path);
	_send_payload("open_script", params);
	return true;
}

bool VSCodeBridge::save_file(const String &p_res_path, const String &p_content) {
	Error err = OK;
	Ref<FileAccess> fa = FileAccess::open(p_res_path, FileAccess::WRITE, &err);
	if (err != OK || fa.is_null()) {
		return false;
	}
	fa->store_string(p_content);
	fa->flush();

	dirty = false;
	if (dirty_callback) {
		dirty_callback(p_res_path, false);
	}

	if (EditorFileSystem::get_singleton()) {
		EditorFileSystem::get_singleton()->update_file(p_res_path);
	}
	return true;
}

void VSCodeBridge::handle_js_message(const String &p_json_payload) {
	Ref<JSON> json;
	json.instantiate();
	if (json->parse(p_json_payload) != OK) {
		return;
	}

	Dictionary msg = json->get_data();
	String method = msg.get("method", "");
	Dictionary params = msg.get("params", Dictionary());

	if (method == "save_script") {
		String path = params.get("path", active_file_path);
		String content = params.get("content", "");
		if (!path.is_empty()) {
			save_file(path, content);
		}
	} else if (method == "content_changed") {
		String path = params.get("path", active_file_path);
		bool is_dirty = params.get("dirty", true);
		dirty = is_dirty;
		if (dirty_callback) {
			dirty_callback(path, is_dirty);
		}
	}
}

#endif // TOOLS_ENABLED
