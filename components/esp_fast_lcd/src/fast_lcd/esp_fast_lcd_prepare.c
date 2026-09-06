#include "esp_fast_lcd.h"
#include "esp_fast_lcd_common.h"

void esp_fast_lcd_prepare_rgba8888_bitmap_to_rgba8888_pre_mul(
	const	uint32_t*	bitmap_rgba8888_in,
			uint32_t*	bitmap_rgba8888__pre_mul_out,
	const	uint32_t	bitmap_size_x,
	const	uint32_t	bitmap_size_y
) {

}

void esp_fast_lcd_prepare_rgba8888_bitmap_to_rgb565_flipped(
	const	uint32_t*	bitmap_rgba8888_in,
			uint16_t*	bitmap_rgb565_flipped_out,
	const	uint32_t	bitmap_size_x,
	const	uint32_t	bitmap_size_y
) {

}

void esp_fast_lcd_prepare_rgba8888_bitmap_to_rgb565_masked(
	const	uint32_t*	bitmap_rgba8888_in,
			uint16_t*	bitmap_rgb565_flipped_out,
			uint16_t*	bitmap_mask_flipped_out,
	const	uint32_t	bitmap_size_x,
	const	uint32_t	bitmap_size_y,
	const	uint8_t		bitmap_alpha_threshold
) {

}

void esp_fast_lcd_prepare_rgba8888_bitmap_to_rgb565_pre_mul_a8_inv(
	const	uint32_t*	bitmap_rgba8888_in,
			uint16_t*	bitmap_rgb565_pre_mul_out,
			uint16_t*	bitmap_a8_inv_out,
	const	uint32_t	bitmap_size_x,
	const	uint32_t	bitmap_size_y
) {

}

void esp_fast_lcd_prepare_rgb565_bitmap_to_rgb565_flipped(
	const	uint16_t*	bitmap_rgb565_in,
			uint16_t*	bitmap_rgb565_flipped_out,
	const	uint32_t	bitmap_size_x,
	const	uint32_t	bitmap_size_y
) {

}