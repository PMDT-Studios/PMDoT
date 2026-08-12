#include "aseprite_workspace.h"
#include "pixel_art_presets.h"
#include "modules/pmdot_aseprite/pmdot_pixel_pipeline.h"
#include "core/object/class_db.h"
#include "core/io/dir_access.h"
#include "core/os/keyboard.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/separator.h"
#include "core/input/input_event.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#include "editor/editor_file_system.h"
#endif

EditorAsepriteWorkspace::EditorAsepriteWorkspace() {
	set_v_size_flags(SIZE_EXPAND_FILL);
	set_h_size_flags(SIZE_EXPAND_FILL);

	_create_checkerboard_texture();

	// --- Top Toolbar ---
	toolbar = memnew(HBoxContainer);
	add_child(toolbar);

	Label *lbl_name = memnew(Label);
	lbl_name->set_text(" Sprite Name: ");
	toolbar->add_child(lbl_name);

	sprite_name_input = memnew(LineEdit);
	sprite_name_input->set_text("sprite_01");
	sprite_name_input->set_custom_minimum_size(Vector2(100, 0));
	toolbar->add_child(sprite_name_input);

	toolbar->add_child(memnew(VSeparator));

	Label *lbl_dir = memnew(Label);
	lbl_dir->set_text(" Target Folder: ");
	toolbar->add_child(lbl_dir);

	export_dir_input = memnew(LineEdit);
	export_dir_input->set_text("res://assets/sprites/");
	export_dir_input->set_custom_minimum_size(Vector2(160, 0));
	toolbar->add_child(export_dir_input);

	btn_browse_dir = memnew(Button);
	btn_browse_dir->set_text(" Browse... ");
	toolbar->add_child(btn_browse_dir);
	btn_browse_dir->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_browse_dir_pressed));

	toolbar->add_child(memnew(VSeparator));

	Label *lbl_dim = memnew(Label);
	lbl_dim->set_text(" Size: ");
	toolbar->add_child(lbl_dim);

	canvas_w_spin = memnew(SpinBox);
	canvas_w_spin->set_min(8);
	canvas_w_spin->set_max(2048);
	canvas_w_spin->set_value(64);
	canvas_w_spin->set_step(8);
	toolbar->add_child(canvas_w_spin);

	Label *lbl_x = memnew(Label);
	lbl_x->set_text("x");
	toolbar->add_child(lbl_x);

	canvas_h_spin = memnew(SpinBox);
	canvas_h_spin->set_min(8);
	canvas_h_spin->set_max(2048);
	canvas_h_spin->set_value(64);
	canvas_h_spin->set_step(8);
	toolbar->add_child(canvas_h_spin);

	quick_size_selector = memnew(OptionButton);
	quick_size_selector->add_item("Custom Size", 0);
	quick_size_selector->add_item("16 x 16", 1);
	quick_size_selector->add_item("32 x 32", 2);
	quick_size_selector->add_item("48 x 48", 3);
	quick_size_selector->add_item("64 x 64", 4);
	quick_size_selector->add_item("128 x 128", 5);
	quick_size_selector->add_item("256 x 256", 6);
	quick_size_selector->select(4);
	toolbar->add_child(quick_size_selector);
	quick_size_selector->connect("item_selected", callable_mp(this, &EditorAsepriteWorkspace::_on_quick_size_selected));

	btn_resize = memnew(Button);
	btn_resize->set_text(" Scale Canvas ");
	btn_resize->set_tooltip_text("Resizes and scales existing artwork to new dimensions");
	toolbar->add_child(btn_resize);
	btn_resize->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_resize_pressed));

	btn_new = memnew(Button);
	btn_new->set_text(" New Blank ");
	btn_new->set_tooltip_text("Creates a new blank canvas with current dimensions");
	toolbar->add_child(btn_new);
	btn_new->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_new_canvas_pressed));

	toolbar->add_child(memnew(VSeparator));

	btn_undo = memnew(Button);
	btn_undo->set_text(" Undo ");
	toolbar->add_child(btn_undo);
	btn_undo->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_undo));

	btn_redo = memnew(Button);
	btn_redo->set_text(" Redo ");
	toolbar->add_child(btn_redo);
	btn_redo->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_redo));

	toolbar->add_child(memnew(VSeparator));

	btn_export = memnew(Button);
	btn_export->set_text(" Export PNG ");
	toolbar->add_child(btn_export);
	btn_export->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_export_pressed));

	toolbar->add_child(memnew(VSeparator));

	btn_zoom_out = memnew(Button);
	btn_zoom_out->set_text(" - ");
	toolbar->add_child(btn_zoom_out);
	btn_zoom_out->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_zoom_out));

	zoom_label = memnew(Label);
	zoom_label->set_text(" 8x ");
	toolbar->add_child(zoom_label);

	btn_zoom_in = memnew(Button);
	btn_zoom_in->set_text(" + ");
	toolbar->add_child(btn_zoom_in);
	btn_zoom_in->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_zoom_in));

	btn_zoom_reset = memnew(Button);
	btn_zoom_reset->set_text(" Reset ");
	toolbar->add_child(btn_zoom_reset);
	btn_zoom_reset->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_zoom_reset));

	toolbar->add_child(memnew(VSeparator));

	Label *lbl_preset = memnew(Label);
	lbl_preset->set_text(" Preset: ");
	toolbar->add_child(lbl_preset);

	preset_selector = memnew(OptionButton);
	Array presets = PMDoTPixelArtPresets::get_preset_names();
	for (int i = 0; i < presets.size(); ++i) {
		preset_selector->add_item(presets[i], i);
	}
	preset_selector->select(2);
	toolbar->add_child(preset_selector);
	preset_selector->connect("item_selected", callable_mp(this, &EditorAsepriteWorkspace::_on_preset_selected));

	// --- Pages / Frames Navigation Bar ---
	pages_bar = memnew(HBoxContainer);
	add_child(pages_bar);

	Label *lbl_pages = memnew(Label);
	lbl_pages->set_text(" Pages/Frames: ");
	pages_bar->add_child(lbl_pages);

	page_buttons_hb = memnew(HBoxContainer);
	pages_bar->add_child(page_buttons_hb);

	btn_add_page = memnew(Button);
	btn_add_page->set_text(" + ");
	btn_add_page->set_tooltip_text("Create new blank page/frame");
	pages_bar->add_child(btn_add_page);
	btn_add_page->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_add_page_pressed));

	btn_duplicate_page = memnew(Button);
	btn_duplicate_page->set_text(" Duplicate ");
	btn_duplicate_page->set_tooltip_text("Duplicate active page/frame");
	pages_bar->add_child(btn_duplicate_page);
	btn_duplicate_page->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_duplicate_page_pressed));

	btn_delete_page = memnew(Button);
	btn_delete_page->set_text(" Delete ");
	btn_delete_page->set_tooltip_text("Delete active page/frame");
	pages_bar->add_child(btn_delete_page);
	btn_delete_page->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_delete_page_pressed));

	// --- Main Workspace Body ---
	workspace_body = memnew(HBoxContainer);
	workspace_body->set_v_size_flags(SIZE_EXPAND_FILL);
	add_child(workspace_body);

	// --- Left Tools Panel ---
	tools_panel = memnew(VBoxContainer);
	tools_panel->set_custom_minimum_size(Vector2(110, 0));
	workspace_body->add_child(tools_panel);

	Label *lbl_tools = memnew(Label);
	lbl_tools->set_text("Tools");
	tools_panel->add_child(lbl_tools);

	btn_pencil = memnew(Button);
	btn_pencil->set_text("Pencil [P]");
	tools_panel->add_child(btn_pencil);
	btn_pencil->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_tool_selected).bind(TOOL_PENCIL));

	btn_eraser = memnew(Button);
	btn_eraser->set_text("Eraser [E]");
	tools_panel->add_child(btn_eraser);
	btn_eraser->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_tool_selected).bind(TOOL_ERASER));

	btn_picker = memnew(Button);
	btn_picker->set_text("Picker [I]");
	tools_panel->add_child(btn_picker);
	btn_picker->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_tool_selected).bind(TOOL_PICKER));

	btn_fill = memnew(Button);
	btn_fill->set_text("Bucket [G]");
	tools_panel->add_child(btn_fill);
	btn_fill->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_tool_selected).bind(TOOL_FILL));

	tools_panel->add_child(memnew(HSeparator));

	Label *lbl_brush = memnew(Label);
	lbl_brush->set_text("Brush Size:");
	tools_panel->add_child(lbl_brush);

	brush_size_spin = memnew(SpinBox);
	brush_size_spin->set_min(1);
	brush_size_spin->set_max(16);
	brush_size_spin->set_value(1);
	tools_panel->add_child(brush_size_spin);

	tools_panel->add_child(memnew(HSeparator));

	btn_clear = memnew(Button);
	btn_clear->set_text("Clear Canvas");
	tools_panel->add_child(btn_clear);
	btn_clear->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_clear_pressed));

	// --- Center Canvas Viewport with Checkerboard Background ---
	ScrollContainer *scroll = memnew(ScrollContainer);
	scroll->set_h_size_flags(SIZE_EXPAND_FILL);
	scroll->set_v_size_flags(SIZE_EXPAND_FILL);
	workspace_body->add_child(scroll);

	canvas_center_container = memnew(CenterContainer);
	canvas_center_container->set_h_size_flags(SIZE_EXPAND_FILL);
	canvas_center_container->set_v_size_flags(SIZE_EXPAND_FILL);
	scroll->add_child(canvas_center_container);

	canvas_viewport_box = memnew(PanelContainer);
	canvas_center_container->add_child(canvas_viewport_box);

	canvas_bg_texture_rect = memnew(TextureRect);
	canvas_bg_texture_rect->set_texture(checkerboard_texture);
	canvas_bg_texture_rect->set_stretch_mode(TextureRect::STRETCH_TILE);
	canvas_bg_texture_rect->set_texture_filter(CanvasItem::TEXTURE_FILTER_NEAREST);
	canvas_viewport_box->add_child(canvas_bg_texture_rect);

	canvas_texture_rect = memnew(TextureRect);
	canvas_texture_rect->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
	canvas_texture_rect->set_stretch_mode(TextureRect::STRETCH_SCALE);
	canvas_texture_rect->set_texture_filter(CanvasItem::TEXTURE_FILTER_NEAREST);
	canvas_viewport_box->add_child(canvas_texture_rect);
	canvas_texture_rect->connect("gui_input", callable_mp(this, &EditorAsepriteWorkspace::_gui_input_canvas));

	// --- Right Palette Panel ---
	palette_panel = memnew(VBoxContainer);
	palette_panel->set_custom_minimum_size(Vector2(140, 0));
	workspace_body->add_child(palette_panel);

	Label *lbl_palette = memnew(Label);
	lbl_palette->set_text("Color Palette");
	palette_panel->add_child(lbl_palette);

	color_picker_btn = memnew(ColorPickerButton);
	color_picker_btn->set_pick_color(current_color);
	color_picker_btn->set_custom_minimum_size(Vector2(0, 30));
	palette_panel->add_child(color_picker_btn);
	color_picker_btn->connect("color_changed", callable_mp(this, &EditorAsepriteWorkspace::_on_color_changed));

	palette_panel->add_child(memnew(HSeparator));

	palette_grid = memnew(GridContainer);
	palette_grid->set_columns(4);
	palette_panel->add_child(palette_grid);

	_setup_palette();

	// --- Bottom Status Bar ---
	status_bar = memnew(HBoxContainer);
	add_child(status_bar);

	status_label = memnew(Label);
	status_label->set_text(" PMDoT Embedded Aseprite Ready ");
	status_bar->add_child(status_label);

	status_bar->add_child(memnew(VSeparator));

	cursor_pos_label = memnew(Label);
	cursor_pos_label->set_text("Cursor: (0, 0)");
	status_bar->add_child(cursor_pos_label);

	status_bar->add_child(memnew(VSeparator));

	canvas_size_label = memnew(Label);
	canvas_size_label->set_text("Canvas: 64x64");
	status_bar->add_child(canvas_size_label);

	initialize_workspace(64, 64);
	_update_tool_buttons();
}

