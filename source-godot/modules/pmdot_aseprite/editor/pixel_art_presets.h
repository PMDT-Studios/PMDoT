#ifndef PMDOT_PIXEL_ART_PRESETS_H
#define PMDOT_PIXEL_ART_PRESETS_H

#include "core/object/ref_counted.h"
#include "core/typedefs.h"
#include "core/variant/dictionary.h"

class PMDoTPixelArtPresets : public RefCounted {
	GDCLASS(PMDoTPixelArtPresets, RefCounted);

public:
	enum PresetResolution {
		PRESET_320X180 = 0,
		PRESET_480X270 = 1,
		PRESET_640X360 = 2,
	};

protected:
	static void _bind_methods();

public:
	PMDoTPixelArtPresets();
	~PMDoTPixelArtPresets();

	// Apply Pixel Art settings to Godot ProjectSettings
	static void apply_preset(PresetResolution p_preset, int p_scale_multiplier = 4);

	// Create default directory structure in target directory
	static void create_default_folder_structure(const String &p_project_path);

	// Get list of available preset options for UI
	static Array get_preset_names();
};

VARIANT_ENUM_CAST(PMDoTPixelArtPresets::PresetResolution);

#endif // PMDOT_PIXEL_ART_PRESETS_H
