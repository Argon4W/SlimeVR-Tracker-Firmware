#include "slime_config.h"
#include "lsm6dsv_reg.h"
#include "qmc6309_reg.h"
#include "esp_lcd_st7735.h"

const slime_nvs_context_config_t nvs_context_config = {
	.nvs_partition = CONFIG_SLIME_NVS_PARTITION,	/*!< Configurable NVS partition name. */
	.nvs_namespace = CONFIG_SLIME_NVS_NAMESPACE,	/*!< Configurable NVS namespace. */
	.nvs_skip_init = false							/*!< We need NVS context to take over the initialization of the NVS flash initialization. */
};

const slime_gpio_context_config_t gpio_context_config = {
	.gpio_led_config = {
		.pin_bit_mask	= 1ULL << CONFIG_SLIME_GPIO_LED,	/*!< The configurable GPIO Num of the LED on the sensor board. */
		.mode			= GPIO_MODE_OUTPUT,					/*!< Set to output mode. */
		.pull_up_en		= GPIO_PULLUP_DISABLE,				/*!< Disable internal pull-up. */
		.pull_down_en	= GPIO_PULLDOWN_DISABLE,			/*!< Disable internal pull-down. */
		.intr_type		= GPIO_INTR_DISABLE					/*!< Disable interrupt. */
	},
	.gpio_backlight_config = {
		.pin_bit_mask	= 1ULL << CONFIG_SLIME_GPIO_BACKLIGHT,	/*!< The configurable GPIO Num of the Backlight of the LCD panel. */
		.mode			= GPIO_MODE_OUTPUT,						/*!< Set to output mode. */
		.pull_up_en		= GPIO_PULLUP_DISABLE,					/*!< Disable internal pull-up. */
		.pull_down_en	= GPIO_PULLDOWN_DISABLE,				/*!< Disable internal pull-down. */
		.intr_type		= GPIO_INTR_DISABLE						/*!< Disable interrupt. */
	}
};

const slime_i2c_context_config_t i2c_context_config = {
	.i2c_master_bus_config = {
		.clk_source			= I2C_CLK_SRC_DEFAULT,			/*!<  Use default I2C clock source. */
		.i2c_port			= I2C_NUM_0,					/*!<  Use I2C0. */
		.scl_io_num			= CONFIG_SLIME_I2C_MASTER_SCL,	/*!< Configurable SCL GPIO Num. */
		.sda_io_num			= CONFIG_SLIME_I2C_MASTER_SDA,	/*!< Configurable SDA GPIO Num. */
		.glitch_ignore_cnt	= 7								/*!< Typical value of glitch period ignore count.*/
	},
	.i2c_imu_device_config = {
		.dev_addr_length	= I2C_ADDR_BIT_LEN_7,					/*!< Use 7-bit address length. */
		.device_address		= LSM6DSV_I2C_ADD_L >> 1,				/*!< Convert the 8-bit address into 7-bit. */
		.scl_speed_hz		= CONFIG_SLIME_I2C_MASTER_FREQUENCY,	/*!< Configurable I2C master device frequency. */
	},
	.i2c_mag_device_config = {
		.dev_addr_length	= I2C_ADDR_BIT_LEN_7,					/*!< Use 7-bit address length. */
		.device_address		= QMC6309_I2C_ADDRESS,					/*!< The address in QMC6309 driver is already 7-bit. */
		.scl_speed_hz		= CONFIG_SLIME_I2C_MASTER_FREQUENCY,	/*!< Configurable I2C master device frequency. */
	}
};

/**
 * @brief The internal ST7735P3 96x54 vendor-specific initialization command sequence of the LCD context .
 */
static const st7735_lcd_init_cmd_t st7735p3_96x54_init_commands[] = {
	{ ST7735_SWRESET,	(uint8_t[]) {0x00U},																											1,	150U },
	{ ST7735_SLPOUT,	(uint8_t[]) {0x00U},																											1,	255U },
	{ ST7735_FRMCTR1,	(uint8_t[]) {0x01U, 0x01U, 0x01U},																								3,	0U },
	{ ST7735_FRMCTR2,	(uint8_t[]) {0x05U, 0x3CU, 0x3CU},																								3,	0U },
	{ ST7735_FRMCTR3,	(uint8_t[]) {0x05U, 0x3CU, 0x3CU, 0x05U, 0x3CU, 0x3CU},																			6,	0U },
	{ ST7735_INVCTR,	(uint8_t[]) {0x03U},																											1,	0U },
	{ ST7735_PWCTR1,	(uint8_t[]) {0x0EU, 0x0EU, 0x04U},																								3,	0U },
	{ ST7735_PWCTR2,	(uint8_t[]) {0xC0U},																											1,	0U },
	{ ST7735_PWCTR3,	(uint8_t[]) {0x0DU, 0x00U},																										2,	0U },
	{ ST7735_PWCTR4,	(uint8_t[]) {0x8DU, 0x2AU},																										2,	0U },
	{ ST7735_PWCTR5,	(uint8_t[]) {0x8DU, 0xEEU},																										2,	0U },
	{ ST7735_VMCTR1,	(uint8_t[]) {0x04U},																											1,	0U },
	{ ST7735_MADCTL,	(uint8_t[]) {0xC8U},																											1,	0U },
	{ ST7735_COLMOD,	(uint8_t[]) {0x05U},																											1,	0U },
	{ ST7735_GMCTRP1,	(uint8_t[]) {0x05U, 0x1AU, 0x0BU, 0x15U, 0x3DU, 0x38U, 0x2EU, 0x30U, 0x2DU, 0x28U, 0x30U, 0x3BU, 0x00U, 0x01U, 0x02U, 0x10U},	16,	0U },
	{ ST7735_GMCTRN1,	(uint8_t[]) {0x05U, 0x1AU, 0x0BU, 0x15U, 0x36U, 0x2EU, 0x28U, 0x2BU, 0x2BU, 0x28U, 0x30U, 0x3BU, 0x00U, 0x01U, 0x02U, 0x10U},	16,	0U },
	{ ST7735_INVOFF,	(uint8_t[]) {0x00U},																											1,	0U },
	{ ST7735_NORON,		(uint8_t[]) {0x00U},																											1,	0U },
	{ ST7735_DISPON,	(uint8_t[]) {0x00U},																											1,	0U }
};

