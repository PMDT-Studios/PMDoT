#include "register_types.h"

#include "core/object/class_db.h"
#include "pmdot_pixel_pipeline.h"
#include "editor/pixel_art_presets.h"
#include "editor/aseprite_workspace.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#include "editor/plugins/editor_plugin.h"
#endif

void initialize_pmdot_aseprite_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(PMDoTPixelPipeline);
		GDREGISTER_CLASS(PMDoTPixelArtPresets);
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_CLASS(EditorAsepriteWorkspace);
		GDREGISTER_CLASS(AsepriteEditorPlugin);
		EditorPlugins::add_by_type<AsepriteEditorPlugin>();
	}
#endif
}

void uninitialize_pmdot_aseprite_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Clean up scene level resources if needed
	}
}
