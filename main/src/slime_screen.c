#include "stdarg.h"
#include "esp_check.h"
#include "slime_screen.h"

/**
 * @brief The log tag of the Slime Screen.
 */
static const char* TAG = "slime_screen_";

esp_err_t slime_screen_set_backlight(
	const slime_screen_context_t*	screen_context,
	const uint8_t					screen_backlight
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(screen_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t provided when performing setting backlight.");

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

	// Draw the masked rectangle using the fast LCD panel device rendering backend.
	ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_rectangle_masked(
		/* context			= */ screen_context->fast_lcd_panel_device,
		/* position_x		= */ position_x,
		/* position_y		= */ position_y,
		/* size_x			= */ size_x,
		/* size_y			= */ size_y,
		/* bitmask_offset_x	= */ bitmask_offset_x,
		/* bitmask_offset_y	= */ bitmask_offset_y,
		/* bitmask_size_x	= */ bitmask_info->bitmask_size_x,
		/* bitmask_flipped	= */ bitmask_info->bitmask_flipped,
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

	// Draw the masked rectangle using the fast LCD panel device rendering backend.
	ESP_RETURN_ON_ERROR(esp_fast_lcd_draw_native_rectangle_masked(
		/* context			= */ screen_context->fast_lcd_panel_device,
		/* position_x		= */ position_x,
		/* position_y		= */ position_y,
		/* size_x			= */ size_x,
		/* size_y			= */ size_y,
		/* bitmask_offset_x	= */ bitmask_offset_x,
		/* bitmask_offset_y	= */ bitmask_offset_y,
		/* bitmask_size_x	= */ bitmask_info->bitmask_size_x,
		/* bitmask_flipped	= */ bitmask_info->bitmask_flipped,
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

	switch (bitmap_info->bitmap_color_type) {
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
				/* bitmap_size_x			= */ bitmap_info->bitmap_size_x,
				/* bitmap_pre_multiplied	= */ bitmap_info->bitmap_optimized,
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
				/* bitmap_size_x			= */ bitmap_info->bitmap_size_x,
				/* bitmap_flipped			= */ bitmap_info->bitmap_optimized,
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
				/* bitmap_size_x		= */ bitmap_info->bitmap_size_x,
				/* bitmap_flipped		= */ bitmap_info->bitmap_optimized,
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

	switch (bitmap_info->bitmap_color_type) {
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
				/* bitmap_size_x			= */ bitmap_info->bitmap_size_x,
				/* bitmask_offset_x			= */ bitmask_offset_x,
				/* bitmask_offset_y			= */ bitmask_offset_y,
				/* bitmask_size_x			= */ bitmask_info->bitmask_size_x,
				/* bitmap_pre_multiplied	= */ bitmap_info->bitmap_optimized,
				/* bitmask_flipped			= */ bitmask_info->bitmask_flipped,
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
				/* bitmap_size_x			= */ bitmap_info->bitmap_size_x,
				/* bitmask_offset_x			= */ bitmask_offset_x,
				/* bitmask_offset_y			= */ bitmask_offset_y,
				/* bitmask_size_x			= */ bitmask_info->bitmask_size_x,
				/* bitmap_flipped			= */ bitmap_info->bitmap_optimized,
				/* bitmask_flipped			= */ bitmask_info->bitmask_flipped,
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
				/* bitmap_size_x		= */ bitmap_info->bitmap_size_x,
				/* bitmask_offset_x		= */ bitmask_offset_x,
				/* bitmask_offset_y		= */ bitmask_offset_y,
				/* bitmask_size_x		= */ bitmask_info->bitmask_size_x,
				/* bitmap_flipped		= */ bitmap_info->bitmap_optimized,
				/* bitmask_flipped		= */ bitmask_info->bitmask_flipped,
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

	// Draw outlined glyph if the style is outlined.
	if (text_style->text_outlined) {
		switch (text_style->text_color_type) {
			case RGBA8888:
				// Draw the translucent glyph if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_outlined_glyph(
					/* text_engine_context		= */ screen_context->fast_text_engine_instance,
					/* panel_device_context		= */ screen_context->fast_lcd_panel_device,
					/* codepoint				= */ codepoint,
					/* position_x				= */ position_x,
					/* position_y				= */ position_y,
					/* color_rgba8888			= */ text_style->text_color,
					/* color_outline_rgba8888	= */ text_style->text_outline_color,
					/* advance_x				= */ advance_x
				), TAG, "Failed to draw translucent outlined glyph on the screen.");
				break;
			case RGB565:
				// Draw the translucent glyph if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_outlined_glyph(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* codepoint			= */ codepoint,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgb565			= */ text_style->text_color,
					/* color_outline_rgb565	= */ text_style->text_outline_color,
					/* advance_x			= */ advance_x
				), TAG, "Failed to draw opaque native outlined glyph on the screen.");
				break;
			default:
				ESP_LOGE(TAG, "Unsupported bitmap color type.");
				return ESP_ERR_INVALID_ARG;
		}
	} else {
		switch (text_style->text_color_type) {
			case RGBA8888:
				// Draw the translucent glyph if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_glyph(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* codepoint			= */ codepoint,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgba8888		= */ text_style->text_color,
					/* advance_x			= */ advance_x
				), TAG, "Failed to draw translucent glyph on the screen.");
				break;
			case RGB565:
				// Draw the translucent glyph if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_glyph(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* codepoint			= */ codepoint,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgb565			= */ text_style->text_color,
					/* advance_x			= */ advance_x
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

	// Draw outlined glyph if the style is outlined.
	if (text_style->text_outlined) {
		switch (text_style->text_color_type) {
			case RGBA8888:
				// Draw the translucent string if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_outlined_string(
					/* text_engine_context		= */ screen_context->fast_text_engine_instance,
					/* panel_device_context		= */ screen_context->fast_lcd_panel_device,
					/* position_x				= */ position_x,
					/* position_y				= */ position_y,
					/* color_rgba8888			= */ text_style->text_color,
					/* color_outline_rgba8888	= */ text_style->text_outline_color,
					/* string					= */ string
				), TAG, "Failed to draw translucent outlined string on the screen.");
				break;
			case RGB565:
				// Draw the translucent string if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_outlined_string(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgb565			= */ text_style->text_color,
					/* color_outline_rgb565	= */ text_style->text_outline_color,
					/* string				= */ string
				), TAG, "Failed to draw opaque native outlined string on the screen.");
				break;
			default:
				ESP_LOGE(TAG, "Unsupported string color type.");
				return ESP_ERR_INVALID_ARG;
		}
	} else {
		switch (text_style->text_color_type) {
			case RGBA8888:
				// Draw the translucent string if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_string(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgba8888		= */ text_style->text_color,
					/* string				= */ string
				), TAG, "Failed to draw translucent string on the screen.");
				break;
			case RGB565:
				// Draw the translucent string if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_string(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgb565			= */ text_style->text_color,
					/* string				= */ string
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

	// Set up the variable length arguments.
	va_list args;
	va_start(args, string);

	// Draw outlined glyph if the style is outlined.
	if (text_style->text_outlined) {
		switch (text_style->text_color_type) {
			case RGBA8888:
				// Draw the translucent string if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_outlined_string_vfmt(
					/* text_engine_context		= */ screen_context->fast_text_engine_instance,
					/* panel_device_context		= */ screen_context->fast_lcd_panel_device,
					/* position_x				= */ position_x,
					/* position_y				= */ position_y,
					/* color_rgba8888			= */ text_style->text_color,
					/* color_outline_rgba8888	= */ text_style->text_outline_color,
					/* string					= */ string,
					/* args						= */ args
				), TAG, "Failed to draw translucent outlined string on the screen.");
				break;
			case RGB565:
				// Draw the translucent string if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_outlined_string_vfmt(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgb565			= */ text_style->text_color,
					/* color_outline_rgb565	= */ text_style->text_outline_color,
					/* string				= */ string,
					/* args					= */ args
				), TAG, "Failed to draw opaque native outlined string on the screen.");
				break;
			default:
				ESP_LOGE(TAG, "Unsupported string color type.");
				return ESP_ERR_INVALID_ARG;
		}
	} else {
		switch (text_style->text_color_type) {
			case RGBA8888:
				// Draw the translucent string if the color type is RGBA8888.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_string_vfmt(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgba8888		= */ text_style->text_color,
					/* string				= */ string,
					/* args					= */ args
				), TAG, "Failed to draw translucent string on the screen.");
				break;
			case RGB565:
				// Draw the translucent string if the color type is RGB565.
				ESP_RETURN_ON_ERROR(esp_fast_text_engine_draw_native_string_vfmt(
					/* text_engine_context	= */ screen_context->fast_text_engine_instance,
					/* panel_device_context	= */ screen_context->fast_lcd_panel_device,
					/* position_x			= */ position_x,
					/* position_y			= */ position_y,
					/* color_rgb565			= */ text_style->text_color,
					/* string				= */ string,
					/* args					= */ args
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

	// Commit changes to the ring buffer slot as pending frames.
	ESP_RETURN_ON_ERROR(esp_fast_lcd_commit(screen_context->fast_lcd_panel_device), TAG, "Failed to commit changes.");

	return ESP_OK;
}