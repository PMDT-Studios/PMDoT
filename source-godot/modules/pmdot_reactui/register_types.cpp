#include "register_types.h"
#include "src/react_ui.h"

void initialize_pmdot_reactui_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(ReactUI);
	}
}

void uninitialize_pmdot_reactui_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Clean up scene level resources if needed
	}
}
