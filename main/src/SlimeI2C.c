#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "SlimeI2C.h"

#include <esp_check.h>

#include "lsm6dsv_reg.h"

static const_string TAG = "SlimeI2C";

// The I2C master bus configuration.
const static i2c_master_bus_config_t I2CMasterBusConfig = {
	.clk_source			= I2C_CLK_SRC_DEFAULT,		// Use default I2C clock source.
	.i2c_port			= I2C_NUM_0,				// Use I2C0.
	.scl_io_num			= CONFIG_I2C_MASTER_SCL,	// Configurable SCL GPIO Num through menuconfig.
	.sda_io_num			= CONFIG_I2C_MASTER_SDA,	// Configurable SDA GPIO Num through menuconfig.
	.glitch_ignore_cnt	= 7,						// Typical value of glitch period ignore count.
	.flags				= {
		.enable_internal_pullup = true				// Use internal pull-up.
	}
};

// The I2C master device configuration of the LSM6DSV.
const static i2c_device_config_t LSM6DSV_I2CDeviceConfig = {
	.dev_addr_length	= I2C_ADDR_BIT_LEN_7,
	.device_address		= LSM6DSV_I2C_ADD_L,
	.scl_speed_hz		= CONFIG_I2C_MASTER_FREQUENCY
};

// The I2C master device configuration of the QMC6309.
const static i2c_device_config_t QMC6309_I2CDeviceConfig = {
	.dev_addr_length	= I2C_ADDR_BIT_LEN_7,
	.device_address		= 0x7CU,
	.scl_speed_hz		= CONFIG_I2C_MASTER_FREQUENCY
};

// The I2C master device names.
static const_string LSM6DSV_name = "LSM6DSV";
static const_string QMC6309_name = "QMC6309";

// The ESP32 series implementation function of platform independent I2C write function.
s32 platformI2CWrite(opaque userHandle, u8 registerAddress, const_ptr(u8) buffer, u16 length) {
	// Describe all buffers we need to send through I2C.
	i2c_master_transmit_multi_buffer_info_t buffers[2] = {
		{.write_buffer = ref(registerAddress),	.buffer_size = 1		},	// The first buffer is the register address. It tells the device the register we are going to write data into.
		{.write_buffer = buffer,				.buffer_size = length	},	// The second buffer is the actual data buffer.
	};

	// Cast the opaque user handle to I2C device context.
	const ptr(I2CDeviceContext) deviceContext = cast_ptr(I2CDeviceContext, userHandle);

	// Log the operation if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C master bus is writing %" PRIu16 " byte(s) register 0x%02" PRIX8 " on device 0x%02" PRIX8 " (%s).\n",
			/* PRIu16	*/ length,
			/* PRIX8	*/ registerAddress,
			/* PRIX8	*/ deviceContext->I2CDeviceAddress,
			/* s		*/ deviceContext->I2CDeviceName
		);
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Send all buffers through I2C to the device indicated by user_handle.
	return cast_to(s32, i2c_master_multi_buffer_transmit(
		/* i2c_device	= */ deviceContext->I2CDeviceHandle,
		/* buffer_array	= */ buffers,
		/* buffer_size	= */ 2,
		/* timeout_ms	= */ -1
	));
}

// The ESP32 series implementation function of platform independent I2C read function.
s32 platformI2CRead(opaque userHandle, u8 registerAddress, ptr(u8) buffer, u16 length) {
	// Cast the opaque user handle to I2C device context.
	const ptr(I2CDeviceContext) deviceContext = cast_ptr(I2CDeviceContext, userHandle);

	// Log the operation if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C master bus is reading %" PRIu16 " byte(s) register 0x%02" PRIX8 " on device 0x%02" PRIX8 " (%s).\n",
			/* PRIu16	*/ length,
			/* PRIX8	*/ registerAddress,
			/* PRIX8	*/ deviceContext->I2CDeviceAddress,
			/* %s		*/ deviceContext->I2CDeviceName
		);
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Send the register address to the device indicated by user_handle then read from the device.
	return cast_to(s32, i2c_master_transmit_receive(
		/* i2c_device	= */ deviceContext->I2CDeviceHandle,
		/* write_buffer	= */ ref(registerAddress),
		/* write_size	= */ 1,
		/* read_buffer	= */ buffer,
		/* read_size	= */ length,
		/* timeout_ms	= */ -1
	));
}

// The ESP32 series implementation function of platform independent I2C delay function.
void platformDelay(u32 milliseconds) {
	// Log the operation if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C master bus is delayed by %" PRIu32 " millisecond(s).\n", milliseconds);
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Delay.
	vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

esp_err_t newI2CRuntimeContext(ptr(I2CRuntimeContext) I2CRuntimeContextOut) {
	// Log the operation if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating I2C runtime context.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Create the I2C master bus using the master bus configuration.
	ESP_RETURN_ON_ERROR(i2c_new_master_bus(ref(I2CMasterBusConfig), ref(I2CRuntimeContextOut->I2CMasterBus)), TAG, "Failed to create I2C master bus.");

	// Create the I2C master device context.
	ptr(I2CDeviceContext) LSM6DSV_I2CDeviceContext = alloc_ptr(I2CDeviceContext);
	ptr(I2CDeviceContext) QMC6309_I2CDeviceContext = alloc_ptr(I2CDeviceContext);

	// Fill the I2C master device context.
	LSM6DSV_I2CDeviceContext->I2CDeviceAddress	= LSM6DSV_I2C_ADD_L;
	QMC6309_I2CDeviceContext->I2CDeviceAddress	= 0x7CU;
	LSM6DSV_I2CDeviceContext->I2CDeviceName		= LSM6DSV_name;
	QMC6309_I2CDeviceContext->I2CDeviceName		= QMC6309_name;

	// Create the I2C master devices using the master device configurations then fill the handles into device contexts.
	ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(I2CRuntimeContextOut->I2CMasterBus, &LSM6DSV_I2CDeviceConfig, ref(LSM6DSV_I2CDeviceContext->I2CDeviceHandle)), TAG, "Failed to create I2C master device for LSM6DSV."); // Create the I2C master device for LSM6DSV.
	ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(I2CRuntimeContextOut->I2CMasterBus, &QMC6309_I2CDeviceConfig, ref(QMC6309_I2CDeviceContext->I2CDeviceHandle)), TAG, "Failed to create I2C master device for QMC6309."); // Create the I2C master device for QMC6309.

	// Fill the runtime context.
	I2CRuntimeContextOut->LSM6DSV_I2CDeviceContext	= LSM6DSV_I2CDeviceContext;
	I2CRuntimeContextOut->QMC6309_I2CDeviceContext	= QMC6309_I2CDeviceContext;
	I2CRuntimeContextOut->platformI2CWriteFunc		= platformI2CWrite;
	I2CRuntimeContextOut->platformI2CReadFunc		= platformI2CRead;
	I2CRuntimeContextOut->platformDelayFunc			= platformDelay;

	// Log the operation if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C runtime context created.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	return ESP_OK;
}