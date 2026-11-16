#include "register_types.h"

void initialize_pmdot_vscode_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		// Editor plugin registration added in subsequent commits.
	}
}

void uninitialize_pmdot_vscode_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		// Cleanup editor resources.
	}
}
