#ifndef ASEPRITE_WORKSPACE_H
#define ASEPRITE_WORKSPACE_H

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/center_container.h"
#include "scene/gui/color_picker.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/texture_rect.h"
#include "scene/resources/image_texture.h"

#ifdef TOOLS_ENABLED
#include "editor/gui/editor_file_dialog.h"
#endif

class EditorAsepriteWorkspace : public VBoxContainer {
	GDCLASS(EditorAsepriteWorkspace, VBoxContainer);

private:
	HBoxContainer *toolbar = nullptr;
	HBoxContainer *pages_bar = nullptr;
	HBoxContainer *workspace_body = nullptr;
	VBoxContainer *tools_panel = nullptr;
	PanelContainer *canvas_container = nullptr;
	CenterContainer *canvas_center_container = nullptr;
	PanelContainer *canvas_viewport_box = nullptr;
	TextureRect *canvas_bg_texture_rect = nullptr;
	TextureRect *canvas_texture_rect = nullptr;
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

	Ref<Image> canvas_image;
	Ref<ImageTexture> canvas_texture;
	Ref<ImageTexture> checkerboard_texture;

	Color current_color = Color(1, 1, 1, 1);
	int canvas_width = 64;
	int canvas_height = 64;
	float zoom_level = 8.0f;

	bool is_drawing = false;
	Vector2i last_mouse_px = Vector2i(-1, -1);

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

#ifdef TOOLS_ENABLED
	EditorFileDialog *export_file_dialog = nullptr;
#endif

	struct WorkspaceUndoState {
		int width = 64;
		int height = 64;
		int page_index = 0;
		Vector<Ref<Image>> pages_backup;
	};

	Vector<WorkspaceUndoState> undo_stack;
	Vector<WorkspaceUndoState> redo_stack;

	enum ToolType {
		TOOL_PENCIL,
		TOOL_ERASER,
		TOOL_PICKER,
		TOOL_FILL
	};

	ToolType current_tool = TOOL_PENCIL;

protected:
	static void _bind_methods();
	void _notification(int p_what);
	virtual void unhandled_key_input(const Ref<InputEvent> &p_event) override;
	void _gui_input_canvas(const Ref<InputEvent> &p_event);

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

	void _save_undo_state();
	void _undo();
	void _redo();

	void _on_zoom_in();
	void _on_zoom_out();
	void _on_zoom_reset();

	void _apply_tool_at(int p_x, int p_y);
	void _draw_brush_at(int p_x, int p_y, Color p_color);
	void _draw_line_brush(int p_x0, int p_y0, int p_x1, int p_y1, Color p_color);
	void _flood_fill(int p_x, int p_y, Color p_target, Color p_replacement);
	void _update_tool_buttons();
	void _setup_palette();
	void _create_checkerboard_texture();
	void _update_canvas_rect_size();
	void _resize_canvas(int p_new_width, int p_new_height, bool p_scale);

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
