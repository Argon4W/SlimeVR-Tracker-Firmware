#ifndef SLIME_SCREEN_H
#define SLIME_SCREEN_H

#include "esp_check.h"
#include "esp_fast_lcd.h"
#include "esp_fast_text_engine.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "slime_lcd.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The configuration struct of the transmission task of the screen context.
 */
typedef struct {
	uint32_t transmit_task_framerate;	/*!< The framerate of the transmission task. */
	uint32_t transmit_task_core_id;		/*!< The core ID of the transmission task. */
	uint32_t transmit_task_stack_depth;	/*!< The stack depth of the transmission task. */
	uint32_t transmit_task_priority;	/*!< The priority of the transmission task. */
} slime_screen_transmit_task_config_t;

/**
 * @brief The configuration struct of the screen context.
 */
typedef struct {
	slime_screen_transmit_task_config_t				screen_transmit_task_config;				/*!< The transmission task configuration of the screen. */
	esp_fast_lcd_panel_configuration_t				screen_fast_lcd_panel_config;				/*!< The fast LCD panel device configuration of the screen. */
	esp_fast_text_engine_instance_configuration_t	screen_fast_text_engine_instance_config;	/*!< The fast text engine configuration of the screen. */
	esp_fast_text_engine_font_t						screen_fast_text_engine_font;				/*!< The font info used by the screen. */
} slime_screen_context_config_t;

/**
 * @brief The screen context struct.
 */
typedef struct {
	slime_lcd_context_t*				lcd_context;				/*!< The LCD context of the screen. */
	esp_fast_lcd_panel_device_t*		fast_lcd_panel_device;		/*!< The rendering backend of the screen. */
	esp_fast_text_engine_instance_t*	fast_text_engine_instance;	/*!< The text rendering engine of the screen. */
	TaskHandle_t						transmit_task_handle;		/*!< The task handle of the transmission task. */
	uint32_t							transmit_framerate;			/*!< The framerate of the transmission task in FPS (frames-per-second). */
} slime_screen_context_t;

/**
 * @brief The color types enum of the bitmap.
 */
typedef enum {
	RGBA8888,	/*!< This color type has 8-bit red at [31:24], 8-bit green at [23:16], 8-bit blue at [15:8], and 8-bit alpha at [7:0]. */
	RGB565,		/*!< This color type has 5-bit red at [15:11], 6-bit green at [10:5], 5-bit blue at [4:0]. */
	RGB565A8	/*!< This color type is same as RGB565 but has an extra 8-bit alpha bitmap for alpha component. */
} slime_screen_color_type_t;

/**
 * @brief The info struct of a bitmap.
 */
typedef struct {
	slime_screen_color_type_t	bitmap_color_type;	/*!< The color type of the bitmap. */
	uint8_t						bitmap_optimized;	/*!< True if the bitmap is optimized (pre-multiplied for RGBA8888, flipped for RGB565 and its variants). */
	uint32_t					bitmap_size_x;		/*!< The width of the bitmap in pixels. */
	void*						bitmap_data_0;		/*!< The first color data of the bitmap. */
	void*						bitmap_data_1;		/*!< The second color data of the bitmap (for RGB565A8). */
} slime_screen_bitmap_info_t;

/**
 * @brief The info struct of a bitmask.
 */
typedef struct {
	uint8_t		bitmask_flipped;	/*!< True if the bitmask is flipped (LSByte and MSByte swapped). */
	uint32_t	bitmask_size_x;		/*!< The width of the bitmask in pixels. */
	uint16_t*	bitmask_data;		/*!< The 16-bit mask data of the bitmask. */
} slime_screen_bitmask_info_t;

/**
 * @brief The info struct of a text style.
 */
typedef struct {
	slime_screen_color_type_t	text_color_type;	/*!< The color type of the style, RGB565A8 is not allowed. */
	uint32_t					text_color;			/*!< The color of the text. */
	uint32_t					text_outline_color;	/*!< The color of the outline ring of the text. */
	uint8_t						text_outlined;		/*!< True if the text of this style is outlined. */
} slime_screen_text_style_t;

