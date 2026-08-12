#ifndef PMDOT_PIXEL_PIPELINE_H
#define PMDOT_PIXEL_PIPELINE_H

#include "core/object/ref_counted.h"
#include "core/typedefs.h"
#include "core/io/image.h"

#include <cstdint>
#include <cstddef>

class PMDoTPixelPipeline : public RefCounted {
	GDCLASS(PMDoTPixelPipeline, RefCounted);

protected:
	static void _bind_methods();

public:
	PMDoTPixelPipeline();
	~PMDoTPixelPipeline();

	// Convert 8-bit indexed palette buffer into 32-bit RGBA buffer using SIMD acceleration
	static void convert_indexed_to_rgba_simd(
		const uint8_t *p_indexed_indices,
		const uint32_t *p_palette_rgba,
		uint32_t *p_out_rgba,
		size_t p_pixel_count
	);

	// Swizzle channel order from BGRA8888 to RGBA8888 in real time using SIMD
	static void convert_bgra_to_rgba_simd(
		const uint8_t *p_bgra_pixels,
		uint8_t *p_rgba_pixels,
		size_t p_pixel_count
	);

	// Fast SIMD alpha premultiplication on RGBA8888 buffer
	static void premultiply_alpha_simd(
		uint8_t *p_rgba_pixels,
		size_t p_pixel_count
	);

	// Real-time SIMD blit & blend of sub-rectangle pixel buffers (for sprite layer composite)
	static void blit_pixel_rect_simd(
		const uint32_t *p_src_rgba,
		int p_src_width,
		int p_src_height,
		uint32_t *p_dst_rgba,
		int p_dst_width,
		int p_dst_height,
		int p_dst_x,
		int p_dst_y
	);

	// Helper method to process a Godot Image directly
	static void process_image_palette_swap(
		Ref<Image> p_image,
		const PackedColorArray &p_old_palette,
		const PackedColorArray &p_new_palette
	);
};

#endif // PMDOT_PIXEL_PIPELINE_H
