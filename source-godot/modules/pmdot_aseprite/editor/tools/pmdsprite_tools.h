#ifndef PMDSPRITE_TOOLS_H
#define PMDSPRITE_TOOLS_H

#include "core/io/image.h"
#include "core/math/color.h"

class PmdspriteTools {
public:
	static void draw_brush_at(const Ref<Image> &p_image, int p_x, int p_y, Color p_color, int p_size);
	static void flood_fill(const Ref<Image> &p_image, int p_x, int p_y, Color p_target, Color p_replacement);
};

#endif