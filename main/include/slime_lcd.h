#ifndef SLIME_LCD_H
#define SLIME_LCD_H

#include "esp_check.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_io.h"
#include "slime_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The configuration struct of the LCD context.
 */
typedef struct {
	spi_bus_config_t				lcd_spi_bus_config;			/*!< The SPI bus configuration of the LCD. */
	esp_lcd_panel_io_spi_config_t	lcd_panel_io_spi_config;	/*!< The SPI panel IO configuration of the LCD. */
	esp_lcd_panel_dev_config_t		lcd_panel_device_config;	/*!< The configuration of the panel device of the LCD. */
	uint32_t						lcd_gap_offset_x;			/*!< The vendor specific gap offset X in pixels of the LCD. */
	uint32_t						lcd_gap_offset_y;			/*!< The vendor specific gap offset Y in pixels of the LCD. */
} slime_lcd_context_config_t;

/**
 * @brief The LCD context struct.
 */
typedef struct {
			esp_lcd_panel_io_handle_t	panel_io_handle;	/*!< The panel IO handle of the LCD. */
			esp_lcd_panel_handle_t		panel_handle;		/*!< The panel handle of the LCD. */
	const	slime_gpio_context_t*		gpio_context;		/*!< The GPIO context for backlight control of the LCD. */
} slime_lcd_context_t;

/**
 * @brief						Create the LCD context.
 * @param lcd_context_out		The handle to receive the created LCD context.
 * @param lcd_context_config	The configuration of the LCD context.
 * @param gpio_context			The GPIO context that provides the backlight control.
 * @return						The status of the creation.
 */
esp_err_t slime_lcd_context_new(
			slime_lcd_context_t**		lcd_context_out,
	const	slime_gpio_context_t*		gpio_context,
	const	slime_lcd_context_config_t*	lcd_context_config
);

/**
 * @brief					Release the LCD context.
 * @param lcd_context_in	The LCD context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_lcd_context_del(slime_lcd_context_t* lcd_context_in);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_LCD_H