/**
 * @brief The internal ST7735P3 vendor-specific configuration of the LCD context.
 */
static st7735_vendor_config_t st7735p3_96x54_vendor_config = {
	.init_cmds		= st7735p3_96x54_init_commands,	/*!< The pointer to initialization command sequence. */
	.init_cmds_size	= 19U							/*!< The count of commands of the initialization command sequence. */
};

const slime_lcd_context_config_t lcd_context_config = {
	.lcd_spi_bus_config = {
		.mosi_io_num		= CONFIG_SLIME_SPI_BUS_MOSI,								/*!< Configurable MOSI GPIO Num. */
		.sclk_io_num		= CONFIG_SLIME_SPI_BUS_SCLK,								/*!< Configurable SCLK GPIO Num. */
		.miso_io_num		= GPIO_NUM_NC,												/*!< ST7735 doesn't need master input. */
		.quadwp_io_num		= GPIO_NUM_NC,												/*!< ST7735 doesn't need write protect. */
		.quadhd_io_num		= GPIO_NUM_NC,												/*!< ST7735 doesn't need hold. */
		.max_transfer_sz	= CONFIG_SLIME_SPI_BUS_MAX_TRANSFER_SZ,						/*!< Configurable maximum transfer size in bytes. */
		.flags				= SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_IOMUX_PINS,	/*!< Make sure the SPI Bus in master mode and uses IO mux rather than GPIO matrix. */
	},
	.lcd_panel_io_spi_config = {
		.cs_gpio_num		= GPIO_NUM_NC,						/*!< We don't need CS because we have only one device attached to the bus. */
		.dc_gpio_num		= CONFIG_SLIME_LCD_DC,				/*!< Configurable DC GPIO Num. */
		.pclk_hz			= CONFIG_SLIME_SPI_BUS_FREQUENCY,	/*!< Configurable SPI Bus frequency. */
		.spi_mode			= 0,								/*!< ST7735 supports SPI mode 0. */
		.trans_queue_depth	= 10,								/*!< Copied from test app of ST7735 ESP LCD Driver. */
		.lcd_cmd_bits		= 8,								/*!< Copied from test app of ST7735 ESP LCD Driver. */
		.lcd_param_bits		= 8,								/*!< Copied from test app of ST7735 ESP LCD Driver. */
	},
	.lcd_panel_device_config = {
		.reset_gpio_num	= CONFIG_SLIME_LCD_RES,				/*!< Configurable reset GPIO num. */
		.color_space	= ESP_LCD_COLOR_SPACE_RGB,			/*!< Copied from test app of ST7735 ESP LCD Driver. */
		.bits_per_pixel = 16,								/*!< Copied from test app of ST7735 ESP LCD Driver. */
		.vendor_config	= &st7735p3_96x54_vendor_config,	/*!< The vendor configuration of ST7735P3 96x54. */
	},
	.lcd_gap_offset_x = 16U,	/*!< The vendor specific gap offset X in pixels of the ST7735P3 96x54. */
	.lcd_gap_offset_y = 106U	/*!< The vendor specific gap offset Y in pixels of the ST7735P3 96x54. */
};

