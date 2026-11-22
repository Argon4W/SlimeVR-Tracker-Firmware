#include "stdarg.h"
#include "stdlib.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "slime_screen.h"

/**
 * @brief The log tag of the Slime Screen.
 */
static const char* TAG = "slime_screen";

/**
 * @brief The definition of the debug logging function of the Slime Screen.
 */
#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_SCREEN_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_SCREEN_LOG(format, ...)
#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

esp_err_t slime_screen_set_backlight(const slime_screen_context_t* screen_context, const uint8_t screen_backlight) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(screen_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing setting backlight.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is setting the backlight to %s", screen_backlight ? "on" : "off");

	// Set the backlight of the screen using the GPIO context in the LCD context of the screen context.
	ESP_RETURN_ON_ERROR(slime_gpio_backlight_set_level(screen_context->lcd_context->gpio_context, screen_backlight), TAG, "Failed to set the backlight of screen.");

	return ESP_OK;
}

esp_err_t slime_screen_draw_pixel(
	const slime_screen_context_t*	screen_context,
	const int32_t					position_x,
	const int32_t					position_y,
	const uint32_t					color_rgba8888
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(screen_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing drawing a pixel.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is drawing a pixel of RGBA8888 color 0x%08" PRIX32 " at (%" PRId32, ", %" PRId32 ")."
		/* PRIX32 */ color_rgba8888,
		/* PRId32 */ position_x,
		/* PRId32 */ position_y
	);

	// Draw the pixel using the fast LCD panel device rendering backend.
	ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_pixel(
		/* context			= */ screen_context->fast_lcd_panel_device,
		/* position_x		= */ position_x,
		/* position_y		= */ position_y,
		/* color_rgba8888	= */ color_rgba8888
	), TAG, "Failed to draw pixel on the screen.");

	return ESP_OK;
}

esp_err_t slime_screen_draw_rectangle(
	const slime_screen_context_t*	screen_context,
	const int32_t					position_x,
	const int32_t					position_y,
	const uint32_t					size_x,
	const uint32_t					size_y,
	const uint32_t					color_rgba8888
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(screen_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing drawing a rectangle.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is drawing a rectangle of size (%" PRId32 ", %" PRId32 ") in RGBA8888 color 0x%08" PRIX32 " at (%" PRId32, ", %" PRId32 ")."
		/* PRId32 */ size_x,
		/* PRId32 */ size_y,
		/* PRIX32 */ color_rgba8888,
		/* PRId32 */ position_x,
		/* PRId32 */ position_y
	);

	// Draw the rectangle using the fast LCD panel device rendering backend.
	ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_rectangle(
		/* context			= */ screen_context->fast_lcd_panel_device,
		/* position_x		= */ position_x,
		/* position_y		= */ position_y,
		/* size_x			= */ size_x,
		/* size_y			= */ size_y,
		/* color_rgba8888	= */ color_rgba8888
	), TAG, "Failed to draw pixel on the screen.");

	return ESP_OK;
}

esp_err_t slime_screen_draw_native_rectangle(
	const slime_screen_context_t*	screen_context,
	const int32_t					position_x,
	const int32_t					position_y,
	const uint32_t					size_x,
	const uint32_t					size_y,
	const uint16_t					color_rgb565
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(screen_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing drawing a rectangle.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is drawing a rectangle of size (%" PRId32 ", %" PRId32 ") in RGB565 color 0x%04" PRIX16 " at (%" PRId32, ", %" PRId32 ")."
		/* PRId32 */ size_x,
		/* PRId32 */ size_y,
		/* PRIX16 */ color_rgb565,
		/* PRId32 */ position_x,
		/* PRId32 */ position_y
	);

	// Draw the rectangle using the fast LCD panel device rendering backend.
	ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_native_rectangle(
		/* context		= */ screen_context->fast_lcd_panel_device,
		/* position_x	= */ position_x,
		/* position_y	= */ position_y,
		/* size_x		= */ size_x,
		/* size_y		= */ size_y,
		/* color_rgb565	= */ color_rgb565
	), TAG, "Failed to draw pixel on the screen.");

	return ESP_OK;
}

