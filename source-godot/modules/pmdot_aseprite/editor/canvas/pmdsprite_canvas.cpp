#include "pmdsprite_canvas.h"
#include "core/input/input_event.h"

PmdspriteCanvas::PmdspriteCanvas() {
	_create_checkerboard_texture();

	set_h_size_flags(SIZE_EXPAND_FILL);
	set_v_size_flags(SIZE_EXPAND_FILL);

	canvas_center_container = memnew(CenterContainer);
	canvas_center_container->set_h_size_flags(SIZE_EXPAND_FILL);
	canvas_center_container->set_v_size_flags(SIZE_EXPAND_FILL);
	add_child(canvas_center_container);

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
	canvas_texture_rect->connect("gui_input", callable_mp(this, &PmdspriteCanvas::_on_canvas_gui_input));
}

PmdspriteCanvas::~PmdspriteCanvas() {}

void PmdspriteCanvas::_bind_methods() {
	ADD_SIGNAL(MethodInfo("pixel_pressed", PropertyInfo(Variant::VECTOR2I, "pos")));
	ADD_SIGNAL(MethodInfo("pixel_dragged", PropertyInfo(Variant::VECTOR2I, "from"), PropertyInfo(Variant::VECTOR2I, "to")));
	ADD_SIGNAL(MethodInfo("pixel_released"));
	ADD_SIGNAL(MethodInfo("pixel_hovered", PropertyInfo(Variant::VECTOR2I, "pos")));
	ADD_SIGNAL(MethodInfo("zoom_changed", PropertyInfo(Variant::FLOAT, "zoom")));
}

void PmdspriteCanvas::_create_checkerboard_texture() {
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

void PmdspriteCanvas::_update_canvas_rect_size() {
	Vector2 sz = Vector2(canvas_width * zoom_level, canvas_height * zoom_level);
	if (canvas_bg_texture_rect) {
		canvas_bg_texture_rect->set_custom_minimum_size(sz);
	}
	if (canvas_texture_rect) {
		canvas_texture_rect->set_custom_minimum_size(sz);
	}
}

void PmdspriteCanvas::update_canvas_texture() {
	if (canvas_image.is_valid() && canvas_texture.is_valid()) {
		canvas_texture->update(canvas_image);
	}
}

void PmdspriteCanvas::set_image(const Ref<Image> &p_image) {
	canvas_image = p_image;
	canvas_width = canvas_image->get_width();
	canvas_height = canvas_image->get_height();

	if (!canvas_texture.is_valid()) {
		canvas_texture = ImageTexture::create_from_image(canvas_image);
	} else {
		canvas_texture->set_image(canvas_image);
	}
	canvas_texture_rect->set_texture(canvas_texture);

	_update_canvas_rect_size();
}

Vector2i PmdspriteCanvas::_get_pixel_position(const Vector2 &p_local_position) const {
	return Vector2i(Math::floor(p_local_position.x / zoom_level), Math::floor(p_local_position.y / zoom_level));
}

void PmdspriteCanvas::_on_canvas_gui_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> mb = p_event;
	Ref<InputEventMouseMotion> mm = p_event;

	if (mb.is_valid()) {
		if (mb->get_button_index() == MouseButton::LEFT) {
			if (mb->is_pressed()) {
				is_drawing = true;
				Vector2i p = _get_pixel_position(canvas_texture_rect->get_local_mouse_position());
				p = Vector2i(CLAMP(p.x, 0, canvas_width - 1), CLAMP(p.y, 0, canvas_height - 1));
				last_mouse_px = p;
				emit_signal("pixel_pressed", p);
			} else {
				is_drawing = false;
				last_mouse_px = Vector2i(-1, -1);
				emit_signal("pixel_released");
			}
			return;
		}

		if (mb->is_pressed()) {
			if (mb->get_button_index() == MouseButton::WHEEL_UP) {
				zoom_in();
			} else if (mb->get_button_index() == MouseButton::WHEEL_DOWN) {
				zoom_out();
			}
		}
		return;
	}

	if (mm.is_valid()) {
		Vector2i p = _get_pixel_position(canvas_texture_rect->get_local_mouse_position());

		if (p.x >= 0 && p.x < canvas_width && p.y >= 0 && p.y < canvas_height) {
			emit_signal("pixel_hovered", p);
		}

		if (is_drawing && mm->get_button_mask().has_flag(MouseButtonMask::LEFT)) {
			p = Vector2i(CLAMP(p.x, 0, canvas_width - 1), CLAMP(p.y, 0, canvas_height - 1));
			Vector2i from = (last_mouse_px.x >= 0 && last_mouse_px.y >= 0) ? last_mouse_px : p;
			emit_signal("pixel_dragged", from, p);
			last_mouse_px = p;
		}
	}
}

void PmdspriteCanvas::zoom_in() {
	if (zoom_level < 32.0f) {
		zoom_level *= 2.0f;
		emit_signal("zoom_changed", zoom_level);
		_update_canvas_rect_size();
	}
}

void PmdspriteCanvas::zoom_out() {
	if (zoom_level > 1.0f) {
		zoom_level /= 2.0f;
		emit_signal("zoom_changed", zoom_level);
		_update_canvas_rect_size();
	}
}

void PmdspriteCanvas::zoom_reset() {
	zoom_level = 8.0f;
	emit_signal("zoom_changed", zoom_level);
	_update_canvas_rect_size();
}