#include "pmdsprite_tools.h"

void PmdspriteTools::draw_brush_at(const Ref<Image> &p_image, int p_x, int p_y, Color p_color, int p_size) {
	if (!p_image.is_valid()) return;

	int w = p_image->get_width();
	int h = p_image->get_height();
	for (int dx = 0; dx < p_size; ++dx) {
		for (int dy = 0; dy < p_size; ++dy) {
			int target_x = p_x + dx;
			int target_y = p_y + dy;
			if (target_x >= 0 && target_x < w && target_y >= 0 && target_y < h) {
				p_image->set_pixel(target_x, target_y, p_color);
			}
		}
	}
}

void PmdspriteTools::flood_fill(const Ref<Image> &p_image, int p_x, int p_y, Color p_target, Color p_replacement) {
	if (p_target == p_replacement) return;
	if (p_x < 0 || p_x >= p_image->get_width() || p_y < 0 || p_y >= p_image->get_height()) return;
	if (p_image->get_pixel(p_x, p_y) != p_target) return;

	p_image->set_pixel(p_x, p_y, p_replacement);

	flood_fill(p_image, p_x + 1, p_y, p_target, p_replacement);
	flood_fill(p_image, p_x - 1, p_y, p_target, p_replacement);
	flood_fill(p_image, p_x, p_y + 1, p_target, p_replacement);
	flood_fill(p_image, p_x, p_y - 1, p_target, p_replacement);
}