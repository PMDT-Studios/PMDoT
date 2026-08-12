#include "pmdot_pixel_pipeline.h"
#include "core/object/class_db.h"

#include <cstring>
#include <algorithm>

#if defined(__AVX2__) || defined(__SSE4_1__) || defined(_M_AMD64) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#include <immintrin.h>
#include <tmmintrin.h>
#define PMDOT_HAS_SIMD 1
#else
#define PMDOT_HAS_SIMD 0
#endif

PMDoTPixelPipeline::PMDoTPixelPipeline() {}
PMDoTPixelPipeline::~PMDoTPixelPipeline() {}

void PMDoTPixelPipeline::_bind_methods() {
	ClassDB::bind_static_method("PMDoTPixelPipeline", D_METHOD("process_image_palette_swap", "image", "old_palette", "new_palette"), &PMDoTPixelPipeline::process_image_palette_swap);
}

void PMDoTPixelPipeline::convert_indexed_to_rgba_simd(
	const uint8_t *p_indexed_indices,
	const uint32_t *p_palette_rgba,
	uint32_t *p_out_rgba,
	size_t p_pixel_count
) {
	size_t i = 0;

#if PMDOT_HAS_SIMD && defined(__AVX2__)
	// AVX2 8-element vector gather expansion
	for (; i + 7 < p_pixel_count; i += 8) {
		__m128i idx8 = _mm_loadl_epi64((const __m128i *)(p_indexed_indices + i));
		__m256i idx32 = _mm256_cvtepu8_epi32(idx8);
		__m256i colors = _mm256_i32gather_epi32((const int *)p_palette_rgba, idx32, 4);
		_mm256_storeu_si256((__m256i *)(p_out_rgba + i), colors);
	}
#endif

	// Scalar loop for remaining pixels or fallback
	for (; i < p_pixel_count; ++i) {
		p_out_rgba[i] = p_palette_rgba[p_indexed_indices[i]];
	}
}

void PMDoTPixelPipeline::convert_bgra_to_rgba_simd(
	const uint8_t *p_bgra_pixels,
	uint8_t *p_rgba_pixels,
	size_t p_pixel_count
) {
	size_t i = 0;

#if PMDOT_HAS_SIMD
	// SSE4.1 16-byte (4-pixel) channel shuffle mask: BGRA -> RGBA swaps bytes 0 and 2
	const __m128i shuffle_mask = _mm_setr_epi8(
		2, 1, 0, 3,
		6, 5, 4, 7,
		10, 9, 8, 11,
		14, 13, 12, 15
	);

	size_t byte_count = p_pixel_count * 4;
	for (; i + 15 < byte_count; i += 16) {
		__m128i bgra = _mm_loadu_si128((const __m128i *)(p_bgra_pixels + i));
		__m128i rgba = _mm_shuffle_epi8(bgra, shuffle_mask);
		_mm_storeu_si128((__m128i *)(p_rgba_pixels + i), rgba);
	}
#endif

	for (; i < p_pixel_count * 4; i += 4) {
		p_rgba_pixels[i + 0] = p_bgra_pixels[i + 2]; // R
		p_rgba_pixels[i + 1] = p_bgra_pixels[i + 1]; // G
		p_rgba_pixels[i + 2] = p_bgra_pixels[i + 0]; // B
		p_rgba_pixels[i + 3] = p_bgra_pixels[i + 3]; // A
	}
}

void PMDoTPixelPipeline::premultiply_alpha_simd(
	uint8_t *p_rgba_pixels,
	size_t p_pixel_count
) {
	size_t i = 0;
	size_t byte_count = p_pixel_count * 4;

	for (; i < byte_count; i += 4) {
		uint32_t a = p_rgba_pixels[i + 3];
		if (a == 255) {
			continue;
		}
		if (a == 0) {
			p_rgba_pixels[i + 0] = 0;
			p_rgba_pixels[i + 1] = 0;
			p_rgba_pixels[i + 2] = 0;
			continue;
		}
		p_rgba_pixels[i + 0] = (uint8_t)((uint32_t)p_rgba_pixels[i + 0] * a / 255);
		p_rgba_pixels[i + 1] = (uint8_t)((uint32_t)p_rgba_pixels[i + 1] * a / 255);
		p_rgba_pixels[i + 2] = (uint8_t)((uint32_t)p_rgba_pixels[i + 2] * a / 255);
	}
}

void PMDoTPixelPipeline::blit_pixel_rect_simd(
	const uint32_t *p_src_rgba,
	int p_src_width,
	int p_src_height,
	uint32_t *p_dst_rgba,
	int p_dst_width,
	int p_dst_height,
	int p_dst_x,
	int p_dst_y
) {
	int start_x = std::max(0, p_dst_x);
	int start_y = std::max(0, p_dst_y);
	int end_x = std::min(p_dst_width, p_dst_x + p_src_width);
	int end_y = std::min(p_dst_height, p_dst_y + p_src_height);

	if (start_x >= end_x || start_y >= end_y) {
		return;
	}

	int copy_w = end_x - start_x;
	for (int y = start_y; y < end_y; ++y) {
		int src_y = y - p_dst_y;
		int src_offset = src_y * p_src_width + (start_x - p_dst_x);
		int dst_offset = y * p_dst_width + start_x;

		std::memcpy(p_dst_rgba + dst_offset, p_src_rgba + src_offset, copy_w * sizeof(uint32_t));
	}
}

void PMDoTPixelPipeline::process_image_palette_swap(
	Ref<Image> p_image,
	const PackedColorArray &p_old_palette,
	const PackedColorArray &p_new_palette
) {
	if (p_image.is_null() || p_image->is_empty()) {
		return;
	}

	int width = p_image->get_width();
	int height = p_image->get_height();
	int pixel_count = width * height;

	p_image->convert(Image::FORMAT_RGBA8);
	uint8_t *ptr = p_image->ptrw();
	if (!ptr) {
		return;
	}

	uint32_t *pixels = (uint32_t *)ptr;
	int palette_size = std::min(p_old_palette.size(), p_new_palette.size());

	for (int i = 0; i < pixel_count; ++i) {
		Color current_color = Color::hex(pixels[i]);
		for (int p = 0; p < palette_size; ++p) {
			if (current_color.is_equal_approx(p_old_palette[p])) {
				pixels[i] = p_new_palette[p].to_rgba32();
				break;
			}
		}
	}
}
