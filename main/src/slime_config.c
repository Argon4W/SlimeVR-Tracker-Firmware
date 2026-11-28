#include "slime_config.h"

#include <vqf.h>

#include "lsm6dsv_reg.h"
#include "qmc6309_reg.h"
#include "esp_lcd_st7735.h"
#include "slime_sensor_empty.h"
#include "slime_sensor_lsm6dsv_mmc5603.h"
#include "slime_sensor_lsm6dsv_qmc6309.h"
#include "slime_fusion_vqf.h"

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

/**
 * @brief The internal default value of the serialized calibration coefficients.
 */
static const slime_magneto_calibration_coefficients_t slime_magneto_calibration_coefficients_default = {
	.soft_iron_matrix = {	/*!< The identity soft iron matrix. */
		1.0f, 0.0f, 0.0f,	/*!< The first row of the soft iron matrix. */
		0.0f, 1.0f, 0.0f,	/*!< The second row of the soft iron matrix. */
		0.0f, 0.0f, 1.0f	/*!< The third row of the soft iron matrix. */
	},
	.hard_iron_vector = { /*!< The zero hard iron vector. */
		0.0f,	/*!< The X dimension component of the hard iron vector. */
		0.0f,	/*!< The Y dimension component of the hard iron vector. */
		0.0f	/*!< The Z dimension component of the hard iron vector. */
	},
	.reference_length	= 0.0f,	/*!< Invalid reference value. */
	.valid				= false	/*!< Not valid calibration data. */
};

/**
 * @brief The internal configuration of the LSM6DSV+MMC5603 sensor context.
 */
static const slime_lsm6dsv_mmc5603_sensor_context_config_t slime_lsm6dsv_mmc5603_sensor_context_config = {
	.lsm6dsv_device_address					= LSM6DSV_I2C_ADD_L >> 1U,		/*!< The I2C address of the LSM6DSV in 7-bit format. */
	.mmc5603_device_address					= MMC5603_I2C_ADDRESS,			/*!< The I2C address of the MMC5603. */
	.lsm6dsv_gyroscope_full_scale			= LSM6DSV_1000dps,				/*!< Set gyroscope full scale range to 1000dps. */
	.lsm6dsv_accelerometer_full_scale		= LSM6DSV_4g,					/*!< Set accelerometer full scale range to 4g. */
	.lsm6dsv_sensor_hub_data_rate			= LSM6DSV_SH_60Hz,				/*!< Set sensor hub data rate to 60Hz that matches the 60Hz ODR of MMC5603. */
	.lsm6dsv_gyroscope_data_rate			= LSM6DSV_ODR_AT_240Hz,			/*!< Set the output data rate of gyroscope to 960Hz. */
	.lsm6dsv_accelerometer_data_rate		= LSM6DSV_ODR_AT_120Hz,			/*!< Set the output data rate of accelerometer to 960Hz. */
	.lsm6dsv_fifo_gyroscope_batch_rate		= LSM6DSV_GY_BATCHED_AT_240Hz,	/*!< Set the FIFO batch rate of gyroscope to the same rate as the gyroscope ODR. */
	.lsm6dsv_fifo_accelerometer_batch_rate	= LSM6DSV_XL_BATCHED_AT_120Hz,	/*!< Set the FIFO batch rate of accelerometer to the same rate as the accelerometer ODR. */
	.mmc5603_setup = {								/*!< The MMC5603 setup parameters. */
		.odr					= 60U,				/*!< Set the output data rate to 60Hz. */
		.high_power				= PROPERTY_DISABLE,	/*!< We don't need 1000Hz. */
		.auto_set_reset			= PROPERTY_ENABLE,	/*!< MUST enable the automatic set/reset. */
		.periodic_set			= PROPERTY_DISABLE,	/*!< It is recommended to be enabled, but it will shift the output for some reason. */
		.bandwidth				= MEAS_TIME_6_6_MS	/*!< Better noise performance. */
	}
};

/**
 * @brief The internal configuration of the LSM6DSV+QMC6309 sensor context.
 */
