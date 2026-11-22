#ifndef PMDSPRITE_ANIMATION_H
#define PMDSPRITE_ANIMATION_H

class PmdspriteAnimation {
	bool playing = false;
	double frame_duration_ms = 0.0;
	double time_accumulator = 0.0;
	int animation_index = 0;

public:
	bool is_playing() const { return playing; }
	void start(int p_fps);
	void stop();
	bool advance(double p_delta_sec, int p_page_count, int &r_index);
};

#endif