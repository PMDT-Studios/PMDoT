#ifndef ASEPRITE_WORKSPACE_H
#define ASEPRITE_WORKSPACE_H

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/color_picker.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"
#include "scene/gui/spin_box.h"
#include "scene/resources/image_texture.h"

#include "workspace_undo_state.h"
#include "pmdsprite_history.h"
#include "pmdsprite_animation.h"
#include "pmdsprite_tools.h"
#include "pmdsprite_exporter.h"
#include "pmdsprite_canvas.h"

#ifdef TOOLS_ENABLED
#include "editor/gui/editor_file_dialog.h"
#endif

class EditorAsepriteWorkspace : public VBoxContainer {
	GDCLASS(EditorAsepriteWorkspace, VBoxContainer);

	enum ToolType {
		TOOL_PENCIL,
		TOOL_ERASER,
		TOOL_PICKER,
		TOOL_FILL
	};

private:
	HBoxContainer *toolbar = nullptr;
	HBoxContainer *pages_bar = nullptr;
	HBoxContainer *workspace_body = nullptr;
	VBoxContainer *tools_panel = nullptr;
	PmdspriteCanvas *canvas = nullptr;
	VBoxContainer *palette_panel = nullptr;
	HBoxContainer *status_bar = nullptr;

	LineEdit *sprite_name_input = nullptr;
	LineEdit *export_dir_input = nullptr;
	Button *btn_browse_dir = nullptr;

	SpinBox *canvas_w_spin = nullptr;
	SpinBox *canvas_h_spin = nullptr;
	OptionButton *quick_size_selector = nullptr;
	SpinBox *brush_size_spin = nullptr;

	ColorPickerButton *color_picker_btn = nullptr;
	GridContainer *palette_grid = nullptr;

	Vector<Ref<Image>> pages;
	int current_page_index = 0;
	HBoxContainer *page_buttons_hb = nullptr;
	Button *btn_add_page = nullptr;
	Button *btn_duplicate_page = nullptr;
	Button *btn_delete_page = nullptr;
	Button *btn_play_animation = nullptr;
	Label *animation_status_label = nullptr;
	LineEdit *animation_fps_line_edit = nullptr;

	Color current_color = Color(1, 1, 1, 1);

	PmdspriteHistory history;
	PmdspriteAnimation animation;

	Button *btn_pencil = nullptr;
	Button *btn_eraser = nullptr;
	Button *btn_picker = nullptr;
	Button *btn_fill = nullptr;
	Button *btn_clear = nullptr;
	Button *btn_new = nullptr;
	Button *btn_resize = nullptr;
	Button *btn_undo = nullptr;
	Button *btn_redo = nullptr;
	Button *btn_export = nullptr;
	Button *btn_zoom_in = nullptr;
	Button *btn_zoom_out = nullptr;
	Button *btn_zoom_reset = nullptr;

	OptionButton *preset_selector = nullptr;
	Label *status_label = nullptr;
	Label *cursor_pos_label = nullptr;
	Label *canvas_size_label = nullptr;
	Label *zoom_label = nullptr;

	Ref<Image> canvas_image;
	int canvas_width = 64;
	int canvas_height = 64;
	ToolType current_tool = TOOL_PENCIL;

#ifdef TOOLS_ENABLED
	EditorFileDialog *export_file_dialog = nullptr;
#endif

protected:
	static void _bind_methods();
	void _notification(int p_what);
	virtual void unhandled_key_input(const Ref<InputEvent> &p_event) override;

	void _on_tool_selected(int p_tool_id);
	void _on_preset_selected(int p_index);
	void _on_quick_size_selected(int p_index);
	void _on_new_canvas_pressed();
	void _on_resize_pressed();
	void _on_export_pressed();
	void _on_browse_dir_pressed();
	void _on_export_dir_selected(const String &p_dir);
	void _on_clear_pressed();
	void _on_color_changed(const Color &p_color);
	void _on_palette_color_clicked(const Color &p_color);

	void _on_add_page_pressed();
	void _on_duplicate_page_pressed();
	void _on_delete_page_pressed();
	void _select_page(int p_index);
	void _update_pages_ui();
	void _update_canvas_page(int p_index);
	void _on_play_animation_pressed();

	void _save_undo_state();
	void _undo();
	void _redo();

	void _on_zoom_in();
	void _on_zoom_out();
	void _on_zoom_reset();

	void _apply_tool_at(int p_x, int p_y);
	void _draw_line_brush(int p_x0, int p_y0, int p_x1, int p_y1, Color p_color);
	void _update_tool_buttons();
	void _setup_palette();
	void _resize_canvas(int p_new_width, int p_new_height, bool p_scale);


	void _on_canvas_pixel_pressed(const Vector2i &p_pos);
	void _on_canvas_pixel_dragged(const Vector2i &p_from, const Vector2i &p_to);
	void _on_canvas_pixel_hovered(const Vector2i &p_pos);
	void _on_canvas_zoom_changed(float p_zoom);

	Ref<Texture2D> _load_svg_icon(const char *p_svg);

public:
	EditorAsepriteWorkspace();
	~EditorAsepriteWorkspace();

	void initialize_workspace(int p_width = 64, int p_height = 64);
	void update_canvas_texture();
};

#ifdef TOOLS_ENABLED
#include "editor/plugins/editor_plugin.h"

class AsepriteEditorPlugin : public EditorPlugin {
	GDCLASS(AsepriteEditorPlugin, EditorPlugin);

private:
	EditorAsepriteWorkspace *aseprite_workspace = nullptr;

public:
	virtual String get_name() const override { return "Aseprite"; }
	virtual bool has_main_screen() const override { return true; }
	virtual void make_visible(bool p_visible) override;
	virtual void edit(Object *p_object) override {}
	virtual bool handles(Object *p_object) const override { return false; }

	AsepriteEditorPlugin();
	~AsepriteEditorPlugin();
};
#endif

#endif // ASEPRITE_WORKSPACE_H
