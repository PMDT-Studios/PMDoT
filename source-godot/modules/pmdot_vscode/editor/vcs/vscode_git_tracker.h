#ifndef PMDOT_VSCODE_GIT_TRACKER_H
#define PMDOT_VSCODE_GIT_TRACKER_H

#ifdef TOOLS_ENABLED

#include "../platform/webview_backend.h"
#include "core/templates/list.h"
#include "core/templates/vector.h"

class VSCodeGitTracker {
public:
	enum ChangeType {
		CHANGE_ADDED,
		CHANGE_MODIFIED,
		CHANGE_DELETED
	};

	struct GutterHunk {
		int start_line = 1;
		int end_line = 1;
		ChangeType type = CHANGE_MODIFIED;
	};

private:
	WebViewBackend *backend = nullptr;

	String _get_project_root() const;
	String _run_git(const List<String> &p_args, int *r_exit_code = nullptr) const;

public:
	void attach(WebViewBackend *p_backend);
	static Vector<GutterHunk> parse_unified_diff_hunks(const String &p_diff_text);
	void push_gutter_hunks(const String &p_path, const Vector<GutterHunk> &p_hunks);
	void open_side_by_side_diff(const String &p_path, const String &p_original, const String &p_modified, const String &p_language);

	void refresh_status();
	void refresh_graph();
	void stage_file(const String &p_rel_path);
	void unstage_file(const String &p_rel_path);
	void commit_changes(const String &p_message);
	void open_file_diff(const String &p_rel_path);
};

#endif // TOOLS_ENABLED

#endif // PMDOT_VSCODE_GIT_TRACKER_H