const slime_screen_context_config_t screen_context_config = {
	.screen_transmit_task_config = {
		.transmit_task_framerate	= CONFIG_SLIME_SCREEN_TRANSMIT_TASK_FRAMERATE,		/*!< Configurable framerate of the transmission task. */
		.transmit_task_core_id		= CONFIG_SLIME_SCREEN_TRANSMIT_TASK_CORE_ID,		/*!< Configurable core ID of the transmission task. */
		.transmit_task_stack_depth	= CONFIG_SLIME_SCREEN_TRANSMIT_TASK_STACK_DEPTH,	/*!< Configurable stack depth of the transmission task. */
		.transmit_task_priority		= CONFIG_SLIME_SCREEN_TRANSMIT_TASK_PRIORITY		/*!< Configurable priority of the transmission task. */
	},
	.screen_fast_lcd_panel_config = {
		.frame_size_x			= 96U,					/*!< Set the width of the screen to the width of the LCD panel in pixels. */
		.frame_size_y			= 54U,					/*!< Set the height of the screen to the height of the LCD panel in pixels. */
		.frame_tile_size_x		= 16U,					/*!< Slice the X axis of the screen into 6 tiles, */
		.frame_tile_size_y		= 18U,					/*!< Slice the Y axis of the screen into 3 tiles, */
		.ring_buffer_slot_count	= 3U,					/*!< Common 3 in-flight transmissions. */
		.buffer_flags			= MALLOC_CAP_INTERNAL,	/*!< Allocate the framebuffer and ring buffer slots in the internal SRAM */
		.name					= "slime_screen"		/*!< The name of the screen used in debugging. */
	},
	.screen_fast_text_engine_instance_config = {
		.crlf_mode				= false,						/*!< We don't use CRLF. */
		.font_size_scale		= 1U,							/*!< We usually use scale 1 text. */
		.font_outline_radius	= 1U,							/*!< 1px outline radius is enough for scale 1. */
		.iram_atlas_slot_count	= 18U,							/*!< The maximum count that can display on the screen is about 18 (3*6). */
		.psram_atlas_slot_count	= 32U,							/*!< L2 cache should be bigger than L1 cache for real-time performance (e.g. switching back to parent view). */
		.string_buffer_size		= 32U,							/*!< 32 is enough for 96x54 screen. */
		.atlas_flags			= 0U,							/*!< No extra flags needed. */
		.name					= "slime_screen_text_engine"	/*!< The name of the text engine. */
	},
	.screen_fast_text_engine_font = {
		.size_x_max		= 16U,							/*!< The height of the glyphs of Unifont is 16 in pixels. */
		.size_y			= 16U,							/*!< The maximum width of the glyphs of Unifont is 16 in pixels. */
		.glyph_data		= unifont_17_0_05_1bpp_bits,	/*!< The tightly packed 1bpp LSBit-first font glyph data of the modified version of the Unifont 17.0.05. */
		.glyph_table	= glyph_table					/*!< The lookup-table of the font. */
	}
};

const slime_button_context_config_t button_context_config = {
	.button_return_config = {
		.button_config = {
			.long_press_time	= 0,								/*!< We don't need long press for now. */
			.short_press_time	= CONFIG_SLIME_BUTTON_CLICK_TIME	/*!< Configurable short press time in milliseconds. */
		},
		.button_gpio_config = {
			.gpio_num			= CONFIG_SLIME_BUTTON_RETURN,		/*!< Configurable GPIO Num of the return button. */
			.active_level		= CONFIG_SLIME_BUTTON_ACTIVE_LEVEL,	/*!< Configurable GPIO level when the button is pressed down. */
			.enable_power_save	= false,							/*!< We don't need power saving. */
			#ifdef SLIME_BUTTON_DISABLE_INTERNAL_PULL				/*!< Configurable internal pull-up/pull-down. */
				.disable_pull	= true
			#endif // SLIME_BUTTON_DISABLE_INTERNAL_PULL
		}
	},
	.button_switch_config = {
		.button_config = {
			.long_press_time	= 0,								/*!< We don't need long press for now. */
			.short_press_time	= CONFIG_SLIME_BUTTON_CLICK_TIME	/*!< Configurable short press time in milliseconds. */
		},
		.button_gpio_config = {
			.gpio_num			= CONFIG_SLIME_BUTTON_SWITCH,		/*!< Configurable GPIO Num of the switch button. */
			.active_level		= CONFIG_SLIME_BUTTON_ACTIVE_LEVEL,	/*!< Configurable GPIO level when the button is pressed down. */
			.enable_power_save	= false,							/*!< We don't need power saving. */
			#ifdef SLIME_BUTTON_DISABLE_INTERNAL_PULL				/*!< Configurable internal pull-up/pull-down. */
				.disable_pull	= true
			#endif // SLIME_BUTTON_DISABLE_INTERNAL_PULL
		}
	},
	.button_confirm_config = {
		.button_config = {
			.long_press_time	= 0,								/*!< We don't need long press for now. */
			.short_press_time	= CONFIG_SLIME_BUTTON_CLICK_TIME	/*!< Configurable short press time in milliseconds. */
		},
		.button_gpio_config = {
			.gpio_num			= CONFIG_SLIME_BUTTON_CONFIRM,		/*!< Configurable GPIO Num of the confirm button. */
			.active_level		= CONFIG_SLIME_BUTTON_ACTIVE_LEVEL,	/*!< Configurable GPIO level when the button is pressed down. */
			.enable_power_save	= false,							/*!< We don't need power saving. */
			#ifdef SLIME_BUTTON_DISABLE_INTERNAL_PULL				/*!< Configurable internal pull-up/pull-down. */
				.disable_pull	= true
			#endif // SLIME_BUTTON_DISABLE_INTERNAL_PULL
		}
	},
	.button_event_queue_size = CONFIG_SLIME_BUTTON_EVENT_QUEUE_SIZE /*!< Configurable button event queue size. */
};