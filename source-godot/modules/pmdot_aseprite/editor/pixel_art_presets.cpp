#include "pixel_art_presets.h"
#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#include "core/object/class_db.h"

PMDoTPixelArtPresets::PMDoTPixelArtPresets() {}
PMDoTPixelArtPresets::~PMDoTPixelArtPresets() {}

void PMDoTPixelArtPresets::_bind_methods() {
	ClassDB::bind_static_method("PMDoTPixelArtPresets", D_METHOD("apply_preset", "preset", "scale_multiplier"), &PMDoTPixelArtPresets::apply_preset, DEFVAL(4));
	ClassDB::bind_static_method("PMDoTPixelArtPresets", D_METHOD("create_default_folder_structure", "project_path"), &PMDoTPixelArtPresets::create_default_folder_structure);
	ClassDB::bind_static_method("PMDoTPixelArtPresets", D_METHOD("get_preset_names"), &PMDoTPixelArtPresets::get_preset_names);

	BIND_ENUM_CONSTANT(PRESET_320X180);
	BIND_ENUM_CONSTANT(PRESET_480X270);
	BIND_ENUM_CONSTANT(PRESET_640X360);
}

void PMDoTPixelArtPresets::apply_preset(PresetResolution p_preset, int p_scale_multiplier) {
	int width = 640;
	int height = 360;

	switch (p_preset) {
		case PRESET_320X180:
			width = 320;
			height = 180;
			break;
		case PRESET_480X270:
			width = 480;
			height = 270;
			break;
		case PRESET_640X360:
		default:
			width = 640;
			height = 360;
			break;
	}

	int window_w = width * p_scale_multiplier;
	int window_h = height * p_scale_multiplier;

	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps) {
		return;
	}

	// Set resolution & scaling
	ps->set_setting("display/window/size/viewport_width", width);
	ps->set_setting("display/window/size/viewport_height", height);
	ps->set_setting("display/window/size/window_width_override", window_w);
	ps->set_setting("display/window/size/window_height_override", window_h);

	// Set stretch mode for crisp pixel rendering
	ps->set_setting("display/window/stretch/mode", "canvas_items");
	ps->set_setting("display/window/stretch/aspect", "keep");

	// Set nearest texture filtering by default (Filter = 0)
	ps->set_setting("rendering/textures/canvas_textures/default_texture_filter", 0);

	// Save settings
	ps->save();
}

void PMDoTPixelArtPresets::create_default_folder_structure(const String &p_project_path) {
	Ref<DirAccess> dir = DirAccess::open(p_project_path);
	if (dir.is_null()) {
		dir = DirAccess::create_for_path(p_project_path);
	}

	if (dir.is_valid()) {
		dir->make_dir_recursive(p_project_path.path_join("assets/sprites"));
		dir->make_dir_recursive(p_project_path.path_join("assets/audio"));
		dir->make_dir_recursive(p_project_path.path_join("assets/fonts"));
		dir->make_dir_recursive(p_project_path.path_join("scenes"));
		dir->make_dir_recursive(p_project_path.path_join("scripts"));
	}
}

Array PMDoTPixelArtPresets::get_preset_names() {
	Array names;
	names.push_back("Retro Ultra Low (320x180)");
	names.push_back("Retro Standard (480x270)");
	names.push_back("Modern Pixel Art (640x360)");
	return names;
}