EditorAsepriteWorkspace::~EditorAsepriteWorkspace() {}

void EditorAsepriteWorkspace::_bind_methods() {
	ClassDB::bind_method(D_METHOD("initialize_workspace", "width", "height"), &EditorAsepriteWorkspace::initialize_workspace);
	ClassDB::bind_method(D_METHOD("update_canvas_texture"), &EditorAsepriteWorkspace::update_canvas_texture);
}

void EditorAsepriteWorkspace::_notification(int p_what) {
}

void EditorAsepriteWorkspace::_create_checkerboard_texture() {
	Ref<Image> checker_img = Image::create_empty(16, 16, false, Image::FORMAT_RGBA8);
	Color col_a = Color(0.18f, 0.18f, 0.18f, 1.0f);
	Color col_b = Color(0.28f, 0.28f, 0.28f, 1.0f);
	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 16; ++x) {
			bool is_a = ((x / 8) + (y / 8)) % 2 == 0;
			checker_img->set_pixel(x, y, is_a ? col_a : col_b);
		}
	}
	checkerboard_texture = ImageTexture::create_from_image(checker_img);
}

void EditorAsepriteWorkspace::_update_canvas_rect_size() {
	Vector2 sz = Vector2(canvas_width * zoom_level, canvas_height * zoom_level);
	if (canvas_bg_texture_rect) {
		canvas_bg_texture_rect->set_custom_minimum_size(sz);
	}
	if (canvas_texture_rect) {
		canvas_texture_rect->set_custom_minimum_size(sz);
	}
}

