#include "esp_check.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "string.h"
#include "esp_lcd_st7735.h"
#include "esp_fast_lcd.h"
#include "SlimeLCD.h"

static const_string SPI_TAG = "SlimeSPI";
static const_string LCD_TAG = "SlimeLCD";

// The SPI bus configuration for the LCD panel.
const static spi_bus_config_t spiBusConfig = {
	.mosi_io_num		= CONFIG_SPI_BUS_MOSI,	// Configurable MOSI GPIO Num through menuconfig.
	.sclk_io_num		= CONFIG_SPI_BUS_SCLK,	// Configurable SCLK GPIO Num through menuconfig.
	.miso_io_num		= GPIO_NUM_NC,			// ST7735 doesn't need MISO.
	.quadwp_io_num		= GPIO_NUM_NC,
	.quadhd_io_num		= GPIO_NUM_NC,
	.max_transfer_sz	= CONFIG_SPI_BUS_MAX_TRANSFER_SZ,							// Configurable maximum transfer size in bytes through menuconfig.
	.flags				= SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_IOMUX_PINS,	// Initialize the SPI Bus in master mode and uses IO mux rather than GPIO matrix.
};

// The SPI IO config of the LCD panel.
const static esp_lcd_panel_io_spi_config_t lcdPanelSPIConfig = {
	.cs_gpio_num			= GPIO_NUM_NC,				// We have only one device on the SPI Bus, we don't need CS.
	.dc_gpio_num			= CONFIG_LCD_PANEL_DC,		// Configurable DC GPIO Num through menuconfig.
	.pclk_hz				= CONFIG_SPI_BUS_FREQUENCY,	// Configurable SPI Bus frequency through menuconfig.
	.spi_mode				= 0,						// ST7735 supports SPI mode 0.
	.trans_queue_depth		= 10,						// Copied from test app of ST7735 ESP LCD Driver.
	.lcd_cmd_bits			= 8,						// Copied from test app of ST7735 ESP LCD Driver.
	.lcd_param_bits			= 8,						// Copied from test app of ST7735 ESP LCD Driver.
};

// The vendor-specific initialization commands of ST7735P3 96x54 gapX=16 gapY=106.
const static st7735_lcd_init_cmd_t st7735P3InitCommands[] = {
	{ ST7735_SWRESET,	(u8[]) {0x00},																								1,	150 },
	{ ST7735_SLPOUT,	(u8[]) {0x00},																								1,	255 },
	{ ST7735_FRMCTR1,	(u8[]) {0x01, 0x01, 0x01},																					3,	0 },
	{ ST7735_FRMCTR2,	(u8[]) {0x05, 0x3C, 0x3C},																					3,	0 },
	{ ST7735_FRMCTR3,	(u8[]) {0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C},																6,	0 },
	{ ST7735_INVCTR,	(u8[]) {0x03},																								1,	0 },
	{ ST7735_PWCTR1,	(u8[]) {0x0E, 0x0E, 0x04},																					3,	0 },
	{ ST7735_PWCTR2,	(u8[]) {0xC0},																								1,	0 },
	{ ST7735_PWCTR3,	(u8[]) {0x0D, 0x00},																						2,	0 },
	{ ST7735_PWCTR4,	(u8[]) {0x8D, 0x2A},																						2,	0 },
	{ ST7735_PWCTR5,	(u8[]) {0x8D, 0xEE},																						2,	0 },
	{ ST7735_VMCTR1,	(u8[]) {0x04},																								1,	0 },
	{ ST7735_MADCTL,	(u8[]) {0xC8},																								1,	0 },
	{ ST7735_COLMOD,	(u8[]) {0x05},																								1,	0 },
	{ ST7735_GMCTRP1,	(u8[]) {0x05, 0x1A, 0x0B, 0x15, 0x3D, 0x38, 0x2E, 0x30, 0x2D, 0x28, 0x30, 0x3B, 0x00, 0x01, 0x02, 0x10},	16,	0 },
	{ ST7735_GMCTRN1,	(u8[]) {0x05, 0x1A, 0x0B, 0x15, 0x36, 0x2E, 0x28, 0x2B, 0x2B, 0x28, 0x30, 0x3B, 0x00, 0x01, 0x02, 0x10},	16,	0 },
	{ ST7735_INVOFF,	(u8[]) {0x00},																								1,	0 },
	{ ST7735_NORON,		(u8[]) {0x00},																								1,	0 },
	{ ST7735_DISPON,	(u8[]) {0x00},																								1,	0 },
	{ ST7735_CASET,		(u8[]) {0x00, 0x10, 0x00, 0x6F},																			4,	0 },
	{ ST7735_RASET,		(u8[]) {0x00, 0x6A, 0x00, 0x9F},																			4,	0 },
	{ ST7735_RAMWR,		(u8[]) {0x00},																								1,	0 }
};

