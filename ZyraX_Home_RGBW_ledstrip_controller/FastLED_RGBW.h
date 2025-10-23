/******************************************************************************/
/*
 * File:    FastLED_RGBW.h
 * Author:  Original code by Jim Bumgardner (http://krazydad.com).
 *          Modified by David Madison 
 * Version: 0.9.0
 * 
 * Brief:   Code to enable SK6812 RGBW strips to work with FastLED.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#ifndef FastLED_RGBW_H
#define FastLED_RGBW_H

struct CRGBW  {
	union {
		struct {
			union {
				uint8_t g;
				uint8_t green;
			};
			union {
				uint8_t r;
				uint8_t red;
			};
			union {
				uint8_t b;
				uint8_t blue;
			};
			union {
				uint8_t w;
				uint8_t white;
			};
		};
		uint8_t raw[4];
	};

	CRGBW() {}

	CRGBW (uint8_t rd, uint8_t grn, uint8_t blu, uint8_t wht) {
		r = rd;
		g = grn;
		b = blu;
		w = wht;
	}

	inline void operator = (const CRGB c) __attribute__((always_inline)) { 
		this->r = c.r;
		this->g = c.g;
		this->b = c.b;
		this->white = 0;
	}
};

inline uint16_t getRGBWsize(uint16_t nleds){
	uint16_t nbytes = nleds * 4;
	if(nbytes % 3 > 0) return nbytes / 3 + 1;
	else return nbytes / 3;
}

#endif