void EditorAsepriteWorkspace::_setup_palette() {
	const Color colors[32] = {
		Color::hex(0x000000FF), Color::hex(0x1D2B53FF), Color::hex(0x7E2553FF), Color::hex(0x008751FF),
		Color::hex(0xAB5236FF), Color::hex(0x5F574FFF), Color::hex(0xC2C3C7FF), Color::hex(0xFFF1E8FF),
		Color::hex(0xFF004DFF), Color::hex(0xFFA300FF), Color::hex(0xFFEC27FF), Color::hex(0x00E436FF),
		Color::hex(0x29ADFFFF), Color::hex(0x83769CFF), Color::hex(0xFF77A8FF), Color::hex(0xFFCCAAFF),
		Color::hex(0x00E756FF), Color::hex(0xFFC700FF), Color::hex(0xFF6C6AFF), Color::hex(0x77AEFFFF),
		Color::hex(0x80FFDBFF), Color::hex(0x480CA8FF), Color::hex(0x7209B7FF), Color::hex(0xF72585FF),
		Color::hex(0x2B2D42FF), Color::hex(0xD8F3DCFF), Color::hex(0x118AB2FF), Color::hex(0x06D6A0FF),
		Color::hex(0xFFD166FF), Color::hex(0xEF476FFF), Color::hex(0x073B4CFF), Color::hex(0xF8F9FAFF)
	};

	for (int i = 0; i < 32; ++i) {
		Button *b = memnew(Button);
		b->set_custom_minimum_size(Vector2(28, 28));
		b->set_modulate(colors[i]);
		b->set_tooltip_text(vformat("#%s", colors[i].to_html(false)));
		palette_grid->add_child(b);
		b->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_on_palette_color_clicked).bind(colors[i]));
	}
}