static const slime_lsm6dsv_qmc6309_sensor_context_config_t slime_lsm6dsv_qmc6309_sensor_context_config = {
	.lsm6dsv_device_address					= LSM6DSV_I2C_ADD_L >> 1U,		/*!< The I2C address of the LSM6DSV in 7-bit format. */
	.qmc6309_device_address					= QMC6309_I2C_ADDRESS,			/*!< The I2C address of the QMC6309. */
	.lsm6dsv_gyroscope_full_scale			= LSM6DSV_1000dps,				/*!< Set gyroscope full scale range to 1000dps. */
	.lsm6dsv_accelerometer_full_scale		= LSM6DSV_4g,					/*!< Set accelerometer full scale range to 4g. */
	.lsm6dsv_sensor_hub_data_rate			= LSM6DSV_SH_60Hz,				/*!< Set sensor hub data rate to 60Hz that matches the 50Hz ODR of QMC6309. */
	.lsm6dsv_gyroscope_data_rate			= LSM6DSV_ODR_AT_240Hz,			/*!< Set the output data rate of gyroscope to 960Hz. */
	.lsm6dsv_accelerometer_data_rate		= LSM6DSV_ODR_AT_120Hz,			/*!< Set the output data rate of accelerometer to 960Hz. */
	.lsm6dsv_fifo_gyroscope_batch_rate		= LSM6DSV_GY_BATCHED_AT_240Hz,	/*!< Set the FIFO batch rate of gyroscope to the same rate as the gyroscope ODR. */
	.lsm6dsv_fifo_accelerometer_batch_rate	= LSM6DSV_XL_BATCHED_AT_120Hz,	/*!< Set the FIFO batch rate of accelerometer to the same rate as the accelerometer ODR. */
	.qmc6309_setup = {					/*!< The QMC6309 setup parameters. */
		.mode			= NORMAL,		/*!< Set to normal mode. */
		.set_reset_mode	= SET_RESET_ON,	/*!< MUST set the set/reset mode to ON. */
		.rng			= RNG_8G,		/*!< Set the full scale range to 8G. */
		.odr			= ODR_50HZ,		/*!< Set the output data rate to 50Hz. */
		.osr1			= OSR1_8,		/*!< Set over sample rate to 8. */
		.osr2			= OSR2_8		/*!< Set low pass filter depth to 8. */
	}
};

/**
 * @brief The internal parameters of the VQF fusion.
 */
static const vqf_params_t vqf_params = {
	.tau_acc							= 3.0f,
	.tau_mag							= 3.0f,
	.motion_bias_est_enabled			= true,
	.rest_bias_est_enabled				= true,
	.mag_dist_rejection_enabled			= true,
	.bias_sigma_init					= 0.5f,
	.bias_forgetting_time				= 100.0f,
	.bias_clip							= 2.0f,
	.bias_sigma_motion					= 0.1f,
	.bias_vertical_forgetting_factor	= 0.0001f,
	.bias_sigma_rest					= 0.03f,
	.rest_min_t							= 1.5f,
	.rest_filter_tau					= 0.5f,
	.rest_th_gyr						= 2.0f,
	.rest_th_acc						= 0.5f,
	.mag_current_tau					= 0.05f,
	.mag_ref_tau						= 20.0f,
	.mag_nrm_th							= 0.1f,
	.mag_dip_th							= 10.0f,
	.mag_new_time						= 20.0f,
	.mag_new_first_time					= 5.0f,
	.mag_new_min_gyr					= 20.0f,
	.mag_min_undisturbed_time			= 0.5f,
	.mag_max_rejection_time				= 60.0f,
	.mag_rejection_factor				= 2.0f
};

const slime_nvs_context_config_t nvs_context_config = {
	.partition = CONFIG_SLIME_NVS_PARTITION, /*!< Configurable NVS partition name. */
	.namespace = CONFIG_SLIME_NVS_NAMESPACE, /*!< Configurable NVS namespace. */
};

const slime_wifi_context_config_t wifi_context_config = {
	.wifi_sta_config = {									/*!< The Wi-Fi station configuration. */
		.ssid				= CONFIG_SLIME_WIFI_CRED_SSID,	/*!< Configurable Wi-Fi credential SSID. */
		.password			= CONFIG_SLIME_WIFI_CRED_PASS,	/*!< Configurable Wi-Fi credential Password. */
		.scan_method		= WIFI_ALL_CHANNEL_SCAN,		/*!< Do all channel scan. */
		.sort_method		= WIFI_CONNECT_AP_BY_SIGNAL,	/*!< Scan results sorted by signal. */
		.threshold.authmode	= WIFI_AUTH_WPA_WPA2_PSK		/*!< Recommended security threshold. */
	},
	.credential_retry_count_max = CONFIG_SLIME_WIFI_MAX_RETRY_COUNT /*!< Configurable maximum retry count. */
};

