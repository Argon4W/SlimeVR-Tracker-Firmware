#include "stdlib.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7735.h"
#include "slime_lcd.h"

/**
 * @brief The log tag of the SPI part of the Slime LCD.
 */
static const char* SPI_TAG = "slime_lcd_spi";

/**
 * @brief The log tag of the Slime LCD.
 */
static const char* LCD_TAG = "slime_lcd";

esp_err_t slime_lcd_context_new(
			slime_lcd_context_t**		lcd_context_out,
	const	slime_lcd_context_config_t*	lcd_context_config,
	const	slime_gpio_context_t*		gpio_context
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration, a GPIO context, and a handle to receive the created LCD context.
	ESP_RETURN_ON_FALSE(lcd_context_out		!= NULL, ESP_ERR_INVALID_ARG, LCD_TAG, "No slime_lcd_context_t handle provided when creating LCD context.");
	ESP_RETURN_ON_FALSE(lcd_context_config	!= NULL, ESP_ERR_INVALID_ARG, LCD_TAG, "No slime_lcd_context_config_t handle provided when creating LCD context.");
	ESP_RETURN_ON_FALSE(gpio_context		!= NULL, ESP_ERR_INVALID_ARG, LCD_TAG, "No slime_gpio_context_t handle provided when creating LCD context.");

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Creating LCD context.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Reserving handles of LCD context.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	// Reserve handles for LCD context.
	slime_lcd_context_t*		lcd_context			= NULL;
	esp_lcd_panel_io_handle_t	context_panel_io	= NULL;
	esp_lcd_panel_handle_t		context_panel		= NULL;

	// Log the progress if SPI bus debug logging is enabled.
	#ifdef CONFIG_SLIME_SPI_BUS_DEBUG_LOGGING
		ESP_LOGD(SPI_TAG, "Creating SPI bus.");
	#endif // CONFIG_SLIME_SPI_BUS_DEBUG_LOGGING

	// Ensure SPI to only be freed when it is initialized.
	uint8_t spi_initialized = false;

	// Initialize the SPI bus.
	ESP_GOTO_ON_ERROR(spi_bus_initialize(
		/* host_id		= */ SPI2_HOST,
		/* bus_config	= */ &lcd_context_config->spi_bus_config,
		/* dma_chan		= */ SPI_DMA_CH_AUTO
	), error, SPI_TAG, "Failed to initialize SPI bus.");

	// Mark SPI initialized.
	spi_initialized = true;

	// Log the progress if SPI bus debug logging is enabled.
	#ifdef CONFIG_SLIME_SPI_BUS_DEBUG_LOGGING
		ESP_LOGD(SPI_TAG, "SPI bus created.");
	#endif // CONFIG_SLIME_SPI_BUS_DEBUG_LOGGING

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Creating LCD panel device.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	// Create the LCD panel device
	ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi	(SPI2_HOST,			&lcd_context_config->panel_io_spi_config,	&context_panel_io),					error, LCD_TAG, "Failed to create SPI LCD panel IO.");
	ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7735	(context_panel_io,	&lcd_context_config->panel_device_config,	&context_panel),					error, LCD_TAG, "Failed to create LCD panel device.");
	ESP_GOTO_ON_ERROR(esp_lcd_panel_set_gap		(context_panel,		lcd_context_config->gap_offset_x,			lcd_context_config->gap_offset_y),	error, LCD_TAG, "Failed to set LCD gap offsets.");

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Initializing LCD panel device.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	// Reset and initialize the LCD panel.
	ESP_GOTO_ON_ERROR(esp_lcd_panel_reset	(context_panel), error, LCD_TAG, "Failed to reset LCD panel device.");
	ESP_GOTO_ON_ERROR(esp_lcd_panel_init	(context_panel), error, LCD_TAG, "Failed to initialize LCD panel device.");

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
			ESP_LOGD(LCD_TAG, "Creating LCD context struct.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	// Create the LCD context handle.
	lcd_context = calloc(1, sizeof(slime_lcd_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(lcd_context != NULL, ESP_ERR_NO_MEM, error, LCD_TAG, "Failed to create the LCD context struct.");

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
			ESP_LOGD(LCD_TAG, "Finalizing LCD context.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	// Fill the LCD context.
	lcd_context->panel_io_handle	= context_panel_io;
	lcd_context->panel_handle		= context_panel;
	lcd_context->gpio_context		= gpio_context;

	// Return the created LCD context handle.
	*lcd_context_out = lcd_context;

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "LCD context has been created.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error if LCD panel debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Error occurred: %s", esp_err_to_name(ret));
		ESP_LOGD(LCD_TAG, "Cleaning up resources.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	if (lcd_context)						free				(lcd_context);			// Cleanup the LCD context struct.
	if (context_panel)		ESP_ERROR_CHECK(esp_lcd_panel_del	(context_panel));		// Cleanup the LCD panel device handle.
	if (context_panel_io)	ESP_ERROR_CHECK(esp_lcd_panel_io_del(context_panel_io));	// Cleanup the LCD SPI panel IO handle.
	if (spi_initialized)	ESP_ERROR_CHECK(spi_bus_free		(SPI2_HOST));			// CLeanup the SPI bus.

	return ret;
}

esp_err_t slime_lcd_context_del(slime_lcd_context_t* lcd_context_in) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(lcd_context_in != NULL, ESP_ERR_INVALID_ARG, LCD_TAG, "No slime_lcd_context_t handle provided when releasing LCD context.");

	// Log the progress if LCD debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Releasing LCD context.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	// Log the progress if LCD debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Releasing LCD panel device.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	// Release the panel device and SPI panel IO.
	ESP_RETURN_ON_ERROR(esp_lcd_panel_del	(lcd_context_in->panel_handle),		LCD_TAG, "Failed to release LCD panel device.");
	ESP_RETURN_ON_ERROR(esp_lcd_panel_io_del(lcd_context_in->panel_io_handle),	LCD_TAG, "Failed to release LCD SPI panel IO.");

	// Log the progress if LCD debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(SPI_TAG, "Releasing SPI bus.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	// Releasing SPI bus.
	ESP_RETURN_ON_ERROR(spi_bus_free(SPI2_HOST), LCD_TAG, "Failed to release SPI bus.");

	// Log the progress if LCD debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Detaching all fields of the LCD context.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	// Detach all fields.
	lcd_context_in->panel_io_handle	= NULL;
	lcd_context_in->panel_handle	= NULL;
	lcd_context_in->gpio_context	= NULL;

	// Log the progress if LCD debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Releasing the LCD context struct.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	// Free the LCD context struct.
	free(lcd_context_in);

	// Log the progress if LCD debug logging is enabled.
	#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "LCD context has been released.");
	#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

	return ESP_OK;
}