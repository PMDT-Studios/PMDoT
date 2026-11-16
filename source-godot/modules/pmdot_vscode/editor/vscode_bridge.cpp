#ifdef TOOLS_ENABLED

#include "vscode_bridge.h"

#include "core/io/dir_access.h"
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

void VSCodeBridge::set_ready_callback(ReadyCallback p_callback) {
	ready_callback = p_callback;
}

String VSCodeBridge::detect_language_id(const String &p_path) {
	String ext = p_path.get_extension().to_lower();
	if (ext == "cs" || ext == "csproj") {
		return ext == "csproj" ? "xml" : "csharp";
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

bool VSCodeBridge::_is_supported_code_file(const String &p_ext) {
	String e = p_ext.to_lower();
	return e == "cs" || e == "csproj" || e == "sln" ||
			e == "cpp" || e == "cc" || e == "cxx" || e == "h" || e == "hpp" || e == "c" ||
			e == "java" || e == "md" || e == "json" || e == "glsl" || e == "txt";
}

String VSCodeBridge::_build_template(const String &p_res_path) {
	String base = p_res_path.get_file().get_basename();
	String ext = p_res_path.get_extension().to_lower();
	if (ext == "cs") {
		return "using Godot;\nusing System;\n\npublic partial class " + base + " : Node\n{\n\tpublic override void _Ready()\n\t{\n\t}\n\n\tpublic override void _Process(double delta)\n\t{\n\t}\n}\n";
	}
	if (ext == "cpp") {
		return "#include \"" + base + ".h\"\n\nvoid " + base + "::init() {\n}\n";
	}
	if (ext == "h" || ext == "hpp") {
		String guard = base.to_upper() + "_H";
		return "#ifndef " + guard + "\n#define " + guard + "\n\nclass " + base + " {\npublic:\n\tvoid init();\n};\n\n#endif // " + guard + "\n";
	}
	if (ext == "java") {
		return "public class " + base + " {\n    public static void main(String[] args) {\n    }\n}\n";
	}
	if (ext == "md") {
		return "# " + base + "\n\n";
	}
	return "";
}

void VSCodeBridge::_scan_dir_recursive(const String &p_dir, Array &r_files) const {
	Ref<DirAccess> da = DirAccess::open(p_dir);
	if (da.is_null()) {
		return;
	}
	da->list_dir_begin();
	String name = da->get_next();
	while (!name.is_empty()) {
		if (name.begins_with(".") || name == "bin" || name == "obj" || name == "node_modules") {
			name = da->get_next();
			continue;
		}
		String full = p_dir.ends_with("/") ? (p_dir + name) : (p_dir + "/" + name);
		if (da->current_is_dir()) {
			_scan_dir_recursive(full, r_files);
		} else if (_is_supported_code_file(name.get_extension())) {
			Dictionary item;
			item["path"] = full;
			item["name"] = name;
			item["language"] = detect_language_id(full);
			r_files.push_back(item);
		}
		name = da->get_next();
	}
	da->list_dir_end();
}

void VSCodeBridge::refresh_project_tree() {
	Array files;
	_scan_dir_recursive("res://", files);
	Dictionary params;
	params["files"] = files;
	_send_payload("project_tree", params);
}

void VSCodeBridge::search_in_files(const String &p_query) {
	Array matches;
	if (p_query.is_empty()) {
		Dictionary params;
		params["query"] = p_query;
		params["matches"] = matches;
		_send_payload("search_results", params);
		return;
	}

	Array files;
	_scan_dir_recursive("res://", files);
	String q_lower = p_query.to_lower();

	for (int i = 0; i < files.size(); i++) {
		Dictionary f = files[i];
		String path = f["path"];
		Ref<FileAccess> fa = FileAccess::open(path, FileAccess::READ);
		if (fa.is_null()) {
			continue;
		}
		int line_num = 1;
		while (!fa->eof_reached()) {
			String line = fa->get_line();
			if (line.to_lower().contains(q_lower)) {
				Dictionary m;
				m["path"] = path;
				m["name"] = f["name"];
				m["line"] = line_num;
				m["preview"] = line.strip_edges().substr(0, 120);
				matches.push_back(m);
				if (matches.size() >= 100) {
					break;
				}
			}
			line_num++;
		}
		if (matches.size() >= 100) {
			break;
		}
	}

	Dictionary params;
	params["query"] = p_query;
	params["matches"] = matches;
	_send_payload("search_results", params);
}

bool VSCodeBridge::create_file(const String &p_res_path) {
	String path = p_res_path.begins_with("res://") ? p_res_path : ("res://" + p_res_path);
	String dir = path.get_base_dir();
	if (!DirAccess::exists(dir)) {
		DirAccess::make_dir_recursive_absolute(dir);
	}
	if (!FileAccess::exists(path)) {
		Ref<FileAccess> fa = FileAccess::open(path, FileAccess::WRITE);
		if (fa.is_null()) {
			return false;
		}
		fa->store_string(_build_template(path));
		fa->flush();
	}
	if (EditorFileSystem::get_singleton()) {
		EditorFileSystem::get_singleton()->update_file(path);
	}
	refresh_project_tree();
	return open_file(path);
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

	if (method == "ready") {
		refresh_project_tree();
		if (ready_callback) {
			ready_callback();
		}
	} else if (method == "request_tree") {
		refresh_project_tree();
	} else if (method == "open_file_req") {
		String path = params.get("path", "");
		if (!path.is_empty()) {
			open_file(path);
		}
	} else if (method == "create_file_req") {
		String path = params.get("path", "");
		if (!path.is_empty()) {
			create_file(path);
		}
	} else if (method == "search_files_req") {
		String query = params.get("query", "");
		search_in_files(query);
	} else if (method == "save_script") {
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