const slime_gpio_context_config_t gpio_context_config = {
	.led_gpio_config = {									/*!< The configuration of the LED GPIO. */
		.pin_bit_mask	= 1ULL << CONFIG_SLIME_GPIO_LED,	/*!< The configurable GPIO Num of the LED on the sensor board. */
		.mode			= GPIO_MODE_OUTPUT,					/*!< Set to output mode. */
		.pull_up_en		= GPIO_PULLUP_DISABLE,				/*!< Disable internal pull-up. */
		.pull_down_en	= GPIO_PULLDOWN_DISABLE,			/*!< Disable internal pull-down. */
		.intr_type		= GPIO_INTR_DISABLE					/*!< Disable interrupt. */
	},
	.backlight_gpio_config = {									/*!< The configuration of the Backlight GPIO. */
		.pin_bit_mask	= 1ULL << CONFIG_SLIME_GPIO_BACKLIGHT,	/*!< The configurable GPIO Num of the Backlight of the LCD panel. */
		.mode			= GPIO_MODE_OUTPUT,						/*!< Set to output mode. */
		.pull_up_en		= GPIO_PULLUP_DISABLE,					/*!< Disable internal pull-up. */
		.pull_down_en	= GPIO_PULLDOWN_DISABLE,				/*!< Disable internal pull-down. */
		.intr_type		= GPIO_INTR_DISABLE						/*!< Disable interrupt. */
	},
	.sensor_id_0_gpio_config = {											/*!< The configuration of the sensor board ID bit 0 GPIO. */
		.pin_bit_mask	= 1ULL << CONFIG_SLIME_GPIO_SENSOR_BOARD_ID_BIT_0,	/*!< The configurable GPIO Num of the Backlight of the LCD panel. */
		.mode			= GPIO_MODE_INPUT,									/*!< Set to output mode. */
		.pull_up_en		= GPIO_PULLUP_DISABLE,								/*!< Disable internal pull-up. */
		.pull_down_en	= GPIO_PULLDOWN_DISABLE,							/*!< Disable internal pull-down. */
		.intr_type		= GPIO_INTR_DISABLE									/*!< Disable interrupt. */
	},
	.sensor_id_1_gpio_config = {											/*!< The configuration of the sensor board ID bit 1 GPIO. */
		.pin_bit_mask	= 1ULL << CONFIG_SLIME_GPIO_SENSOR_BOARD_ID_BIT_1,	/*!< The configurable GPIO Num of the Backlight of the LCD panel. */
		.mode			= GPIO_MODE_INPUT,									/*!< Set to output mode. */
		.pull_up_en		= GPIO_PULLUP_DISABLE,								/*!< Disable internal pull-up. */
		.pull_down_en	= GPIO_PULLDOWN_DISABLE,							/*!< Disable internal pull-down. */
		.intr_type		= GPIO_INTR_DISABLE									/*!< Disable interrupt. */
	},
	.sensor_id_2_gpio_config = {											/*!< The configuration of the sensor board ID bit 2 GPIO. */
		.pin_bit_mask	= 1ULL << CONFIG_SLIME_GPIO_SENSOR_BOARD_ID_BIT_2,	/*!< The configurable GPIO Num of the Backlight of the LCD panel. */
		.mode			= GPIO_MODE_INPUT,									/*!< Set to output mode. */
		.pull_up_en		= GPIO_PULLUP_DISABLE,								/*!< Disable internal pull-up. */
		.pull_down_en	= GPIO_PULLDOWN_DISABLE,							/*!< Disable internal pull-down. */
		.intr_type		= GPIO_INTR_DISABLE									/*!< Disable interrupt. */
	}
};