esp_err_t slime_screen_draw_rectangle_masked(
	const slime_screen_context_t*		screen_context,
	const slime_screen_bitmask_info_t*	bitmask_info,
	const int32_t						position_x,
	const int32_t						position_y,
	const uint32_t						size_x,
	const uint32_t						size_y,
	const uint32_t						bitmask_offset_x,
	const uint32_t						bitmask_offset_y,
	const uint32_t						color_rgba8888
) {
	// We cannot proceed without contexts.
	ESP_RETURN_ON_FALSE(screen_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing drawing an masked rectangle.");
	ESP_RETURN_ON_FALSE(bitmask_info	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_bitmask_info_t provided when performing drawing an masked rectangle.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is drawing a masked rectangle of size (%" PRId32 ", %" PRId32 ") in RGBA8888 color 0x%08" PRIX32 " at (%" PRId32, ", %" PRId32 ")."
		/* PRId32 */ size_x,
		/* PRId32 */ size_y,
		/* PRIX32 */ color_rgba8888,
		/* PRId32 */ position_x,
		/* PRId32 */ position_y
	);

	// Draw the masked rectangle using the fast LCD panel device rendering backend.
	ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_rectangle_masked(
		/* context			= */ screen_context->fast_lcd_panel_device,
		/* position_x		= */ position_x,
		/* position_y		= */ position_y,
		/* size_x			= */ size_x,
		/* size_y			= */ size_y,
		/* bitmask_offset_x	= */ bitmask_offset_x,
		/* bitmask_offset_y	= */ bitmask_offset_y,
		/* bitmask_size_x	= */ bitmask_info->data_size_x,
		/* bitmask_flipped	= */ bitmask_info->data_flipped,
		/* color_rgba8888	= */ color_rgba8888,
		/* bitmask_rgb565	= */ bitmask_info->bitmask_data
	), TAG, "Failed to draw pixel on the screen.");

	return ESP_OK;
}

esp_err_t slime_screen_draw_native_rectangle_masked(
	const slime_screen_context_t*		screen_context,
	const slime_screen_bitmask_info_t*	bitmask_info,
	const int32_t						position_x,
	const int32_t						position_y,
	const uint32_t						size_x,
	const uint32_t						size_y,
	const uint32_t						bitmask_offset_x,
	const uint32_t						bitmask_offset_y,
	const uint16_t						color_rgb565
) {
	// We cannot proceed without contexts.
	ESP_RETURN_ON_FALSE(screen_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing drawing an masked rectangle.");
	ESP_RETURN_ON_FALSE(bitmask_info	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_bitmask_info_t provided when performing drawing an masked rectangle.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is drawing a masked rectangle of size (%" PRId32 ", %" PRId32 ") in RGB565 color 0x%04" PRIX16 " at (%" PRId32, ", %" PRId32 ")."
		/* PRId32 */ size_x,
		/* PRId32 */ size_y,
		/* PRIX16 */ color_rgb565,
		/* PRId32 */ position_x,
		/* PRId32 */ position_y
	);

	// Draw the masked rectangle using the fast LCD panel device rendering backend.
	ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_native_rectangle_masked(
		/* context			= */ screen_context->fast_lcd_panel_device,
		/* position_x		= */ position_x,
		/* position_y		= */ position_y,
		/* size_x			= */ size_x,
		/* size_y			= */ size_y,
		/* bitmask_offset_x	= */ bitmask_offset_x,
		/* bitmask_offset_y	= */ bitmask_offset_y,
		/* bitmask_size_x	= */ bitmask_info->data_size_x,
		/* bitmask_flipped	= */ bitmask_info->data_flipped,
		/* color_rgb565		= */ color_rgb565,
		/* bitmask_rgb565	= */ bitmask_info->bitmask_data
	), TAG, "Failed to draw pixel on the screen.");

	return ESP_OK;
}

