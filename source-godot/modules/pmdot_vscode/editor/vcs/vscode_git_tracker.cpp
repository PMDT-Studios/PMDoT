#ifdef TOOLS_ENABLED

#include "vscode_git_tracker.h"

#include "core/io/json.h"

void VSCodeGitTracker::attach(WebViewBackend *p_backend) {
	backend = p_backend;
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

#endif // TOOLS_ENABLED
