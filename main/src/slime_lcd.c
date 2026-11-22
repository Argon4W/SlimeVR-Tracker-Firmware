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

/**
 * @brief The definition of the debug logging function of the SPI part of the Slime LCD.
 */
#ifdef CONFIG_SLIME_SPI_BUS_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_SPI_LOG(format, ...) ESP_LOGD(SPI_TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_SPI_LOG(format, ...)
#endif // CONFIG_SLIME_SPI_BUS_DEBUG_LOGGING

/**
 * @brief The definition of the debug logging function of the Slime LCD.
 */
#ifdef CONFIG_SLIME_LCD_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_LCD_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_LCD_LOG(format, ...)
#endif // CONFIG_SLIME_LCD_DEBUG_LOGGING

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

	// Log the progress.
	SLIME_LCD_LOG("Creating LCD context.");
	SLIME_LCD_LOG("Reserving handles of LCD context.");

	// Reserve handles of LCD context.
	slime_lcd_context_t*		lcd_context		= NULL;
	esp_lcd_panel_io_handle_t	context_spi_io	= NULL;
	esp_lcd_panel_handle_t		context_panel	= NULL;

	// Log the progress.
	SLIME_SPI_LOG("Creating SPI bus.");

	// Initialize the SPI bus.
	ESP_RETURN_ON_ERROR(spi_bus_initialize(
		/* host_id		= */	lcd_context_config->spi_host_id,
		/* bus_config	= */ &	lcd_context_config->spi_bus_config,
		/* dma_chan		= */	lcd_context_config->spi_dma_channel
	), SPI_TAG, "Failed to initialize SPI bus.");

	// Log the progress.
	SLIME_SPI_LOG("SPI bus created.");
	SLIME_LCD_LOG("Creating LCD panel device.");

	// Create the LCD panel device
	ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi(SPI2_HOST,		&lcd_context_config->panel_io_spi_config, &context_spi_io),	error, LCD_TAG, "Failed to create SPI LCD panel IO.");
	ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7735(context_spi_io,	&lcd_context_config->panel_device_config, &context_panel),	error, LCD_TAG, "Failed to create LCD panel device.");

	// Log the progress.
	SLIME_LCD_LOG("Configuring LCD panel device.");

	// Set the gap of the LCD panel device.
	ESP_GOTO_ON_ERROR(esp_lcd_panel_set_gap(
		/* panel = */ context_panel,
		/* x_gap = */ lcd_context_config->gap_offset_x,
		/* y_gap = */ lcd_context_config->gap_offset_y
	), error, LCD_TAG, "Failed to set LCD gap offsets.");

	// Log the progress.
	SLIME_LCD_LOG("Initializing LCD panel device.");

	// Reset and initialize the LCD panel.
	ESP_GOTO_ON_ERROR(esp_lcd_panel_reset	(context_panel), error, LCD_TAG, "Failed to reset LCD panel device.");
	ESP_GOTO_ON_ERROR(esp_lcd_panel_init	(context_panel), error, LCD_TAG, "Failed to initialize LCD panel device.");

	// Log the progress.
	SLIME_LCD_LOG("Creating LCD context struct.");

	// Create the LCD context handle.
	lcd_context = calloc(1U, sizeof(slime_lcd_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(lcd_context != NULL, ESP_ERR_NO_MEM, error, LCD_TAG, "Failed to create the LCD context struct.");

	// Log the progress.
	SLIME_LCD_LOG("Finalizing LCD context.");

	// Fill the LCD context.
	lcd_context->spi_io_handle	= context_spi_io;
	lcd_context->panel_handle	= context_panel;
	lcd_context->gpio_context	= gpio_context;

	// Return the created LCD context handle.
	*lcd_context_out = lcd_context;

	// Log the progress.
	SLIME_LCD_LOG("LCD context has been created.");

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error.
	SLIME_LCD_LOG("Error occurred: %s", esp_err_to_name(ret));
	SLIME_LCD_LOG("Cleaning up resources.");

	if (lcd_context)					free				(lcd_context);		// Cleanup the LCD context struct.
	if (context_panel)	ESP_ERROR_CHECK(esp_lcd_panel_del	(context_panel));	// Cleanup the LCD panel device handle.
	if (context_spi_io)	ESP_ERROR_CHECK(esp_lcd_panel_io_del(context_spi_io));	// Cleanup the LCD SPI panel IO handle.

	// CLeanup the SPI bus, it must be initialized or returned before everything is allocated.
	ESP_ERROR_CHECK(spi_bus_free(SPI2_HOST));

	return ret;
}

esp_err_t slime_lcd_context_del(slime_lcd_context_t* lcd_context_in) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(lcd_context_in != NULL, ESP_ERR_INVALID_ARG, LCD_TAG, "No slime_lcd_context_t handle provided when releasing LCD context.");

	// Log the progress.
	SLIME_LCD_LOG("Releasing LCD context.");
	SLIME_LCD_LOG("Releasing LCD panel device.");

	// Release the panel device and SPI panel IO.
	ESP_RETURN_ON_ERROR(esp_lcd_panel_del	(lcd_context_in->panel_handle),		LCD_TAG, "Failed to release LCD panel device.");
	ESP_RETURN_ON_ERROR(esp_lcd_panel_io_del(lcd_context_in->spi_io_handle),	LCD_TAG, "Failed to release LCD SPI panel IO.");

	// Log the progress.
	SLIME_SPI_LOG("Releasing SPI bus.");

	// Releasing SPI bus.
	ESP_RETURN_ON_ERROR(spi_bus_free(SPI2_HOST), SPI_TAG, "Failed to release SPI bus.");

	// Log the progress.
	SLIME_LCD_LOG("Detaching all fields of the LCD context.");

	// Detach all fields.
	lcd_context_in->spi_io_handle	= NULL;
	lcd_context_in->panel_handle	= NULL;
	lcd_context_in->gpio_context	= NULL;

	// Log the progress.
	SLIME_LCD_LOG("Releasing the LCD context struct.");

	// Free the LCD context struct.
	free(lcd_context_in);

	// Log the progress.
	SLIME_LCD_LOG("LCD context has been released.");

	return ESP_OK;
}