/**
 * @brief					Control the backlight of the given screen.
 * @param screen_context	The context of the screen you want to control the backlight of.
 * @param screen_backlight	The status of the backlight ("1" = turn on backlight; "0" = turn off backlight).
 * @return					The status of controlling the backlight.
 */
esp_err_t slime_screen_set_backlight(
	const	slime_screen_context_t*	screen_context,
			uint8_t					screen_backlight
);

/**
 * @brief					Draw a pixel on the framebuffer of the given screen.
 * @param screen_context	The context of the screen to be drawn to.
 * @param position_x		The position X of the pixel to be drawn.
 * @param position_y		The position Y of the pixel to be drawn.
 * @param color_rgba8888	The color of the pixel to be drawn in RGBA 8888 format (MSB first).
 * @return					The status of the draw.
 */
esp_err_t slime_screen_draw_pixel(
	const	slime_screen_context_t*	screen_context,
			int32_t					position_x,
			int32_t					position_y,
			uint32_t				color_rgba8888
);

/**
 * @brief					Draw a filled rectangle on the framebuffer of the given screen.
 * @param screen_context	The context of the screen to be drawn to.
 * @param position_x		The top-left origin position X of the rectangle to be drawn.
 * @param position_y		The top-left origin position Y of the rectangle to be drawn.
 * @param size_x			The width of the rectangle, in pixels.
 * @param size_y			The height of the rectangle, in pixels.
 * @param color_rgba8888	The color of the rectangle to be drawn in RGBA 8888 format (MSB first).
 * @return					The status of the draw.
*/
esp_err_t slime_screen_draw_rectangle(
	const	slime_screen_context_t*	screen_context,
			int32_t					position_x,
			int32_t					position_y,
			uint32_t				size_x,
			uint32_t				size_y,
			uint32_t				color_rgba8888
);

/**
 * @brief					Draw a filled rectangle on the framebuffer of the given screen.
 * @param screen_context	The context of the screen to be drawn to.
 * @param position_x		The top-left origin position X of the rectangle to be drawn.
 * @param position_y		The top-left origin position Y of the rectangle to be drawn.
 * @param size_x			The width of the rectangle, in pixels.
 * @param size_y			The height of the rectangle, in pixels.
 * @param color_rgb565		The color of the rectangle to be drawn in RGB 565 format (MSB first).
 * @return					The status of the draw.
*/
esp_err_t slime_screen_draw_native_rectangle(
	const	slime_screen_context_t*	screen_context,
			int32_t					position_x,
			int32_t					position_y,
			uint32_t				size_x,
			uint32_t				size_y,
			uint16_t				color_rgb565
);

/**
 * @brief					Draw a filled bit-masked rectangle on the framebuffer of the given screen.
 * @param screen_context	The context of the screen to be drawn to.
 * @param bitmask_info		The info of the bitmask to be applied to the rectangle.
 * @param position_x		The top-left origin position X of the rectangle to be drawn.
 * @param position_y		The top-left origin position Y of the rectangle to be drawn.
 * @param size_x			The width of the rectangle, in pixels.
 * @param size_y			The height of the rectangle, in pixels.
 * @param bitmask_offset_x	The top-left origin position X of the bitmask slice on the bitmask, in pixels.
 * @param bitmask_offset_y	The top-left origin position Y of the bitmask slice on the bitmask, in pixels.
 * @param color_rgba8888	The color of the rectangle to be drawn in RGBA 8888 format (MSB first).
 * @return					The status of the draw.
*/
esp_err_t slime_screen_draw_rectangle_masked(
	const	slime_screen_context_t*			screen_context,
	const	slime_screen_bitmask_info_t*	bitmask_info,
			int32_t							position_x,
			int32_t							position_y,
			uint32_t						size_x,
			uint32_t						size_y,
			uint32_t						bitmask_offset_x,
			uint32_t						bitmask_offset_y,
			uint32_t						color_rgba8888
);