esp_err_t slime_screen_draw_bitmap(
	const slime_screen_context_t*		screen_context,
	const slime_screen_bitmap_info_t*	bitmap_info,
	const int32_t						position_x,
	const int32_t						position_y,
	const uint32_t						size_x,
	const uint32_t						size_y,
	const uint32_t						bitmap_offset_x,
	const uint32_t						bitmap_offset_y,
	const uint8_t						bitmap_a8_multiplier
) {
	// We cannot proceed without contexts.
	ESP_RETURN_ON_FALSE(screen_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing drawing a bitmap.");
	ESP_RETURN_ON_FALSE(bitmap_info		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_bitmap_info_t provided when performing drawing a bitmap.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is drawing a bitmap slice of size (%" PRId32 ", %" PRId32 ") from (%" PRIu32 ", %" PRIu32 ") of the bitmap 0x%" PRIXPTR " at (%" PRId32, ", %" PRId32 ")."
		/* PRId32	*/				size_x,
		/* PRId32	*/				size_y,
		/* PRIu32	*/				bitmap_offset_x,
		/* PRIu32	*/				bitmap_offset_y,
		/* PRIXPTR	*/ (uintptr_t)	bitmap_info
		/* PRId32	*/				position_x,
		/* PRId32	*/				position_y
	);

	switch (bitmap_info->data_color_type) {
		case RGBA8888:
			// Draw the translucent bitmap if the color type is RGBA8888.
			ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_bitmap(
				/* context					= */ screen_context->fast_lcd_panel_device,
				/* position_x				= */ position_x,
				/* position_y				= */ position_y,
				/* size_x					= */ size_x,
				/* size_y					= */ size_y,
				/* bitmap_offset_x			= */ bitmap_offset_x,
				/* bitmap_offset_y			= */ bitmap_offset_y,
				/* bitmap_size_x			= */ bitmap_info->data_size_x,
				/* bitmap_pre_multiplied	= */ bitmap_info->data_optimized,
				/* bitmap_a8_multiplier		= */ bitmap_a8_multiplier,
				/* bitmap_rgba8888			= */ bitmap_info->bitmap_data_0
			), TAG, "Failed to draw translucent bitmap on the screen.");
			break;
		case RGB565A8:
			// Draw the translucent native bitmap if the color type is RGB565A8.
			ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_bitmap_rgb565_pre_mul_a8_inv(
				/* context					= */ screen_context->fast_lcd_panel_device,
				/* position_x				= */ position_x,
				/* position_y				= */ position_y,
				/* size_x					= */ size_x,
				/* size_y					= */ size_y,
				/* bitmap_offset_x			= */ bitmap_offset_x,
				/* bitmap_offset_y			= */ bitmap_offset_y,
				/* bitmap_size_x			= */ bitmap_info->data_size_x,
				/* bitmap_flipped			= */ bitmap_info->data_optimized,
				/* bitmap_a8_multiplier		= */ bitmap_a8_multiplier,
				/* bitmap_rgb565_pre_mul	= */ bitmap_info->bitmap_data_0,
				/* bitmap_a8_inv			= */ bitmap_info->bitmap_data_1
			), TAG, "Failed to draw translucent native bitmap on the screen.");
			break;
		case RGB565:
			// Draw the opaque native bitmap if the color type is RGB565.
			ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_native_bitmap(
				/* context				= */ screen_context->fast_lcd_panel_device,
				/* position_x			= */ position_x,
				/* position_y			= */ position_y,
				/* size_x				= */ size_x,
				/* size_y				= */ size_y,
				/* bitmap_offset_x		= */ bitmap_offset_x,
				/* bitmap_offset_y		= */ bitmap_offset_y,
				/* bitmap_size_x		= */ bitmap_info->data_size_x,
				/* bitmap_flipped		= */ bitmap_info->data_optimized,
				/* bitmap_a8_multiplier	= */ bitmap_a8_multiplier,
				/* bitmap_rgb565		= */ bitmap_info->bitmap_data_0
			), TAG, "Failed to draw opaque native bitmap on the screen.");
			break;
		default:
			ESP_LOGE(TAG, "Unsupported bitmap color type.");
			return ESP_ERR_INVALID_ARG;
	}

	return ESP_OK;
}