// The vendor config of the ST7735P3 holding the vendor-specific initialization commands/
static st7735_vendor_config_t st7735P3VendorConfig = {
	.init_cmds		= st7735P3InitCommands,	// The pointer to commands.
	.init_cmds_size	= 21					// The length of the commands.
};

const static esp_lcd_panel_dev_config_t lcdPanelDeviceConfig = {
	.reset_gpio_num	= CONFIG_LCD_PANEL_RES,			// Configurable reset GPIO num through menuconfig.
	.color_space	= ESP_LCD_COLOR_SPACE_RGB,		// Copied from test app of ST7735 ESP LCD Driver.
	.bits_per_pixel = 16,							// Copied from test app of ST7735 ESP LCD Driver.
	.vendor_config	= ref(st7735P3VendorConfig),	// Initialization command sequences of variant ST7735P3.
};

// The GPIO config of the backlight of the LCD panel.
const static gpio_config_t lcdPanelBacklightGPIOConfig = {
	.pin_bit_mask	= 1ULL << CONFIG_LCD_PANEL_BACKLIGHT,	// Configurable backlight GPIO Num through menuconfig.
	.mode			= GPIO_MODE_OUTPUT,						// Set to output mode.
	.pull_up_en		= GPIO_PULLUP_DISABLE,					// Disable internal pull-up.
	.pull_down_en	= GPIO_PULLDOWN_DISABLE,				// Disable internal pull-down.
	.intr_type		= GPIO_INTR_DISABLE						// Disable interrupt.
};

// LCD panel device properties of the ST7735
const static esp_fast_lcd_panel_configuration_t st7735Properties = {
	.frame_size_x			= 96U,
	.frame_size_y			= 54U,
	.frame_tile_size_x		= 16U,
	.frame_tile_size_y		= 18U,
	.ring_buffer_slot_count	= 3U,
	.name					= "ST7735P3 96x54",
	.buffer_flags			= MALLOC_CAP_INTERNAL
};

static const u32 st7735OffsetX	= 16U;
static const u32 st7735OffsetY	= 106U;