/**
 * @brief					Draw a filled bit-masked rectangle on the framebuffer of the given screen.
 * @param screen_context	The context of the screen to be drawn to.
 * @param bitmask_info		The info of the bitmask to be applied to the rectangle.
 * @param position_x		The top-left origin position X of the rectangle to be drawn.
 * @param position_y		The top-left origin position Y of the rectangle to be drawn.
 * @param size_x			The width of the rectangle, in pixels.
 * @param size_y			The height of the rectangle, in pixels.
 * @param bitmask_offset_x	The top-left origin position X of the bitmask slice on the bitmask, in pixels.
 * @param bitmask_offset_y	The top-left origin position Y of the bitmask slice on the bitmask, in pixels.
 * @param color_rgb565		The color of the rectangle to be drawn in RGB 565 format (MSB first).
 * @return					The status of the draw.
*/
esp_err_t slime_screen_draw_native_rectangle_masked(
	const	slime_screen_context_t*			screen_context,
	const	slime_screen_bitmask_info_t*	bitmask_info,
			int32_t							position_x,
			int32_t							position_y,
			uint32_t						size_x,
			uint32_t						size_y,
			uint32_t						bitmask_offset_x,
			uint32_t						bitmask_offset_y,
			uint16_t						color_rgb565
);

/**
 * @brief						Draw a slice of bitmap on the framebuffer of the given screen.
 * @param screen_context		The context of the screen to be drawn to.
 * @param bitmap_info			The info of the bitmap to be drawn.
 * @param position_x			The top-left origin position X of the bitmap slice to be drawn on the framebuffer.
 * @param position_y			The top-left origin position Y of the bitmap slice to be drawn on the framebuffer.
 * @param size_x				The width of the bitmap slice, in pixels.
 * @param size_y				The height of the bitmap slice, in pixels.
 * @param bitmap_offset_x		The top-left origin position X of the slice on the bitmap, in pixels.
 * @param bitmap_offset_y		The top-left origin position Y of the slice on the bitmap, in pixels.
 * @param bitmap_a8_multiplier	The 8-bit alpha multiplier to be applied on the draw of the bitmap.
 * @return						The status of the draw.
*/
esp_err_t slime_screen_draw_bitmap(
	const	slime_screen_context_t*		screen_context,
	const	slime_screen_bitmap_info_t*	bitmap_info,
			int32_t						position_x,
			int32_t						position_y,
			uint32_t					size_x,
			uint32_t					size_y,
			uint32_t					bitmap_offset_x,
			uint32_t					bitmap_offset_y,
			uint8_t						bitmap_a8_multiplier
);

/**
 * @brief						Draw a slice of bit-masked bitmap on the framebuffer of the given screen.
 * @param screen_context		The context of the screen to be drawn to.
 * @param bitmap_info			The info of the bitmap to be drawn.
 * @param bitmask_info			The info of the bitmask to be applied to the bitmap.
 * @param position_x			The top-left origin position X of the bitmap slice to be drawn on the framebuffer.
 * @param position_y			The top-left origin position Y of the bitmap slice to be drawn on the framebuffer.
 * @param size_x				The width of the bitmap slice, in pixels.
 * @param size_y				The height of the bitmap slice, in pixels.
 * @param bitmap_offset_x		The top-left origin position X of the slice on the bitmap, in pixels.
 * @param bitmap_offset_y		The top-left origin position Y of the slice on the bitmap, in pixels.
 * @param bitmask_offset_x		The top-left origin position X of the bitmask slice on the bitmask, in pixels.
 * @param bitmask_offset_y		The top-left origin position Y of the bitmask slice on the bitmask, in pixels.
 *								of a pixel is already pre-multiplied with the alpha of the pixel, and the alpha component
 *								of the pixel is replaced by the 255 - alpha.
 * @param bitmap_a8_multiplier	The 8-bit alpha multiplier to be applied on the draw of the bitmap.
 * @return						The status of the draw.
*/
esp_err_t slime_screen_draw_bitmap_masked(
	const	slime_screen_context_t*			screen_context,
	const	slime_screen_bitmap_info_t*		bitmap_info,
	const	slime_screen_bitmask_info_t*	bitmask_info,
			int32_t							position_x,
			int32_t							position_y,
			uint32_t						size_x,
			uint32_t						size_y,
			uint32_t						bitmap_offset_x,
			uint32_t						bitmap_offset_y,
			uint32_t						bitmask_offset_x,
			uint32_t						bitmask_offset_y,
			uint8_t							bitmap_a8_multiplier
);