esp_err_t slime_screen_draw_bitmap_masked(
	const slime_screen_context_t*		screen_context,
	const slime_screen_bitmap_info_t*	bitmap_info,
	const slime_screen_bitmask_info_t*	bitmask_info,
	const int32_t						position_x,
	const int32_t						position_y,
	const uint32_t						size_x,
	const uint32_t						size_y,
	const uint32_t						bitmap_offset_x,
	const uint32_t						bitmap_offset_y,
	const uint32_t						bitmask_offset_x,
	const uint32_t						bitmask_offset_y,
	const uint8_t						bitmap_a8_multiplier
) {
	// We cannot proceed without contexts.
	ESP_RETURN_ON_FALSE(screen_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing drawing a masked bitmap.");
	ESP_RETURN_ON_FALSE(bitmap_info		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_bitmap_info_t provided when performing drawing a masked bitmap.");
	ESP_RETURN_ON_FALSE(bitmask_info	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_bitmask_info_t provided when performing drawing a masked bitmap.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is drawing a massked bitmap slice of size (%" PRId32 ", %" PRId32 ") from (%" PRIu32 ", %" PRIu32 ") of the bitmap 0x%" PRIXPTR " at (%" PRId32, ", %" PRId32 ")."
		/* PRId32	*/				size_x,
		/* PRId32	*/				size_y,
		/* PRIu32	*/				bitmap_offset_x,
		/* PRIu32	*/				bitmap_offset_y,
		/* PRIXPTR	*/ (uintptr_t)	bitmap_info
		/* PRId32	*/				position_x,
		/* PRId32	*/				position_y
	);

	switch (bitmap_info->data_color_type) {
		case RGBA8888:
			// Draw the translucent bitmap if the color type is RGBA8888.
			ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_bitmap_masked(
				/* context					= */ screen_context->fast_lcd_panel_device,
				/* position_x				= */ position_x,
				/* position_y				= */ position_y,
				/* size_x					= */ size_x,
				/* size_y					= */ size_y,
				/* bitmap_offset_x			= */ bitmap_offset_x,
				/* bitmap_offset_y			= */ bitmap_offset_y,
				/* bitmap_size_x			= */ bitmap_info->data_size_x,
				/* bitmask_offset_x			= */ bitmask_offset_x,
				/* bitmask_offset_y			= */ bitmask_offset_y,
				/* bitmask_size_x			= */ bitmask_info->data_size_x,
				/* bitmap_pre_multiplied	= */ bitmap_info->data_optimized,
				/* bitmask_flipped			= */ bitmask_info->data_flipped,
				/* bitmap_a8_multiplier		= */ bitmap_a8_multiplier,
				/* bitmap_rgba8888			= */ bitmap_info->bitmap_data_0,
				/* bitmask_rgb565			= */ bitmask_info->bitmask_data
			), TAG, "Failed to draw translucent masked bitmap on the screen.");
			break;
		case RGB565A8:
			// Draw the translucent native bitmap if the color type is RGB565A8.
			ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_bitmap_rgb565_pre_mul_a8_inv_masked(
				/* context					= */ screen_context->fast_lcd_panel_device,
				/* position_x				= */ position_x,
				/* position_y				= */ position_y,
				/* size_x					= */ size_x,
				/* size_y					= */ size_y,
				/* bitmap_offset_x			= */ bitmap_offset_x,
				/* bitmap_offset_y			= */ bitmap_offset_y,
				/* bitmap_size_x			= */ bitmap_info->data_size_x,
				/* bitmask_offset_x			= */ bitmask_offset_x,
				/* bitmask_offset_y			= */ bitmask_offset_y,
				/* bitmask_size_x			= */ bitmask_info->data_size_x,
				/* bitmap_flipped			= */ bitmap_info->data_optimized,
				/* bitmask_flipped			= */ bitmask_info->data_flipped,
				/* bitmap_a8_multiplier		= */ bitmap_a8_multiplier,
				/* bitmap_rgb565_pre_mul	= */ bitmap_info->bitmap_data_0,
				/* bitmap_a8_inv			= */ bitmap_info->bitmap_data_1,
				/* bitmask_rgb565			= */ bitmask_info->bitmask_data
			), TAG, "Failed to draw translucent native masked bitmap on the screen.");
			break;
		case RGB565:
			// Draw the opaque native bitmap if the color type is RGB565.
			ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_native_bitmap_masked(
				/* context				= */ screen_context->fast_lcd_panel_device,
				/* position_x			= */ position_x,
				/* position_y			= */ position_y,
				/* size_x				= */ size_x,
				/* size_y				= */ size_y,
				/* bitmap_offset_x		= */ bitmap_offset_x,
				/* bitmap_offset_y		= */ bitmap_offset_y,
				/* bitmap_size_x		= */ bitmap_info->data_size_x,
				/* bitmask_offset_x		= */ bitmask_offset_x,
				/* bitmask_offset_y		= */ bitmask_offset_y,
				/* bitmask_size_x		= */ bitmask_info->data_size_x,
				/* bitmap_flipped		= */ bitmap_info->data_optimized,
				/* bitmask_flipped		= */ bitmask_info->data_flipped,
				/* bitmap_a8_multiplier	= */ bitmap_a8_multiplier,
				/* bitmap_rgb565		= */ bitmap_info->bitmap_data_0,
				/* bitmask_rgb565		= */ bitmask_info->bitmask_data
			), TAG, "Failed to draw opaque native masked bitmap on the screen.");
			break;
		default:
			ESP_LOGE(TAG, "Unsupported bitmap color type.");
			return ESP_ERR_INVALID_ARG;
	}

	return ESP_OK;
}