esp_err_t newLCDDeviceHandle(esp_fast_lcd_panel_device_t** context) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without an allocated context.
	ESP_RETURN_ON_FALSE(context != NULL, ESP_ERR_INVALID_ARG, LCD_TAG, "No esp_fast_lcd_panel_device_t handle provided when initializing LCD panel device.");

	// Reserve panel handle and panel IO handle for ST7735.
	esp_lcd_panel_io_handle_t	st7735PanelIO		= NULL;
	esp_lcd_panel_handle_t		st7735PanelHandle	= NULL;

	// Log the progress if SPI bus debug logging is enabled.
	#ifdef CONFIG_SPI_BUS_DEBUG_LOGGING
		ESP_LOGD(SPI_TAG, "Creating SPI bus.");
	#endif // CONFIG_SPI_BUS_DEBUG_LOGGING

	// Ensure SPI to only be freed when it is initialized.
	b8 spiInitialized = false;

	// Initialize the SPI bus.
	ESP_GOTO_ON_ERROR(spi_bus_initialize(
		/* host_id		= */ SPI2_HOST,
		/* bus_config	= */ ref(spiBusConfig),
		/* dma_chan		= */ SPI_DMA_CH_AUTO
	), error, SPI_TAG, "Failed to initialize SPI bus");

	// Set SPI initialized.
	spiInitialized = true;

	// Log the progress if SPI bus debug logging is enabled.
	#ifdef CONFIG_SPI_BUS_DEBUG_LOGGING
		ESP_LOGD(SPI_TAG, "SPI bus created.");
	#endif // CONFIG_SPI_BUS_DEBUG_LOGGING

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_LCD_PANEL_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Creating LCD panel device for ST7735.");
	#endif // CONFIG_LCD_PANEL_DEBUG_LOGGING

	ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi	(SPI2_HOST,			ref(lcdPanelSPIConfig),		ref(st7735PanelIO)),		error, LCD_TAG, "Failed to create LCD panel SPI IO for ST7735.");
	ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7735	(st7735PanelIO,		ref(lcdPanelDeviceConfig),	ref(st7735PanelHandle)),	error, LCD_TAG, "Failed to create LCD panel handle of ST7735.");
	ESP_GOTO_ON_ERROR(esp_lcd_panel_set_gap		(st7735PanelHandle,	st7735OffsetX,				st7735OffsetY),				error, LCD_TAG, "Failed to set LCD panel gap of ST7735.");

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_LCD_PANEL_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Initializing ST7735 LCD panel.");
	#endif // CONFIG_LCD_PANEL_DEBUG_LOGGING

	// Reset and initialize the ST7735.
	ESP_GOTO_ON_ERROR(esp_lcd_panel_reset	(st7735PanelHandle), error, LCD_TAG, "Failed to reset ST7735.");
	ESP_GOTO_ON_ERROR(esp_lcd_panel_init	(st7735PanelHandle), error, LCD_TAG, "Failed to initialize ST7735.");

	// The test app delays.
	vTaskDelay(pdMS_TO_TICKS(500));

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_LCD_PANEL_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Enabling LCD backlight GPIO.");
	#endif // CONFIG_LCD_PANEL_DEBUG_LOGGING

	ESP_GOTO_ON_ERROR(gpio_config	(ref(lcdPanelBacklightGPIOConfig)),	error, LCD_TAG, "Failed to configure LCD panel backlight GPIO.");
	ESP_GOTO_ON_ERROR(gpio_set_level(CONFIG_LCD_PANEL_BACKLIGHT, 1),	error, LCD_TAG, "Failed to set LCD panel backlight GPIO level.");

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_LCD_PANEL_DEBUG_LOGGING
			ESP_LOGD(LCD_TAG, "Creating LCD panel device for ST7735.");
	#endif // CONFIG_LCD_PANEL_DEBUG_LOGGING

	esp_fast_lcd_new_lcd_panel_device(
		/* panel_device_context_out		= */ context,
		/* panel_device_configuration	= */ st7735Properties,
		/* panel_handle					= */ st7735PanelHandle,
		/* panel_io						= */ st7735PanelIO
	);

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_LCD_PANEL_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "LCD panel device handle for ST7735 has created.");
	#endif // CONFIG_LCD_PANEL_DEBUG_LOGGING

	return ret;

	// Log the progress if SPI bus debug logging is enabled.
	#ifdef CONFIG_LCD_PANEL_DEBUG_LOGGING
		ESP_LOGD(LCD_TAG, "Error occurred: %s", esp_err_to_name(ret));
		ESP_LOGD(LCD_TAG, "Cleaning up resources.");
	#endif // CONFIG_LCD_PANEL_DEBUG_LOGGING

	// Resource cleanup when error occurred.
	error:

	if (st7735PanelHandle)	ESP_ERROR_CHECK(esp_lcd_panel_del	(st7735PanelHandle));	// Cleanup the LCD panel handle of ST7735.
	if (st7735PanelIO)		ESP_ERROR_CHECK(esp_lcd_panel_io_del(st7735PanelIO));		// Cleanup the LCD panel SPI IO of ST7735.
	if (spiInitialized)		ESP_ERROR_CHECK(spi_bus_free		(SPI2_HOST));			// CLeanup the SPI bus.

	return ret;
}