void EditorAsepriteWorkspace::initialize_workspace(int p_width, int p_height) {
	canvas_width = p_width;
	canvas_height = p_height;

	pages.clear();
	Ref<Image> first_page = Image::create_empty(canvas_width, canvas_height, false, Image::FORMAT_RGBA8);
	first_page->fill(Color(0, 0, 0, 0));
	pages.push_back(first_page);
	current_page_index = 0;
	canvas_image = pages[0];

	if (!canvas_texture.is_valid()) {
		canvas_texture = ImageTexture::create_from_image(canvas_image);
	} else {
		canvas_texture->set_image(canvas_image);
	}
	if (canvas_texture_rect) {
		canvas_texture_rect->set_texture(canvas_texture);
	}

	_update_canvas_rect_size();
	_update_pages_ui();

	if (canvas_size_label) {
		canvas_size_label->set_text(vformat("Canvas: %dx%d", canvas_width, canvas_height));
	}
}

void EditorAsepriteWorkspace::update_canvas_texture() {
	if (canvas_image.is_valid() && canvas_texture.is_valid()) {
		canvas_texture->update(canvas_image);
	}
}

void EditorAsepriteWorkspace::_save_undo_state() {
	WorkspaceUndoState state;
	state.width = canvas_width;
	state.height = canvas_height;
	state.page_index = current_page_index;
	for (int i = 0; i < pages.size(); ++i) {
		if (pages[i].is_valid()) {
			state.pages_backup.push_back(pages[i]->duplicate());
		}
	}
	undo_stack.push_back(state);
	if (undo_stack.size() > 50) {
		undo_stack.remove_at(0);
	}
	redo_stack.clear();
}

void EditorAsepriteWorkspace::_undo() {
	if (undo_stack.is_empty()) {
		if (status_label) status_label->set_text(" Nothing to Undo ");
		return;
	}

	WorkspaceUndoState curr;
	curr.width = canvas_width;
	curr.height = canvas_height;
	curr.page_index = current_page_index;
	for (int i = 0; i < pages.size(); ++i) {
		if (pages[i].is_valid()) {
			curr.pages_backup.push_back(pages[i]->duplicate());
		}
	}
	redo_stack.push_back(curr);

	WorkspaceUndoState prev = undo_stack[undo_stack.size() - 1];
	undo_stack.remove_at(undo_stack.size() - 1);

	canvas_width = prev.width;
	canvas_height = prev.height;
	pages.clear();
	for (int i = 0; i < prev.pages_backup.size(); ++i) {
		pages.push_back(prev.pages_backup[i]->duplicate());
	}
	current_page_index = CLAMP(prev.page_index, 0, (int)pages.size() - 1);
	canvas_image = pages[current_page_index];

	if (canvas_w_spin) canvas_w_spin->set_value(canvas_width);
	if (canvas_h_spin) canvas_h_spin->set_value(canvas_height);
	if (canvas_size_label) canvas_size_label->set_text(vformat("Canvas: %dx%d", canvas_width, canvas_height));

	_update_canvas_rect_size();
	update_canvas_texture();
	_update_pages_ui();
	if (status_label) status_label->set_text(" Undo ");
}

