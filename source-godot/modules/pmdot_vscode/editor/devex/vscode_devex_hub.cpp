#ifdef TOOLS_ENABLED

#include "vscode_devex_hub.h"

#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/os/os.h"

void VSCodeDevExHub::attach(WebViewBackend *p_backend) {
	backend = p_backend;
}

void VSCodeDevExHub::push_log(const String &p_level, const String &p_text) {
	Dictionary entry;
	entry["level"] = p_level;
	entry["text"] = p_text;
	log_buffer.push_back(entry);
	if (log_buffer.size() > max_logs) {
		log_buffer.pop_front();
	}

	if (!backend || !backend->is_ready()) {
		return;
	}
	Dictionary params;
	params["logs"] = log_buffer;
	Dictionary msg;
	msg["method"] = "logs_update";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

void VSCodeDevExHub::clear_logs() {
	log_buffer.clear();
	if (!backend || !backend->is_ready()) {
		return;
	}
	Dictionary params;
	params["logs"] = log_buffer;
	Dictionary msg;
	msg["method"] = "logs_update";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

void VSCodeDevExHub::open_external_url(const String &p_url) {
	if (!p_url.is_empty()) {
		OS::get_singleton()->shell_open(p_url);
	}
}

void VSCodeDevExHub::_scan_scenes_recursive(const String &p_dir, Array &r_scenes) {
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
			_scan_scenes_recursive(full, r_scenes);
		} else if (name.ends_with(".tscn")) {
			Ref<FileAccess> fa = FileAccess::open(full, FileAccess::READ);
			if (fa.is_valid()) {
				Array nodes;
				String current_node;
				while (!fa->eof_reached()) {
					String line = fa->get_line().strip_edges();
					if (line.begins_with("[node ")) {
						int n_start = line.find("name=\"");
						if (n_start >= 0) {
							int n_end = line.find("\"", n_start + 6);
							if (n_end > n_start + 6) {
								current_node = line.substr(n_start + 6, n_end - n_start - 6);
								Dictionary node_info;
								node_info["name"] = current_node;
								node_info["parent"] = line.contains("parent=\"") ? "child" : "root";
								nodes.push_back(node_info);
							}
						}
					}
				}
				Dictionary scene_info;
				scene_info["path"] = full;
				scene_info["name"] = name;
				scene_info["nodes"] = nodes;
				r_scenes.push_back(scene_info);
			}
		}
		name = da->get_next();
	}
	da->list_dir_end();
}

void VSCodeDevExHub::scan_scene_bindings() {
	if (!backend || !backend->is_ready()) {
		return;
	}
	Array scenes;
	_scan_scenes_recursive("res://", scenes);
	Dictionary params;
	params["scenes"] = scenes;
	Dictionary msg;
	msg["method"] = "scene_graph_update";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

void VSCodeDevExHub::_scan_todos_recursive(const String &p_dir, Array &r_todos) {
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
			_scan_todos_recursive(full, r_todos);
		} else if (name.ends_with(".cs") || name.ends_with(".cpp") || name.ends_with(".h") || name.ends_with(".java")) {
			Ref<FileAccess> fa = FileAccess::open(full, FileAccess::READ);
			if (fa.is_valid()) {
				int line_num = 1;
				while (!fa->eof_reached()) {
					String line = fa->get_line();
					if (line.contains("TODO") || line.contains("FIXME") || line.contains("HACK")) {
						Dictionary item;
						item["path"] = full;
						item["name"] = name;
						item["line"] = line_num;
						item["text"] = line.strip_edges().substr(0, 100);
						r_todos.push_back(item);
					}
					line_num++;
				}
			}
		}
		name = da->get_next();
	}
	da->list_dir_end();
}

void VSCodeDevExHub::scan_todos() {
	if (!backend || !backend->is_ready()) {
		return;
	}
	Array todos;
	_scan_todos_recursive("res://", todos);
	Dictionary params;
	params["todos"] = todos;
	Dictionary msg;
	msg["method"] = "todos_update";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

#endif // TOOLS_ENABLED
