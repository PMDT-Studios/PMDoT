#ifndef PMDSPRITE_CANVAS_H
#define PMDSPRITE_CANVAS_H

#include "core/io/image.h"
#include "scene/gui/center_container.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/texture_rect.h"
#include "scene/resources/image_texture.h"

class PmdspriteCanvas : public ScrollContainer {
	GDCLASS(PmdspriteCanvas, ScrollContainer);

	Ref<Image> canvas_image;
	Ref<ImageTexture> canvas_texture;
	Ref<ImageTexture> checkerboard_texture;

	float zoom_level = 8.0f;
	int canvas_width = 64;
	int canvas_height = 64;

	bool is_drawing = false;
	Vector2i last_mouse_px = Vector2i(-1, -1);

	CenterContainer *canvas_center_container = nullptr;
	PanelContainer *canvas_viewport_box = nullptr;
	TextureRect *canvas_bg_texture_rect = nullptr;
	TextureRect *canvas_texture_rect = nullptr;

	void _create_checkerboard_texture();
	void _update_canvas_rect_size();
	Vector2i _get_pixel_position(const Vector2 &p_local_position) const;
	void _on_canvas_gui_input(const Ref<InputEvent> &p_event);

protected:
	static void _bind_methods();

public:
	PmdspriteCanvas();
	~PmdspriteCanvas();

	void set_image(const Ref<Image> &p_image);
	void update_canvas_texture();

	void zoom_in();
	void zoom_out();
	void zoom_reset();
};

#endif