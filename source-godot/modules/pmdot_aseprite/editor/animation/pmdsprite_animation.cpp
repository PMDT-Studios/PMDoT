#include "pmdsprite_animation.h"
#include "core/math/math_funcs.h"

void PmdspriteAnimation::start(int p_fps) {
	frame_duration_ms = 1000.0 / p_fps;
	playing = true;
}

void PmdspriteAnimation::stop() {
	playing = false;
}

bool PmdspriteAnimation::advance(double p_delta_sec, int p_page_count, int &r_index) {
	if (!playing || p_page_count <= 1 || frame_duration_ms <= 0.0) {
		return false;
	}

	const double delta_ms = MIN(p_delta_sec * 1000.0, 250.0);
	time_accumulator += delta_ms;

	if (time_accumulator >= frame_duration_ms) {
		const int steps = static_cast<int>(time_accumulator / frame_duration_ms);
		time_accumulator = Math::fmod(time_accumulator, frame_duration_ms);

		animation_index = (animation_index + steps) % p_page_count;
		r_index = animation_index;
		return true;
	}
	return false;
}