esp_err_t slime_screen_draw_glyph(
	const	slime_screen_context_t*		screen_context,
	const	slime_screen_text_style_t*	text_style,
	const	uint16_t					codepoint,
	const	int32_t						position_x,
	const	int32_t						position_y,
			int32_t*					advance_x
) {
	// We cannot proceed without contexts.
	ESP_RETURN_ON_FALSE(screen_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing drawing a glyph.");
	ESP_RETURN_ON_FALSE(text_style		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No text_style provided when performing drawing a glyph.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is drawing a glyph of codepoint 0x%04" PRIX16 " at (%" PRId32, ", %" PRId32 ")."
		/* PRIX16 */ codepoint,
		/* PRId32 */ position_x,
		/* PRId32 */ position_y
	);

	// Draw outlined glyph if the style is outlined.
	if (text_style->outlined) {
		switch (text_style->color_type) {
			case RGBA8888:
				// Draw the translucent glyph if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_outlined_glyph(
					/* text_engine_context		= */ screen_context->fast_text_engine_instance,
					/* panel_device_context		= */ screen_context->fast_lcd_panel_device,
					/* codepoint				= */ codepoint,
					/* position_x				= */ position_x,
					/* position_y				= */ position_y,
					/* color_rgba8888			= */ text_style->color_text,
					/* color_outline_rgba8888	= */ text_style->color_outline,
					/* advance_x				= */ advance_x
				), TAG, "Failed to draw translucent outlined glyph on the screen.");
				break;
			case RGB565:
				// Draw the translucent glyph if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_outlined_glyph(
					/* text_engine_context	= */			screen_context->fast_text_engine_instance,
					/* panel_device_context	= */			screen_context->fast_lcd_panel_device,
					/* codepoint			= */			codepoint,
					/* position_x			= */			position_x,
					/* position_y			= */			position_y,
					/* color_rgb565			= */ (uint16_t)	text_style->color_text,
					/* color_outline_rgb565	= */ (uint16_t)	text_style->color_outline,
					/* advance_x			= */			advance_x
				), TAG, "Failed to draw opaque native outlined glyph on the screen.");
				break;
			default:
				ESP_LOGE(TAG, "Unsupported bitmap color type.");
				return ESP_ERR_INVALID_ARG;
		}
	} else {
		switch (text_style->color_type) {
			case RGBA8888:
				// Draw the translucent glyph if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_glyph(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* codepoint			= */ codepoint,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgba8888		= */ text_style->color_text,
					/* advance_x			= */ advance_x
				), TAG, "Failed to draw translucent glyph on the screen.");
				break;
			case RGB565:
				// Draw the translucent glyph if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_glyph(
					/* text_engine_context	= */			screen_context->fast_text_engine_instance,
					/* panel_device_context	= */			screen_context->fast_lcd_panel_device,
					/* codepoint			= */			codepoint,
					/* position_x			= */			position_x,
					/* position_y			= */			position_y,
					/* color_rgb565			= */ (uint16_t)	text_style->color_text,
					/* advance_x			= */			advance_x
				), TAG, "Failed to draw opaque native glyph on the screen.");
				break;
			default:
				ESP_LOGE(TAG, "Unsupported glyph color type.");
				return ESP_ERR_INVALID_ARG;
		}
	}

	return ESP_OK;
}

esp_err_t slime_screen_draw_string(
	const	slime_screen_context_t*		screen_context,
	const	slime_screen_text_style_t*	text_style,
	const	int32_t						position_x,
	const	int32_t						position_y,
			char*						string
) {
	// We cannot proceed without contexts.
	ESP_RETURN_ON_FALSE(screen_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing drawing a string.");
	ESP_RETURN_ON_FALSE(text_style		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No text_style provided when performing drawing a string.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is drawing string \"%s\" at (%" PRId32, ", %" PRId32 ")."
		/* s		*/ string,
		/* PRId32	*/ position_x,
		/* PRId32	*/ position_y
	);

	// Draw outlined glyph if the style is outlined.
	if (text_style->outlined) {
		switch (text_style->color_type) {
			case RGBA8888:
				// Draw the translucent string if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_outlined_string(
					/* text_engine_context		= */ screen_context->fast_text_engine_instance,
					/* panel_device_context		= */ screen_context->fast_lcd_panel_device,
					/* position_x				= */ position_x,
					/* position_y				= */ position_y,
					/* color_rgba8888			= */ text_style->color_text,
					/* color_outline_rgba8888	= */ text_style->color_outline,
					/* string					= */ string
				), TAG, "Failed to draw translucent outlined string on the screen.");
				break;
			case RGB565:
				// Draw the translucent string if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_outlined_string(
					/* text_engine_context	= */			screen_context->fast_text_engine_instance,
					/* panel_device_context	= */			screen_context->fast_lcd_panel_device,
					/* position_x			= */			position_x,
					/* position_y			= */			position_y,
					/* color_rgb565			= */ (uint16_t)	text_style->color_text,
					/* color_outline_rgb565	= */ (uint16_t)	text_style->color_outline,
					/* string				= */			string
				), TAG, "Failed to draw opaque native outlined string on the screen.");
				break;
			default:
				ESP_LOGE(TAG, "Unsupported string color type.");
				return ESP_ERR_INVALID_ARG;
		}
	} else {
		switch (text_style->color_type) {
			case RGBA8888:
				// Draw the translucent string if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_string(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgba8888		= */ text_style->color_text,
					/* string				= */ string
				), TAG, "Failed to draw translucent string on the screen.");
				break;
			case RGB565:
				// Draw the translucent string if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_string(
					/* text_engine_context	= */			screen_context->fast_text_engine_instance,
					/* panel_device_context	= */			screen_context->fast_lcd_panel_device,
					/* position_x			= */			position_x,
					/* position_y			= */			position_y,
					/* color_rgb565			= */ (uint16_t)	text_style->color_text,
					/* string				= */			string
				), TAG, "Failed to draw opaque native string on the screen.");
				break;
			default:
				ESP_LOGE(TAG, "Unsupported string color type.");
				return ESP_ERR_INVALID_ARG;
		}
	}

	return ESP_OK;
}