const slime_i2c_context_config_t i2c_context_config = {
	.i2c_master_bus_config = {								/*!< The configuration of the I2C master bus. */
		.clk_source			= I2C_CLK_SRC_DEFAULT,			/*!<  Use default I2C clock source. */
		.i2c_port			= I2C_NUM_0,					/*!<  Use I2C0. */
		.scl_io_num			= CONFIG_SLIME_I2C_MASTER_SCL,	/*!< Configurable SCL GPIO Num. */
		.sda_io_num			= CONFIG_SLIME_I2C_MASTER_SDA,	/*!< Configurable SDA GPIO Num. */
		.glitch_ignore_cnt	= 7								/*!< Typical value of glitch period ignore count.*/
	},
	.imu_device_config = {											/*!< The configuration of IMU I2C device. */
		.dev_addr_length	= I2C_ADDR_BIT_LEN_7,					/*!< Use 7-bit address length. */
		.device_address		= 0x00U,								/*!< The default address 0x00U of the IMU I2C device. */
		.scl_speed_hz		= CONFIG_SLIME_I2C_MASTER_FREQUENCY,	/*!< Configurable I2C master device frequency. */
	},
	.mag_device_config = {											/*!< The configuration of magnetometer I2C device. */
		.dev_addr_length	= I2C_ADDR_BIT_LEN_7,					/*!< Use 7-bit address length. */
		.device_address		= 0x00U,								/*!< The default address 0x00U of the magnetometer I2C device. */
		.scl_speed_hz		= CONFIG_SLIME_I2C_MASTER_FREQUENCY,	/*!< Configurable I2C master device frequency. */
	}
};

const slime_lcd_context_config_t lcd_context_config = {
	.spi_host_id = SPI2_HOST, /*!< We choose SPI2 to communicate with the LCD. */
	.spi_bus_config = {																	/*!< The configuration of the SPI Bus. */
		.mosi_io_num		= CONFIG_SLIME_SPI_BUS_MOSI,								/*!< Configurable MOSI GPIO Num. */
		.sclk_io_num		= CONFIG_SLIME_SPI_BUS_SCLK,								/*!< Configurable SCLK GPIO Num. */
		.miso_io_num		= GPIO_NUM_NC,												/*!< ST7735 doesn't need master input. */
		.quadwp_io_num		= GPIO_NUM_NC,												/*!< ST7735 doesn't need write protect. */
		.quadhd_io_num		= GPIO_NUM_NC,												/*!< ST7735 doesn't need hold. */
		.max_transfer_sz	= CONFIG_SLIME_SPI_BUS_MAX_TRANSFER_SZ,						/*!< Configurable maximum transfer size in bytes. */
		.flags				= SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_IOMUX_PINS,	/*!< Make sure the SPI Bus in master mode and uses IO mux rather than GPIO matrix. */
	},
	.spi_dma_channel = SPI_DMA_CH_AUTO, /*!< Let SPI bus select its DMA channel. */
	.panel_io_spi_config = {									/*!< The configuration of the ESP LCD panel SPI IO handle. */
		.cs_gpio_num		= GPIO_NUM_NC,						/*!< We don't need CS because we have only one device attached to the bus. */
		.dc_gpio_num		= CONFIG_SLIME_LCD_DC,				/*!< Configurable DC GPIO Num. */
		.pclk_hz			= CONFIG_SLIME_SPI_BUS_FREQUENCY,	/*!< Configurable SPI Bus frequency. */
		.spi_mode			= 0,								/*!< ST7735 supports SPI mode 0. */
		.trans_queue_depth	= 10,								/*!< Copied from test app of ST7735 ESP LCD Driver. */
		.lcd_cmd_bits		= 8,								/*!< Copied from test app of ST7735 ESP LCD Driver. */
		.lcd_param_bits		= 8,								/*!< Copied from test app of ST7735 ESP LCD Driver. */
	},
	.panel_device_config = {								/*!< The configuration of the ESP LCD panel handle. */
		.reset_gpio_num	= CONFIG_SLIME_LCD_RES,				/*!< Configurable reset GPIO num. */
		.color_space	= ESP_LCD_COLOR_SPACE_RGB,			/*!< Copied from test app of ST7735 ESP LCD Driver. */
		.bits_per_pixel = 16,								/*!< Copied from test app of ST7735 ESP LCD Driver. */
		.vendor_config	= &st7735p3_96x54_vendor_config,	/*!< The vendor configuration of ST7735P3 96x54. */
	},
	.gap_offset_x = 16U,	/*!< The vendor specific gap offset X in pixels of the ST7735P3 96x54. */
	.gap_offset_y = 106U	/*!< The vendor specific gap offset Y in pixels of the ST7735P3 96x54. */
};