/**
 * @brief					Draw a glyph on a given screen.
 * @param screen_context	The context of the screen to be drawn to.
 * @param text_style		The style of the glyph to be drawn.
 * @param codepoint			The codepoint of the glyph to be drawn.
 * @param position_x		The upper-left origin position X of the glyph to draw in pixels.
 * @param position_y		The upper-left origin position Y of the glyph to draw in pixels.
 * @param advance_x			Advance the position X in pixels after the glyph is drawn, can be NULL if no need.
 * @return					The status of the draw.
 */
esp_err_t slime_screen_draw_glyph(
	const	slime_screen_context_t*		screen_context,
	const	slime_screen_text_style_t*	text_style,
			uint16_t					codepoint,
			int32_t						position_x,
			int32_t						position_y,
			int32_t*					advance_x
);

/**
 * @brief					Draw a string on a given screen.
 * @param screen_context	The context of the screen to be drawn to.
 * @param text_style		The style of the text to be drawn.
 * @param position_x		The upper-left origin position X of the glyph to draw in pixels.
 * @param position_y		The upper-left origin position Y of the glyph to draw in pixels.
 * @param string			The string to be drawn
 * @return					The status of the draw.
 */
esp_err_t slime_screen_draw_string(
	const	slime_screen_context_t*		screen_context,
	const	slime_screen_text_style_t*	text_style,
			int32_t						position_x,
			int32_t						position_y,
			char*						string
);

/**
 * @brief					Draw a formatted string on a given screen.
 * @param screen_context	The context of the screen to be drawn to.
 * @param text_style		The style of the text to be drawn.
 * @param position_x		The upper-left origin position X of the glyph to draw in pixels.
 * @param position_y		The upper-left origin position Y of the glyph to draw in pixels.
 * @param string			The string to be drawn
 * @param ...				Optional arguments to be formatted according to the format string.
 * @return					The status of the draw.
 */
esp_err_t slime_screen_draw_string_fmt(
	const	slime_screen_context_t*		screen_context,
	const	slime_screen_text_style_t*	text_style,
			int32_t						position_x,
			int32_t						position_y,
			char*						string,
			...
);

/**
 * @brief					Commit all uncommitted changes on the framebuffer to the ring buffer slot and wait for transmission.
 * @param screen_context	The context of the screen to be committed.
 * @return					The status of the commit.
 */
esp_err_t slime_screen_commit(const slime_screen_context_t* screen_context);

/**
 * @brief						Create the screen context.
 * @param screen_context_out	The handle to receive the created screen context.
 * @param screen_context_config	The configuration of the screen context.
 * @param lcd_context			The LCD context of the screen.
 * @return						The status of the creation.
 */
esp_err_t slime_screen_context_new(
			slime_screen_context_t**		screen_context_out,
			slime_lcd_context_t*			lcd_context,
	const	slime_screen_context_config_t*	screen_context_config
);

/**
 * @brief					Release the screen context.
 * @param screen_context_in	The screen context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_screen_context_del(slime_screen_context_t* screen_context_in);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_SCREEN_H
