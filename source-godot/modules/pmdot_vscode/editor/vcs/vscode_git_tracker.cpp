#ifdef TOOLS_ENABLED

#include "vscode_git_tracker.h"

#include "../vscode_bridge.h"
#include "core/config/project_settings.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/os/os.h"

void VSCodeGitTracker::attach(WebViewBackend *p_backend) {
	backend = p_backend;
}

String VSCodeGitTracker::_get_project_root() const {
	if (ProjectSettings::get_singleton()) {
		return ProjectSettings::get_singleton()->globalize_path("res://");
	}
	return ".";
}

String VSCodeGitTracker::_run_git(const List<String> &p_args, int *r_exit_code) const {
	List<String> args;
	args.push_back("-C");
	args.push_back(_get_project_root());
	for (const String &a : p_args) {
		args.push_back(a);
	}
	String out;
	int code = 0;
	OS::get_singleton()->execute("git", args, &out, &code, true);
	if (r_exit_code) {
		*r_exit_code = code;
	}
	return out;
}

Vector<VSCodeGitTracker::GutterHunk> VSCodeGitTracker::parse_unified_diff_hunks(const String &p_diff_text) {
	Vector<GutterHunk> result;
	PackedStringArray lines = p_diff_text.split("\n");
	for (const String &line : lines) {
		if (!line.begins_with("@@ ")) {
			continue;
		}
		int plus_idx = line.find("+");
		int end_idx = line.find(" @@", plus_idx);
		if (plus_idx < 0 || end_idx < 0) {
			continue;
		}
		String new_range = line.substr(plus_idx + 1, end_idx - plus_idx - 1);
		int comma_idx = new_range.find(",");
		int start = comma_idx >= 0 ? new_range.substr(0, comma_idx).to_int() : new_range.to_int();
		int count = comma_idx >= 0 ? new_range.substr(comma_idx + 1).to_int() : 1;

		GutterHunk hunk;
		hunk.start_line = MAX(1, start);
		if (count == 0) {
			hunk.type = CHANGE_DELETED;
			hunk.end_line = hunk.start_line;
		} else {
			hunk.type = line.contains("-0,0") ? CHANGE_ADDED : CHANGE_MODIFIED;
			hunk.end_line = hunk.start_line + count - 1;
		}
		result.push_back(hunk);
	}
	return result;
}

void VSCodeGitTracker::push_gutter_hunks(const String &p_path, const Vector<GutterHunk> &p_hunks) {
	if (!backend || !backend->is_ready()) {
		return;
	}
	Array items;
	for (const GutterHunk &h : p_hunks) {
		Dictionary d;
		d["startLine"] = h.start_line;
		d["endLine"] = h.end_line;
		d["type"] = int(h.type);
		items.push_back(d);
	}
	Dictionary params;
	params["path"] = p_path;
	params["hunks"] = items;

	Dictionary msg;
	msg["method"] = "git_gutter_update";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

void VSCodeGitTracker::open_side_by_side_diff(const String &p_path, const String &p_original, const String &p_modified, const String &p_language) {
	if (!backend || !backend->is_ready()) {
		return;
	}
	Dictionary params;
	params["path"] = p_path;
	params["original"] = p_original;
	params["modified"] = p_modified;
	params["language"] = p_language;

	Dictionary msg;
	msg["method"] = "git_open_diff";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

void VSCodeGitTracker::refresh_status() {
	if (!backend || !backend->is_ready()) {
		return;
	}
	List<String> args;
	args.push_back("status");
	args.push_back("--porcelain");
	args.push_back("-b");
	String out = _run_git(args);

	String branch = "HEAD";
	Array changes;
	PackedStringArray lines = out.split("\n", false);
	for (const String &line : lines) {
		if (line.begins_with("## ")) {
			String b = line.substr(3).strip_edges();
			int dots = b.find("...");
			branch = dots >= 0 ? b.substr(0, dots) : b;
		} else if (line.length() >= 4) {
			String rel_path = line.substr(3).strip_edges();
			if (rel_path.begins_with("\"") && rel_path.ends_with("\"") && rel_path.length() >= 2) {
				rel_path = rel_path.substr(1, rel_path.length() - 2).c_unescape();
			}
			Dictionary item;
			item["index"] = line.substr(0, 1);
			item["worktree"] = line.substr(1, 1);
			item["path"] = rel_path;
			changes.push_back(item);
		}
	}

	Dictionary params;
	params["branch"] = branch;
	params["changes"] = changes;

	Dictionary msg;
	msg["method"] = "git_status_update";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

void VSCodeGitTracker::refresh_graph() {
	if (!backend || !backend->is_ready()) {
		return;
	}
	List<String> args;
	args.push_back("log");
	args.push_back("-n");
	args.push_back("25");
	args.push_back("--pretty=format:%h|%p|%an|%ar|%s");
	String out = _run_git(args);

	Array commits;
	PackedStringArray lines = out.split("\n", false);
	for (const String &line : lines) {
		PackedStringArray cols = line.split("|");
		if (cols.size() >= 5) {
			Dictionary c;
			c["hash"] = cols[0];
			c["parents"] = cols[1];
			c["author"] = cols[2];
			c["date"] = cols[3];
			c["message"] = cols[4];
			commits.push_back(c);
		}
	}

	Dictionary params;
	params["commits"] = commits;

	Dictionary msg;
	msg["method"] = "git_graph_update";
	msg["params"] = params;
	backend->post_message(JSON::stringify(msg));
}

void VSCodeGitTracker::stage_file(const String &p_rel_path) {
	List<String> args;
	args.push_back("add");
	args.push_back(p_rel_path);
	_run_git(args);
	refresh_status();
}

void VSCodeGitTracker::unstage_file(const String &p_rel_path) {
	List<String> args;
	args.push_back("reset");
	args.push_back("HEAD");
	args.push_back("--");
	args.push_back(p_rel_path);
	_run_git(args);
	refresh_status();
}

void VSCodeGitTracker::commit_changes(const String &p_message) {
	if (p_message.strip_edges().is_empty()) {
		return;
	}
	List<String> args;
	args.push_back("commit");
	args.push_back("-m");
	args.push_back(p_message);
	_run_git(args);
	refresh_status();
	refresh_graph();
}

void VSCodeGitTracker::open_file_diff(const String &p_rel_path) {
	List<String> args;
	args.push_back("show");
	args.push_back("HEAD:" + p_rel_path);
	int code = 0;
	String original = _run_git(args, &code);
	if (code != 0) {
		original = "";
	}

	String full_path = _get_project_root().path_join(p_rel_path);
	String modified = FileAccess::get_file_as_string(full_path);
	open_side_by_side_diff(p_rel_path, original, modified, VSCodeBridge::detect_language_id(p_rel_path));
}

#endif // TOOLS_ENABLED
