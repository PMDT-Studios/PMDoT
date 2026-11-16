#include "register_types.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#include "editor/plugins/editor_plugin.h"
#include "editor/vscode_editor_dock.h"
#endif

void initialize_pmdot_vscode_module(ModuleInitializationLevel p_level) {
#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_CLASS(VSCodeEditorDock);
		GDREGISTER_CLASS(PMDoTVSCodeEditorPlugin);
		EditorPlugins::add_by_type<PMDoTVSCodeEditorPlugin>();
	}
#endif
}

void uninitialize_pmdot_vscode_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		// Dock lifecycle is managed by EditorNode tree.
	}
}