void EditorAsepriteWorkspace::_redo() {
	if (redo_stack.is_empty()) {
		if (status_label) status_label->set_text(" Nothing to Redo ");
		return;
	}

	WorkspaceUndoState curr;
	curr.width = canvas_width;
	curr.height = canvas_height;
	curr.page_index = current_page_index;
	for (int i = 0; i < pages.size(); ++i) {
		if (pages[i].is_valid()) {
			curr.pages_backup.push_back(pages[i]->duplicate());
		}
	}
	undo_stack.push_back(curr);

	WorkspaceUndoState next = redo_stack[redo_stack.size() - 1];
	redo_stack.remove_at(redo_stack.size() - 1);

	canvas_width = next.width;
	canvas_height = next.height;
	pages.clear();
	for (int i = 0; i < next.pages_backup.size(); ++i) {
		pages.push_back(next.pages_backup[i]->duplicate());
	}
	current_page_index = CLAMP(next.page_index, 0, (int)pages.size() - 1);
	canvas_image = pages[current_page_index];

	if (canvas_w_spin) canvas_w_spin->set_value(canvas_width);
	if (canvas_h_spin) canvas_h_spin->set_value(canvas_height);
	if (canvas_size_label) canvas_size_label->set_text(vformat("Canvas: %dx%d", canvas_width, canvas_height));

	_update_canvas_rect_size();
	update_canvas_texture();
	_update_pages_ui();
	if (status_label) status_label->set_text(" Redo ");
}

void EditorAsepriteWorkspace::unhandled_key_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventKey> k = p_event;
	if (k.is_valid() && k->is_pressed() && !k->is_echo()) {
		if (k->is_command_or_control_pressed()) {
			if (k->get_keycode() == Key::Z) {
				if (k->is_shift_pressed()) {
					_redo();
				} else {
					_undo();
				}
				return;
			} else if (k->get_keycode() == Key::Y) {
				_redo();
				return;
			}
		} else if (!k->is_alt_pressed()) {
			if (k->get_keycode() == Key::P) {
				_on_tool_selected(TOOL_PENCIL);
			} else if (k->get_keycode() == Key::E) {
				_on_tool_selected(TOOL_ERASER);
			} else if (k->get_keycode() == Key::I) {
				_on_tool_selected(TOOL_PICKER);
			} else if (k->get_keycode() == Key::G) {
				_on_tool_selected(TOOL_FILL);
			}
		}
	}
}

void EditorAsepriteWorkspace::_gui_input_canvas(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> mb = p_event;
	Ref<InputEventMouseMotion> mm = p_event;

	if (mb.is_valid() && mb->get_button_index() == MouseButton::LEFT) {
		if (mb->is_pressed()) {
			_save_undo_state();
			is_drawing = true;
			Vector2 local_pos = canvas_texture_rect->get_local_mouse_position();
			int px = Math::floor(local_pos.x / zoom_level);
			int py = Math::floor(local_pos.y / zoom_level);
			px = CLAMP(px, 0, canvas_width - 1);
			py = CLAMP(py, 0, canvas_height - 1);

			_apply_tool_at(px, py);
			last_mouse_px = Vector2i(px, py);
			update_canvas_texture();
		} else {
			is_drawing = false;
			last_mouse_px = Vector2i(-1, -1);
		}
	} else if (mm.is_valid()) {
		Vector2 local_pos = canvas_texture_rect->get_local_mouse_position();
		int px = Math::floor(local_pos.x / zoom_level);
		int py = Math::floor(local_pos.y / zoom_level);

		if (px >= 0 && px < canvas_width && py >= 0 && py < canvas_height) {
			cursor_pos_label->set_text(vformat("Cursor: (%d, %d)", px, py));
		}

		if (is_drawing && mm->get_button_mask().has_flag(MouseButtonMask::LEFT)) {
			px = CLAMP(px, 0, canvas_width - 1);
			py = CLAMP(py, 0, canvas_height - 1);

			if (last_mouse_px.x >= 0 && last_mouse_px.y >= 0) {
				_draw_line_brush(last_mouse_px.x, last_mouse_px.y, px, py, current_color);
			} else {
				_apply_tool_at(px, py);
			}
			last_mouse_px = Vector2i(px, py);
			update_canvas_texture();
		}
	}
}