esp_err_t slime_screen_draw_string_fmt(
	const	slime_screen_context_t*		screen_context,
	const	slime_screen_text_style_t*	text_style,
	const	int32_t						position_x,
	const	int32_t						position_y,
			char*						string,
			...
) {
	// We cannot proceed without contexts.
	ESP_RETURN_ON_FALSE(screen_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing drawing a formatted string.");
	ESP_RETURN_ON_FALSE(text_style		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No text_style provided when performing drawing a formatted string.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is drawing formatted string \"%s\" at (%" PRId32, ", %" PRId32 ")."
		/* s		*/ string,
		/* PRId32	*/ position_x,
		/* PRId32	*/ position_y
	);

	// Set up the variable length arguments.
	va_list args;
	va_start(args, string);

	// Draw outlined glyph if the style is outlined.
	if (text_style->outlined) {
		switch (text_style->color_type) {
			case RGBA8888:
				// Draw the translucent string if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_outlined_string_vfmt(
					/* text_engine_context		= */ screen_context->fast_text_engine_instance,
					/* panel_device_context		= */ screen_context->fast_lcd_panel_device,
					/* position_x				= */ position_x,
					/* position_y				= */ position_y,
					/* color_rgba8888			= */ text_style->color_text,
					/* color_outline_rgba8888	= */ text_style->color_outline,
					/* string					= */ string,
					/* args						= */ args
				), TAG, "Failed to draw translucent outlined string on the screen.");
				break;
			case RGB565:
				// Draw the translucent string if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_outlined_string_vfmt(
					/* text_engine_context	= */			screen_context->fast_text_engine_instance,
					/* panel_device_context	= */			screen_context->fast_lcd_panel_device,
					/* position_x			= */			position_x,
					/* position_y			= */			position_y,
					/* color_rgb565			= */ (uint16_t)	text_style->color_text,
					/* color_outline_rgb565	= */ (uint16_t)	text_style->color_outline,
					/* string				= */			string,
					/* args					= */			args
				), TAG, "Failed to draw opaque native outlined string on the screen.");
				break;
			default:
				// Clean up the variable length arguments.
				va_end(args);

				ESP_LOGE(TAG, "Unsupported string color type.");
				return ESP_ERR_INVALID_ARG;
		}
	} else {
		switch (text_style->color_type) {
			case RGBA8888:
				// Draw the translucent string if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_string_vfmt(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgba8888		= */ text_style->color_text,
					/* string				= */ string,
					/* args					= */ args
				), TAG, "Failed to draw translucent string on the screen.");
				break;
			case RGB565:
				// Draw the translucent string if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_string_vfmt(
					/* text_engine_context	= */			screen_context->fast_text_engine_instance,
					/* panel_device_context	= */			screen_context->fast_lcd_panel_device,
					/* position_x			= */			position_x,
					/* position_y			= */			position_y,
					/* color_rgb565			= */ (uint16_t)	text_style->color_text,
					/* string				= */			string,
					/* args					= */			args
				), TAG, "Failed to draw opaque native string on the screen.");
				break;
			default:
				// Clean up the variable length arguments.
				va_end(args);

				ESP_LOGE(TAG, "Unsupported string color type.");
				return ESP_ERR_INVALID_ARG;
		}
	}

	// Clean up the variable length arguments.
	va_end(args);

	return ESP_OK;
}

esp_err_t slime_screen_commit(const slime_screen_context_t* screen_context) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(screen_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing committing changes.");

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context is committing changes.");

	// Check if the transmission task of the screen context is still running.
	ESP_RETURN_ON_FALSE((xEventGroupGetBits(screen_context->transmit_task_events) & SLIME_SCREEN_STOPPED) == 0U, ESP_ERR_INVALID_STATE, TAG, "Transmission task of the screen context has already stopped.");

	// Commit changes to the ring buffer slot as pending frames.
	ESP_RETURN_ON_ERROR(esp_fast_lcd_commit(screen_context->fast_lcd_panel_device), TAG, "Failed to commit changes.");

	return ESP_OK;
}

/**
 * @brief			The transmission task function of the screen context.
 * @param parameter	The context of the screen to be transmitted, should always be slime_screen_context_t*.
 */
static void slime_screen_transmit_task(void* parameter);