const slime_screen_context_config_t screen_context_config = {
	.transmit_task_config = {													/*!< The configuration of the transmission task. */
		.transmit_framerate	= CONFIG_SLIME_SCREEN_TRANSMIT_TASK_FRAMERATE,		/*!< Configurable framerate of the transmission task. */
		.task_core_id		= CONFIG_SLIME_SCREEN_TRANSMIT_TASK_CORE_ID,		/*!< Configurable core ID of the transmission task. */
		.task_stack_depth	= CONFIG_SLIME_SCREEN_TRANSMIT_TASK_STACK_DEPTH,	/*!< Configurable stack depth of the transmission task. */
		.task_priority		= CONFIG_SLIME_SCREEN_TRANSMIT_TASK_PRIORITY,		/*!< Configurable priority of the transmission task. */
		.task_name			= "slime_screen_transmission_task"					/*!< The name of the transmission task. */
	},
	.fast_lcd_panel_config = {							/*!< The configuration of the fast LCD panel */
		.frame_size_x			= 96U,					/*!< Set the width of the screen to the width of the LCD panel in pixels. */
		.frame_size_y			= 54U,					/*!< Set the height of the screen to the height of the LCD panel in pixels. */
		.frame_tile_size_x		= 16U,					/*!< Slice the X axis of the screen into 6 tiles, */
		.frame_tile_size_y		= 18U,					/*!< Slice the Y axis of the screen into 3 tiles, */
		.ring_buffer_slot_count	= 3U,					/*!< Common 3 in-flight transmissions. */
		.buffer_flags			= MALLOC_CAP_INTERNAL,	/*!< Allocate the framebuffer and ring buffer slots in the internal SRAM */
		.name					= "slime_screen"		/*!< The name of the screen used in debugging. */
	},
	.fast_text_engine_instance_config = {						/*!< The configuration of the text engine instance. */
		.crlf_mode				= false,						/*!< We don't use CRLF. */
		.font_size_scale		= 1U,							/*!< We usually use scale 1 text. */
		.font_outline_radius	= 1U,							/*!< 1px outline radius is enough for scale 1. */
		.iram_atlas_slot_count	= 18U,							/*!< The maximum count that can display on the screen is about 18 (3*6). */
		.psram_atlas_slot_count	= 32U,							/*!< L2 cache should be bigger than L1 cache for real-time performance (e.g. switching back to parent view). */
		.string_buffer_size		= 32U,							/*!< 32 is enough for 96x54 screen. */
		.atlas_flags			= 0U,							/*!< No extra flags needed. */
		.name					= "slime_screen_text_engine"	/*!< The name of the text engine. */
	},
	.fast_text_engine_font = {							/*!< The font of the font engine instance. */
		.size_x_max		= 16U,							/*!< The height of the glyphs of Unifont is 16 in pixels. */
		.size_y			= 16U,							/*!< The maximum width of the glyphs of Unifont is 16 in pixels. */
		.glyph_data		= unifont_17_0_05_1bpp_bits,	/*!< The tightly packed 1bpp LSBit-first font glyph data of the modified version of the Unifont 17.0.05. */
		.glyph_table	= glyph_table					/*!< The lookup-table of the font. */
	}
};