void EditorAsepriteWorkspace::_apply_tool_at(int p_x, int p_y) {
	if (p_x < 0 || p_x >= canvas_width || p_y < 0 || p_y >= canvas_height) return;

	if (current_tool == TOOL_PENCIL) {
		_draw_brush_at(p_x, p_y, current_color);
	} else if (current_tool == TOOL_ERASER) {
		_draw_brush_at(p_x, p_y, Color(0, 0, 0, 0));
	} else if (current_tool == TOOL_PICKER) {
		current_color = canvas_image->get_pixel(p_x, p_y);
		color_picker_btn->set_pick_color(current_color);
	} else if (current_tool == TOOL_FILL) {
		Color target = canvas_image->get_pixel(p_x, p_y);
		_flood_fill(p_x, p_y, target, current_color);
	}
}

void EditorAsepriteWorkspace::_draw_line_brush(int p_x0, int p_y0, int p_x1, int p_y1, Color p_color) {
	int dx = Math::abs(p_x1 - p_x0);
	int dy = Math::abs(p_y1 - p_y0);
	int sx = (p_x0 < p_x1) ? 1 : -1;
	int sy = (p_y0 < p_y1) ? 1 : -1;
	int err = dx - dy;

	int x = p_x0;
	int y = p_y0;

	while (true) {
		_apply_tool_at(x, y);
		if (x == p_x1 && y == p_y1) break;
		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x += sx;
		}
		if (e2 < dx) {
			err += dx;
			y += sy;
		}
	}
}

void EditorAsepriteWorkspace::_draw_brush_at(int p_x, int p_y, Color p_color) {
	if (!canvas_image.is_valid()) return;

	int size = (int)brush_size_spin->get_value();
	for (int dx = 0; dx < size; ++dx) {
		for (int dy = 0; dy < size; ++dy) {
			int target_x = p_x + dx;
			int target_y = p_y + dy;
			if (target_x >= 0 && target_x < canvas_width && target_y >= 0 && target_y < canvas_height) {
				canvas_image->set_pixel(target_x, target_y, p_color);
			}
		}
	}
}

void EditorAsepriteWorkspace::_flood_fill(int p_x, int p_y, Color p_target, Color p_replacement) {
	if (p_target == p_replacement) return;
	if (p_x < 0 || p_x >= canvas_width || p_y < 0 || p_y >= canvas_height) return;
	if (canvas_image->get_pixel(p_x, p_y) != p_target) return;

	canvas_image->set_pixel(p_x, p_y, p_replacement);

	_flood_fill(p_x + 1, p_y, p_target, p_replacement);
	_flood_fill(p_x - 1, p_y, p_target, p_replacement);
	_flood_fill(p_x, p_y + 1, p_target, p_replacement);
	_flood_fill(p_x, p_y - 1, p_target, p_replacement);
}

void EditorAsepriteWorkspace::_update_tool_buttons() {
	btn_pencil->set_flat(current_tool != TOOL_PENCIL);
	btn_eraser->set_flat(current_tool != TOOL_ERASER);
	btn_picker->set_flat(current_tool != TOOL_PICKER);
	btn_fill->set_flat(current_tool != TOOL_FILL);
}

void EditorAsepriteWorkspace::_on_tool_selected(int p_tool_id) {
	current_tool = (ToolType)p_tool_id;
	_update_tool_buttons();
}

void EditorAsepriteWorkspace::_on_preset_selected(int p_index) {
	PMDoTPixelArtPresets::apply_preset((PMDoTPixelArtPresets::PresetResolution)p_index);
	if (status_label) status_label->set_text(" PMDoT Preset Applied ");
}

void EditorAsepriteWorkspace::_on_quick_size_selected(int p_index) {
	switch (p_index) {
		case 1: canvas_w_spin->set_value(16); canvas_h_spin->set_value(16); break;
		case 2: canvas_w_spin->set_value(32); canvas_h_spin->set_value(32); break;
		case 3: canvas_w_spin->set_value(48); canvas_h_spin->set_value(48); break;
		case 4: canvas_w_spin->set_value(64); canvas_h_spin->set_value(64); break;
		case 5: canvas_w_spin->set_value(128); canvas_h_spin->set_value(128); break;
		case 6: canvas_w_spin->set_value(256); canvas_h_spin->set_value(256); break;
		default: break;
	}
}