esp_err_t slime_screen_context_new(
			slime_screen_context_t**		screen_context_out,
	const	slime_screen_context_config_t*	screen_context_config,
			slime_lcd_context_t*			lcd_context
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration, an LCD context, and a handle to receive the created screen context.
	ESP_RETURN_ON_FALSE(screen_context_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t handle provided when creating screen context.");
	ESP_RETURN_ON_FALSE(screen_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_config_t handle provided when creating screen context.");
	ESP_RETURN_ON_FALSE(lcd_context				!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_lcd_context_t handle provided when creating screen context.");

	// Log the progress.
	SLIME_SCREEN_LOG("Creating screen context.");
	SLIME_SCREEN_LOG("Reserving handles of screen context.");

	// Reserve the handles.
	slime_screen_context_t*				screen_context				= NULL;
	esp_fast_lcd_panel_device_t*		fast_lcd_panel_device		= NULL;
	esp_fast_text_engine_instance_t*	fast_text_engine_instance	= NULL;
	EventGroupHandle_t					transmit_task_event_group	= NULL;

	// Log the progress.
	SLIME_SCREEN_LOG("Creating fast LCD panel device.");

	// Create the fast LCD panel device.
	ESP_GOTO_ON_ERROR(esp_fast_lcd_new_lcd_panel_device(
		/* panel_device_ret				= */ &	fast_lcd_panel_device,
		/* panel_device_configuration	= */	screen_context_config	->fast_lcd_panel_config,
		/* panel_handle					= */	lcd_context				->panel_handle,
		/* panel_io						= */	lcd_context				->spi_io_handle
	), error, TAG, "Failed to create fast LCD panel device.");

	// Log the progress.
	SLIME_SCREEN_LOG("Creating fast text engine instance.");

	// Create the fast text engine instance.
	ESP_GOTO_ON_ERROR(esp_fast_text_engine_new_text_engine_instance(
		/* engine_instance_ret				= */ &	fast_text_engine_instance,
		/* engine_instance_configuration	= */	screen_context_config->fast_text_engine_instance_config,
		/* engine_instance_font				= */	screen_context_config->fast_text_engine_font
	), error, TAG, "Failed to create fast text engine instance.");

	// Log the progress.
	SLIME_SCREEN_LOG("Creating event group.");

	// Create the event group.
	transmit_task_event_group = xEventGroupCreate();

	// Check the allocation.
	ESP_GOTO_ON_FALSE(transmit_task_event_group != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create the event group.");

	// Clear all bits of the event group.
	xEventGroupClearBits(transmit_task_event_group,
			SLIME_SCREEN_STOP
		|	SLIME_SCREEN_STOPPED
	);

	// Log the progress.
	SLIME_SCREEN_LOG("Creating screen context struct.");

	// Create the screen context handle.
	screen_context = calloc(1U, sizeof(slime_screen_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(screen_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create the screen context struct.");

	// Log the progress.
	SLIME_SCREEN_LOG("Filling the screen context.");

	// Fill the screen context.
	screen_context->lcd_context					= lcd_context;
	screen_context->fast_lcd_panel_device		= fast_lcd_panel_device;
	screen_context->fast_text_engine_instance	= fast_text_engine_instance;
	screen_context->transmit_task_events		= transmit_task_event_group;
	screen_context->transmit_framerate			= screen_context_config->transmit_task_config.transmit_framerate;

	// Log the progress.
	SLIME_SCREEN_LOG("Creating transmission task.");

	// Create the transmission task.
	ESP_GOTO_ON_FALSE(xTaskCreatePinnedToCore(
		/* pxTaskCode		= */	slime_screen_transmit_task,
		/* pcName			= */	screen_context_config	->transmit_task_config.task_name,
		/* usStackDepth		= */	screen_context_config	->transmit_task_config.task_stack_depth,
		/* pvParameters		= */	screen_context,
		/* uxPriority		= */	screen_context_config	->transmit_task_config.task_priority,
		/* pxCreatedTask	= */ &	screen_context			->transmit_task_handle,
		/* xCoreID			= */	screen_context_config	->transmit_task_config.task_core_id
	) == pdPASS, ESP_ERR_INVALID_STATE, error, TAG, "Failed to create transmission task.");

	// Return the created screen context.
	*screen_context_out = screen_context;

	// Log the progress.
	SLIME_SCREEN_LOG("Screen context has been created.");

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error.
	SLIME_SCREEN_LOG("Error occurred: %s", esp_err_to_name(ret));
	SLIME_SCREEN_LOG("Cleaning up resources.");

	if (screen_context)								free											(screen_context);						// Cleanup the screen context struct.
	if (fast_text_engine_instance)	ESP_ERROR_CHECK(esp_fast_text_engine_del_text_engine_instance	(fast_text_engine_instance));	// Cleanup the text engine instance.
	if (fast_lcd_panel_device)		ESP_ERROR_CHECK(esp_fast_lcd_del_lcd_panel_device				(fast_lcd_panel_device));		// Cleanup the fast LCD panel device.

	return ret;
}

esp_err_t slime_screen_context_del(slime_screen_context_t* screen_context_in) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(screen_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t handle provided when releasing screen context.");

	// Log the progress.
	SLIME_SCREEN_LOG("Releasing screen context.");
	SLIME_SCREEN_LOG("Stopping transmission task.");

	// Mark that transmission task should stop.
	xEventGroupSetBits(screen_context_in->transmit_task_events, SLIME_SCREEN_STOP);

	// Wait until the transmission task is stopped.
	xEventGroupWaitBits(
		/* xEventGroup		= */ screen_context_in->transmit_task_events,
		/* uxBitsToWaitFor	= */ SLIME_SCREEN_STOPPED,
		/* xClearOnExit		= */ pdFALSE,
		/* xWaitForAllBits	= */ pdFALSE,
		/* xTicksToWait		= */ portMAX_DELAY
	);

	// Log the progress.
	SLIME_SCREEN_LOG("Deleting transmission task.");

	// Delete the transmission task and its event group.
	vTaskDelete			(screen_context_in->transmit_task_handle);
	vEventGroupDelete	(screen_context_in->transmit_task_events);

	// Log the progress.
	SLIME_SCREEN_LOG("Releasing rendering backends.");

	// Release the fast LCD panel device and fast text engine instance.
	ESP_RETURN_ON_ERROR(esp_fast_text_engine_del_text_engine_instance	(screen_context_in->fast_text_engine_instance),	TAG, "Failed to release fast text engine instance.");
	ESP_RETURN_ON_ERROR(esp_fast_lcd_del_lcd_panel_device				(screen_context_in->fast_lcd_panel_device),		TAG, "Failed to release fast LCD panel device.");

	// Log the progress.
	SLIME_SCREEN_LOG("Detaching all fields of screen context.");

	// Detach all fields.
	screen_context_in->lcd_context					= NULL;
	screen_context_in->fast_lcd_panel_device		= NULL;
	screen_context_in->fast_text_engine_instance	= NULL;
	screen_context_in->transmit_task_handle			= NULL;
	screen_context_in->transmit_framerate			= 0U;

	// Log the progress.
	SLIME_SCREEN_LOG("Releasing screen context struct.");

	// Free the screen context struct.
	free(screen_context_in);

	// Log the progress.
	SLIME_SCREEN_LOG("Screen context has been released.");

	return ESP_OK;
}

static void slime_screen_transmit_task(void* parameter) {
	esp_err_t ret = ESP_OK;

	// Cast the opaque parameter handle to screen context handle.
	const slime_screen_context_t* screen_context = (slime_screen_context_t*) parameter;

	// Calculate the wait period in FreeRTOS ticks.
	const TickType_t period = pdMS_TO_TICKS(1000U / screen_context->transmit_framerate);

	// Log the operation.
	SLIME_SCREEN_LOG("Screen context will start to transmit pending frames using fast LCD device \"%s\" (%" PRIu32 ", %" PRIu32 ") at %" PRIu32 " FPS (every %" PRIu32 " tick(s)).",
		/* S		*/ screen_context->fast_lcd_panel_device->properties->configuration.name,
		/* PRIu32	*/ screen_context->fast_lcd_panel_device->properties->configuration.frame_size_x,
		/* PRIu32	*/ screen_context->fast_lcd_panel_device->properties->configuration.frame_size_y,
		/* PRIu32	*/ screen_context->transmit_framerate,
		/* PRIu32	*/ period
	);

	// Reserve the last wake time in ticks.
	TickType_t last_wake_time = xTaskGetTickCount();

	// Transmit the data until stop event received.
	while ((xEventGroupGetBits(screen_context->transmit_task_events) & SLIME_SCREEN_STOP) == 0U) {
		// Delay before transmitting next pending frames.
		xTaskDelayUntil(&last_wake_time, period);

		// Log the operation.
		SLIME_SCREEN_LOG("Screen context is trying transmitting a pending frame.");

		// Transmit the pending frame.
		ESP_GOTO_ON_ERROR(esp_fast_lcd_transmit(screen_context->fast_lcd_panel_device), error, TAG, "Failed to transmit pending frame.");
	}

	// No error occurred, jump to stop.
	goto stop;

	// Error occurred, terminate the transmission task.
	error:

	// Log the error
	SLIME_SCREEN_LOG("Error occurred while transmitting pending frames: %s", esp_err_to_name(ret));
	SLIME_SCREEN_LOG("Terminating transmission task.");

	stop:

	// Set the transmission task stopped.
	xEventGroupSetBits(screen_context->transmit_task_events, SLIME_SCREEN_STOPPED);

	// Log the stop.
	SLIME_SCREEN_LOG("Transmission task has stopped.");
}