const slime_button_context_config_t button_context_config = {
	.return_button_config = {										/*!< The configuration of the return button */
		.button_config = {											/*!< The IOT button configuration of the return button */
			.long_press_time	= 0,								/*!< We don't need long press for now. */
			.short_press_time	= CONFIG_SLIME_BUTTON_CLICK_TIME	/*!< Configurable short press time in milliseconds. */
		},
		.button_gpio_config = {										/*!< The GPIO configuration of the return button */
			.gpio_num			= CONFIG_SLIME_BUTTON_RETURN,		/*!< Configurable GPIO Num of the return button. */
			.active_level		= CONFIG_SLIME_BUTTON_ACTIVE_LEVEL,	/*!< Configurable GPIO level when the button is pressed down. */
			.enable_power_save	= false,							/*!< We don't need power saving. */
			#ifdef SLIME_BUTTON_DISABLE_INTERNAL_PULL				/*!< Configurable internal pull-up/pull-down. */
				.disable_pull	= true
			#endif // SLIME_BUTTON_DISABLE_INTERNAL_PULL
		}
	},
	.switch_button_config = {										/*!< The configuration of the switch button */
		.button_config = {											/*!< The IOT button configuration of the switch button */
			.long_press_time	= 0,								/*!< We don't need long press for now. */
			.short_press_time	= CONFIG_SLIME_BUTTON_CLICK_TIME	/*!< Configurable short press time in milliseconds. */
		},
		.button_gpio_config = {										/*!< The GPIO configuration of the switch button */
			.gpio_num			= CONFIG_SLIME_BUTTON_SWITCH,		/*!< Configurable GPIO Num of the switch button. */
			.active_level		= CONFIG_SLIME_BUTTON_ACTIVE_LEVEL,	/*!< Configurable GPIO level when the button is pressed down. */
			.enable_power_save	= false,							/*!< We don't need power saving. */
			#ifdef SLIME_BUTTON_DISABLE_INTERNAL_PULL				/*!< Configurable internal pull-up/pull-down. */
				.disable_pull	= true
			#endif // SLIME_BUTTON_DISABLE_INTERNAL_PULL
		}
	},
	.confirm_button_config = {										/*!< The configuration of the confirm button */
		.button_config = {											/*!< The IOT button configuration of the confirm button */
			.long_press_time	= 0,								/*!< We don't need long press for now. */
			.short_press_time	= CONFIG_SLIME_BUTTON_CLICK_TIME	/*!< Configurable short press time in milliseconds. */
		},
		.button_gpio_config = {										/*!< The GPIO configuration of the confirm button */
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

const slime_magneto_context_config_t slime_magneto_context_config = {
	.calibration_coefficients_key = {								/*!< The key of the coefficients of the magneto context. */
		.name = "mag_coeff",										/*!< The name of the key. */
		.init = &	slime_magneto_calibration_coefficients_default,	/*!< The default value of the serialized coefficients. */
		.size =		slime_magneto_calibration_coefficients_size		/*!< The size of the serialized coefficient struct */
	}
};

const slime_sensor_context_type_t slime_sensor_context_type_table[8] = {
	{															/*!< The sensor type of sensor board ID 000 (Currently empty). */
		.sensor_context_new	= slime_empty_sensor_context_new,	/*!< The sensor context creation function handle of the empty sensor context. */
		.config				= NULL,								/*!< No configuration for empty sensor context. */
		.name				= "empty_sensor_id_000"				/*!< The default name of the empty sensor. */
	},
	{																		/*!< The sensor type of sensor board ID 001 (LSM6DSV+MMC5603). */
		.sensor_context_new	= slime_lsm6dsv_mmc5603_sensor_context_new,		/*!< The sensor context creation function handle of the LSM6DSV+MMC5603 sensor context. */
		.config				= &slime_lsm6dsv_mmc5603_sensor_context_config,	/*!< Configuration of the LSM6DSV+MMC5603 sensor context. */
		.name				= "lsm6dsv_mmc5603_sensor"						/*!< The name of the LSM6DSV+MMC5603 sensor. */
	},
	{																		/*!< The sensor type of sensor board ID 010 (LSM6DSV+QMC6309). */
		.sensor_context_new	= slime_lsm6dsv_qmc6309_sensor_context_new,		/*!< The sensor context creation function handle of the LSM6DSV+QMC6309 sensor context. */
		.config				= &slime_lsm6dsv_qmc6309_sensor_context_config,	/*!< Configuration of the LSM6DSV+QMC6309 sensor context. */
		.name				= "lsm6dsv_qmc6309_sensor"						/*!< The name of the LSM6DSV+QMC6309 sensor. */
	},
	{															/*!< The sensor type of sensor board ID 011 (Currently empty). */
		.sensor_context_new	= slime_empty_sensor_context_new,	/*!< The sensor context creation function pointer of the empty sensor context. */
		.config				= NULL,								/*!< No configuration for empty sensor context. */
		.name				= "empty_sensor_id_000"				/*!< The default name of the empty sensor. */
	},
	{															/*!< The sensor type of sensor board ID 100 (Currently empty). */
		.sensor_context_new	= slime_empty_sensor_context_new,	/*!< The sensor context creation function pointer of the empty sensor context. */
		.config				= NULL,								/*!< No configuration for empty sensor context. */
		.name				= "empty_sensor_id_000"				/*!< The default name of the empty sensor. */
	},
	{															/*!< The sensor type of sensor board ID 101 (Currently empty). */
		.sensor_context_new	= slime_empty_sensor_context_new,	/*!< The sensor context creation function pointer of the empty sensor context. */
		.config				= NULL,								/*!< No configuration for empty sensor context. */
		.name				= "empty_sensor_id_000"				/*!< The default name of the empty sensor. */
	},
	{															/*!< The sensor type of sensor board ID 110 (Currently empty). */
		.sensor_context_new	= slime_empty_sensor_context_new,	/*!< The sensor context creation function pointer of the empty sensor context. */
		.config				= NULL,								/*!< No configuration for empty sensor context. */
		.name				= "empty_sensor_id_000"				/*!< The default name of the empty sensor. */
	},
	{															/*!< The sensor type of sensor board ID 111 (Currently empty). */
		.sensor_context_new	= slime_empty_sensor_context_new,	/*!< The sensor context creation function pointer of the empty sensor context. */
		.config				= NULL,								/*!< No configuration for empty sensor context. */
		.name				= "empty_sensor_id_000"				/*!< The default name of the empty sensor. */
	}
};

const slime_fusion_context_type_t slime_fusion_context_type_table[1] = {
	{														/*!< The fusion context type of VQF. */
		.fusion_context_new = slime_vqf_fusion_context_new,	/*!< The fusion context creation function handle of VQF fusion context. */
		.enu_to_fusion_quaternion = {	/*!< VQF is already ENU frame. */
			1.0f,						/*!< W = 1.0f. */
			0.0f,						/*!< X = 0.0f. */
			0.0f,						/*!< Y = 0.0f. */
			0.0f						/*!< Z = 0.0f. */
		},
		.gyroscope_mdps_to_fusion_scale		= 0.00001745329251994330f,	/*!< Scale factor to scale mdps to rad/s. */
		.accelerometer_mg_to_fusion_scale	= 0.00980665f,				/*!< Scale factor to scale mg to m/s^2. */
		.magnetometer_gauss_to_fusion_scale	= 1.0f,						/*!< VQF support magnetometer measurement in arbitrary units. */
		.timestamp_seconds_to_fusion_scale	= 0.0f,						/*!< VQF does not require timestamp. */
		.name								= "VQF",					/*!< The name of the VQF fusion. */
		.config								= &vqf_params				/*!< Use default VQF parameters. */
	}
};

const slime_network_context_config_t network_context_config = {
	.network_tick_config = {											/*!< The network tick context configuration. */
		.socket_context_config.bind_address = {							/*!< The bind address of the UDP context configuration. */
			.addr = INADDR_ANY,											/*!< Bind to local address. */
			.port = htons(CONFIG_SLIME_NETWORK_TICK_UDP_SOCKET_PORT),	/*!< Configurable port of the UDP socket. */
		},
		.buffer_context_config = {								/*!< The IO context configuration of the network tick context. */
			.length	= CONFIG_SLIME_NETWORK_TICK_BUFFER_SIZE,	/*!< Configurable size of the buffer to send or receive the packet. */
			.flags	= MALLOC_CAP_DEFAULT,						/*!< We don't need extra heap capabilities. */
		},
		.handshake_packet_period	= CONFIG_SLIME_NETWORK_TICK_HANDSHAKE_PACKET_PERIOD,	/*!< Configurable handshake packet sending period of the network tick. */
		.info_packet_period			= CONFIG_SLIME_NETWORK_TICK_INFO_PACKET_PERIOD,			/*!< Configurable config packet sending period of the network tick. */
		.keep_alive_period			= CONFIG_SLIME_NETWORK_TICK_KEEP_ALIVE_PERIOD,			/*!< Configurable keep alive period of the network tick if no server packet received. */
		.loop_period				= CONFIG_SLIME_NETWORK_TICK_TASK_LOOP_PERIOD,			/*!< Configurable loop cooldown period of the network tick when error occurred. */
		.tick_rate					= CONFIG_SLIME_NETWORK_TICK_TASK_TICK_RATE				/*!< Configurable tick rate of the network tick. */
	},
	.network_tick_task_config = {											/*!< The network tick task configuration. */
		.task_core_id		= CONFIG_SLIME_NETWORK_TICK_TASK_CORE_ID,		/*!< Configurable core ID of the network task. */
		.task_stack_depth	= CONFIG_SLIME_NETWORK_TICK_TASK_STACK_DEPTH,	/*!< Configurable stack depth of the network task. */
		.task_priority		= CONFIG_SLIME_NETWORK_TICK_TASK_PRIORITY,		/*!< Configurable priority of the network task. */
		.task_name			= "slime_network_udp_task"						/*!< The name of the network task. */
	}
};