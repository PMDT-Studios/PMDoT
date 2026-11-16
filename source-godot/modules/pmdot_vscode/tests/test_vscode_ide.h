#ifndef TEST_VSCODE_IDE_H
#define TEST_VSCODE_IDE_H

#include "tests/test_macros.h"

#ifdef TOOLS_ENABLED
#include "../editor/debugger/vscode_dap_client.h"
#include "../editor/vcs/vscode_git_tracker.h"
#include "../editor/vscode_bridge.h"

namespace TestVSCodeIDE {

TEST_CASE("[Modules][PMDoTVSCode] Language detection") {
	CHECK(VSCodeBridge::detect_language_id("res://scripts/Player.cs") == "csharp");
	CHECK(VSCodeBridge::detect_language_id("res://src/engine.cpp") == "cpp");
	CHECK(VSCodeBridge::detect_language_id("res://src/engine.h") == "cpp");
	CHECK(VSCodeBridge::detect_language_id("res://scripts/Main.java") == "java");
	CHECK(VSCodeBridge::detect_language_id("res://README.md") == "markdown");
}

TEST_CASE("[Modules][PMDoTVSCode] DAP breakpoint toggle") {
	VSCodeDAPClient dap;
	dap.toggle_breakpoint("res://scripts/Player.cs", 42);
	CHECK(dap.get_breakpoints("res://scripts/Player.cs").size() == 1);
	CHECK(dap.get_breakpoints("res://scripts/Player.cs")[0] == 42);

	dap.toggle_breakpoint("res://scripts/Player.cs", 42);
	CHECK(dap.get_breakpoints("res://scripts/Player.cs").is_empty());
}

TEST_CASE("[Modules][PMDoTVSCode] Git unified diff hunk parsing") {
	String diff = "@@ -10,0 +11,3 @@\n+line1\n+line2\n+line3\n@@ -20,2 +23,0 @@\n";
	Vector<VSCodeGitTracker::GutterHunk> hunks = VSCodeGitTracker::parse_unified_diff_hunks(diff);
	CHECK(hunks.size() == 2);
	CHECK(hunks[0].start_line == 11);
	CHECK(hunks[0].end_line == 13);
	CHECK(hunks[0].type == VSCodeGitTracker::CHANGE_ADDED);
	CHECK(hunks[1].start_line == 23);
	CHECK(hunks[1].type == VSCodeGitTracker::CHANGE_DELETED);
}

} // namespace TestVSCodeIDE

#endif // TOOLS_ENABLED

#endif // TEST_VSCODE_IDE_H