void EditorAsepriteWorkspace::_on_add_page_pressed() {
	_save_undo_state();

	Ref<Image> new_page = Image::create_empty(canvas_width, canvas_height, false, Image::FORMAT_RGBA8);
	new_page->fill(Color(0, 0, 0, 0));
	pages.push_back(new_page);

	current_page_index = pages.size() - 1;
	canvas_image = pages[current_page_index];

	update_canvas_texture();
	_update_pages_ui();
	if (status_label) status_label->set_text(vformat(" Created Page %d ", current_page_index + 1));
}

void EditorAsepriteWorkspace::_on_duplicate_page_pressed() {
	if (!canvas_image.is_valid()) return;
	_save_undo_state();

	Ref<Image> dup_page = canvas_image->duplicate();
	pages.push_back(dup_page);

	current_page_index = pages.size() - 1;
	canvas_image = pages[current_page_index];

	update_canvas_texture();
	_update_pages_ui();
	if (status_label) status_label->set_text(vformat(" Duplicated Page %d ", current_page_index + 1));
}

void EditorAsepriteWorkspace::_on_delete_page_pressed() {
	if (pages.size() <= 1) {
		if (status_label) status_label->set_text(" Cannot delete sole remaining page ");
		return;
	}

	_save_undo_state();
	pages.remove_at(current_page_index);
	if (current_page_index >= pages.size()) {
		current_page_index = pages.size() - 1;
	}
	canvas_image = pages[current_page_index];

	update_canvas_texture();
	_update_pages_ui();
	if (status_label) status_label->set_text(vformat(" Page deleted (Active: %d) ", current_page_index + 1));
}

void EditorAsepriteWorkspace::_select_page(int p_index) {
	if (p_index >= 0 && p_index < pages.size()) {
		current_page_index = p_index;
		canvas_image = pages[current_page_index];
		update_canvas_texture();
		_update_pages_ui();
	}
}

void EditorAsepriteWorkspace::_update_pages_ui() {
	if (!page_buttons_hb) return;

	while (page_buttons_hb->get_child_count() > 0) {
		Node *child = page_buttons_hb->get_child(0);
		page_buttons_hb->remove_child(child);
		child->queue_free();
	}

	for (int i = 0; i < pages.size(); ++i) {
		Button *pb = memnew(Button);
		pb->set_text(vformat(" Page %d ", i + 1));
		if (i == current_page_index) {
			pb->set_flat(false);
		} else {
			pb->set_flat(true);
		}
		page_buttons_hb->add_child(pb);
		pb->connect("pressed", callable_mp(this, &EditorAsepriteWorkspace::_select_page).bind(i));
	}
}

void EditorAsepriteWorkspace::_on_browse_dir_pressed() {
#ifdef TOOLS_ENABLED
	if (!export_file_dialog) {
		export_file_dialog = memnew(EditorFileDialog);
		export_file_dialog->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_DIR);
		export_file_dialog->set_access(EditorFileDialog::ACCESS_RESOURCES);
		export_file_dialog->connect("dir_selected", callable_mp(this, &EditorAsepriteWorkspace::_on_export_dir_selected));
		add_child(export_file_dialog);
	}
	export_file_dialog->popup_file_dialog();
#endif
}

void EditorAsepriteWorkspace::_on_export_dir_selected(const String &p_dir) {
	if (export_dir_input) {
		String formatted = p_dir;
		if (!formatted.ends_with("/")) {
			formatted += "/";
		}
		export_dir_input->set_text(formatted);
	}
}

void EditorAsepriteWorkspace::_resize_canvas(int p_new_width, int p_new_height, bool p_scale) {
	if (p_new_width <= 0 || p_new_height <= 0) return;
	_save_undo_state();

	canvas_width = p_new_width;
	canvas_height = p_new_height;

	for (int i = 0; i < pages.size(); ++i) {
		if (pages[i].is_valid()) {
			if (p_scale) {
				pages.write[i]->resize(canvas_width, canvas_height, Image::INTERPOLATE_NEAREST);
			} else {
				Ref<Image> new_img = Image::create_empty(canvas_width, canvas_height, false, Image::FORMAT_RGBA8);
				new_img->fill(Color(0, 0, 0, 0));
				new_img->blit_rect(pages[i], Rect2i(0, 0, MIN(pages[i]->get_width(), canvas_width), MIN(pages[i]->get_height(), canvas_height)), Vector2i(0, 0));
				pages.write[i] = new_img;
			}
		}
	}

	if (current_page_index >= 0 && current_page_index < pages.size()) {
		canvas_image = pages[current_page_index];
	}

	if (canvas_w_spin) canvas_w_spin->set_value(canvas_width);
	if (canvas_h_spin) canvas_h_spin->set_value(canvas_height);
	if (canvas_size_label) canvas_size_label->set_text(vformat("Canvas: %dx%d", canvas_width, canvas_height));

	_update_canvas_rect_size();
	update_canvas_texture();
	if (status_label) status_label->set_text(vformat(" Canvas Resized (%dx%d, Ctrl+Z to Undo) ", canvas_width, canvas_height));
}

void EditorAsepriteWorkspace::_on_resize_pressed() {
	int w = (int)canvas_w_spin->get_value();
	int h = (int)canvas_h_spin->get_value();
	_resize_canvas(w, h, true);
}

void EditorAsepriteWorkspace::_on_new_canvas_pressed() {
	int w = (int)canvas_w_spin->get_value();
	int h = (int)canvas_h_spin->get_value();
	_save_undo_state();
	initialize_workspace(w, h);
	if (status_label) status_label->set_text(vformat(" New Canvas Created (%dx%d) ", w, h));
}

void EditorAsepriteWorkspace::_on_export_pressed() {
	if (!canvas_image.is_valid()) return;

	String sname = sprite_name_input->get_text().strip_edges();
	if (sname.is_empty()) sname = "sprite_01";
	if (sname.ends_with(".png")) sname = sname.left(sname.length() - 4);

	String dir_path = export_dir_input ? export_dir_input->get_text().strip_edges() : "res://assets/sprites/";
	if (!dir_path.ends_with("/")) dir_path += "/";

	String base_dir = dir_path.replace("res://", "");
	Ref<DirAccess> da = DirAccess::open("res://");
	if (da.is_valid()) {
		da->make_dir_recursive(base_dir);
	}

	String main_export_path = dir_path + sname + ".png";
	Error err = canvas_image->save_png(main_export_path);

	if (pages.size() > 1) {
		for (int i = 0; i < pages.size(); ++i) {
			if (pages[i].is_valid()) {
				String page_path = dir_path + sname + vformat("_page_%d.png", i + 1);
				pages[i]->save_png(page_path);
			}
		}
	}

	if (err == OK) {
		if (status_label) status_label->set_text(vformat(" Exported (%d pages) to %s ", pages.size(), main_export_path));
#ifdef TOOLS_ENABLED
		if (EditorFileSystem::get_singleton()) {
			EditorFileSystem::get_singleton()->scan();
		}
#endif
	} else {
		if (status_label) status_label->set_text(vformat(" Failed to export PNG (Error: %d) ", (int)err));
	}
}

void EditorAsepriteWorkspace::_on_clear_pressed() {
	if (canvas_image.is_valid()) {
		_save_undo_state();
		canvas_image->fill(Color(0, 0, 0, 0));
		update_canvas_texture();
		if (status_label) status_label->set_text(" Canvas Cleared ");
	}
}

void EditorAsepriteWorkspace::_on_color_changed(const Color &p_color) {
	current_color = p_color;
}

void EditorAsepriteWorkspace::_on_palette_color_clicked(const Color &p_color) {
	current_color = p_color;
	color_picker_btn->set_pick_color(current_color);
}

void EditorAsepriteWorkspace::_on_zoom_in() {
	if (zoom_level < 32.0f) {
		zoom_level *= 2.0f;
		zoom_label->set_text(vformat(" %.0fx ", zoom_level));
		_update_canvas_rect_size();
	}
}

void EditorAsepriteWorkspace::_on_zoom_out() {
	if (zoom_level > 1.0f) {
		zoom_level /= 2.0f;
		zoom_label->set_text(vformat(" %.0fx ", zoom_level));
		_update_canvas_rect_size();
	}
}

void EditorAsepriteWorkspace::_on_zoom_reset() {
	zoom_level = 8.0f;
	zoom_label->set_text(" 8x ");
	_update_canvas_rect_size();
}

#ifdef TOOLS_ENABLED
void AsepriteEditorPlugin::make_visible(bool p_visible) {
	if (aseprite_workspace) {
		if (p_visible) {
			aseprite_workspace->show();
		} else {
			aseprite_workspace->hide();
		}
	}
}

AsepriteEditorPlugin::AsepriteEditorPlugin() {
	aseprite_workspace = memnew(EditorAsepriteWorkspace);
	aseprite_workspace->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	EditorNode::get_singleton()->get_main_screen_control()->add_child(aseprite_workspace);
	aseprite_workspace->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	aseprite_workspace->hide();
}

AsepriteEditorPlugin::~AsepriteEditorPlugin() {
}
#endif
