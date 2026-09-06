#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lsm6dsv_reg.h"
#include "qmc6309_reg.h"
#include "SlimeI2C.h"

static const_string TAG = "SlimeI2C";

// The I2C master bus configuration.
const static i2c_master_bus_config_t i2cMasterBusConfig = {
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
const static i2c_device_config_t lsm6dsv_I2CDeviceConfig = {
	.dev_addr_length	= I2C_ADDR_BIT_LEN_7,
	.device_address		= LSM6DSV_I2C_ADD_L >> 1,
	.scl_speed_hz		= CONFIG_I2C_MASTER_FREQUENCY
};

// The I2C master device configuration of the QMC6309.
const static i2c_device_config_t qmc6309_I2CDeviceConfig = {
	.dev_addr_length	= I2C_ADDR_BIT_LEN_7,
	.device_address		= QMC6309_I2C_ADDRESS,
	.scl_speed_hz		= CONFIG_I2C_MASTER_FREQUENCY
};

// The I2C master device names.
static const_string lsm6dsvName = "LSM6DSV";
static const_string qmc6309Name = "QMC6309";

// The ESP32 series implementation function of platform independent I2C write function.
s32 platformI2CWrite(
			opaque	userHandle,
			u8		registerAddress,
	const	ptr(u8)	buffer,
			u16		length
) {
	// We cannot proceed without userHandle.
	if (userHandle == NULL) {
		// Log the error if I2C master debug logging is enabled.
		#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
			ESP_LOGE(TAG, "No I2CDeviceContext provided when performing writes to registers.");
		#endif
		return cast_to(s32, ESP_ERR_INVALID_ARG);
	}

	// Describe all buffers we need to send through I2C.
	i2c_master_transmit_multi_buffer_info_t buffers[2] = {
		{.write_buffer = ref(registerAddress),	.buffer_size = 1		},	// The first buffer is the register address. It tells the device the register we are going to write data into.
		{.write_buffer = buffer,				.buffer_size = length	},	// The second buffer is the actual data buffer.
	};

	// Cast the opaque user handle to I2C device context.
	const ptr(I2CDeviceContext) deviceContext = cast_ptr(I2CDeviceContext, userHandle);

	// Log the operation if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C master bus is performing a write of %" PRIu16 " byte(s) to register 0x%02" PRIX8 " on device 0x%02" PRIX8 " (%s).",
			/* PRIu16	*/ length,
			/* PRIX8	*/ registerAddress,
			/* PRIX8	*/ deviceContext->deviceAddress,
			/* s		*/ deviceContext->deviceName
		);
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Send all buffers through I2C to the device indicated by user_handle.
	return cast_to(s32, i2c_master_multi_buffer_transmit(
		/* i2c_device	= */ deviceContext->deviceHandle,
		/* buffer_array	= */ buffers,
		/* buffer_size	= */ 2,
		/* timeout_ms	= */ -1
	));
}

// The ESP32 series implementation function of platform independent I2C read function.
s32 platformI2CRead(
	opaque	userHandle,
	u8		registerAddress,
	ptr(u8)	buffer,
	u16		length
) {
	// We cannot proceed without userHandle.
	if (userHandle == NULL) {
		#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
			ESP_LOGE(TAG, "No I2CDeviceContext provided when performing reads from registers.");
		#endif
		return cast_to(s32, ESP_ERR_INVALID_ARG);
	}

	// Cast the opaque user handle to I2C device context.
	const ptr(I2CDeviceContext) deviceContext = cast_ptr(I2CDeviceContext, userHandle);

	// Log the operation if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C master bus is performing a read of %" PRIu16 " byte(s) from register 0x%02" PRIX8 " on device 0x%02" PRIX8 " (%s).",
			/* PRIu16	*/ length,
			/* PRIX8	*/ registerAddress,
			/* PRIX8	*/ deviceContext->deviceAddress,
			/* s		*/ deviceContext->deviceName
		);
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Send the register address to the device indicated by user_handle then read from the device.
	return cast_to(s32, i2c_master_transmit_receive(
		/* i2c_device	= */ deviceContext->deviceHandle,
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
		ESP_LOGD(TAG, "I2C master bus is delayed by %" PRIu32 " millisecond(s).", milliseconds);
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Delay.
	vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

esp_err_t newI2CRuntimeContext(ptr(I2CRuntimeContext) i2cRuntimeContextOut) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without an allocated context.
	ESP_RETURN_ON_FALSE(i2cRuntimeContextOut != NULL, ESP_ERR_INVALID_ARG, TAG, "No I2CRuntimeContext provided when initializing I2C runtime context.");

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating I2C runtime context.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Reserve the I2C master bus handle.
	i2c_master_bus_handle_t i2cMasterBus = NULL;

	// Reserve the I2C device contexts.
	ptr(I2CDeviceContext) lsm6dsv_I2CDeviceContext = NULL;
	ptr(I2CDeviceContext) qmc6309_I2CDeviceContext = NULL;

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating I2C master bus.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Create the I2C master bus using the master bus configuration.
	ESP_GOTO_ON_ERROR(i2c_new_master_bus(ref(i2cMasterBusConfig), ref(i2cMasterBus)), error, TAG, "Failed to create I2C master bus.");

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Allocating I2C device contexts.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Allocate the I2C device contexts.
	lsm6dsv_I2CDeviceContext = alloc_ptr(I2CDeviceContext);
	qmc6309_I2CDeviceContext = alloc_ptr(I2CDeviceContext);

	// Check if the device contexts is allocated.
	ESP_GOTO_ON_FALSE((lsm6dsv_I2CDeviceContext != NULL), ESP_ERR_NO_MEM, error, TAG, "Failed to create I2C device context for LSM6DSV");
	ESP_GOTO_ON_FALSE((qmc6309_I2CDeviceContext != NULL), ESP_ERR_NO_MEM, error, TAG, "Failed to create I2C device context for QMC6309");

	// Fill the I2C device context.
	lsm6dsv_I2CDeviceContext->deviceAddress	= LSM6DSV_I2C_ADD_L;
	qmc6309_I2CDeviceContext->deviceAddress	= 0x7CU;
	lsm6dsv_I2CDeviceContext->deviceName	= lsm6dsvName;
	qmc6309_I2CDeviceContext->deviceName	= qmc6309Name;

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating I2C master device handles.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Create the I2C master devices using the master device configurations then fill the handles into device contexts.
	ESP_GOTO_ON_ERROR(i2c_master_bus_add_device(i2cMasterBus, &lsm6dsv_I2CDeviceConfig, ref(lsm6dsv_I2CDeviceContext->deviceHandle)), error, TAG, "Failed to create I2C master device for LSM6DSV."); // Create the I2C master device for LSM6DSV.
	ESP_GOTO_ON_ERROR(i2c_master_bus_add_device(i2cMasterBus, &qmc6309_I2CDeviceConfig, ref(qmc6309_I2CDeviceContext->deviceHandle)), error, TAG, "Failed to create I2C master device for QMC6309."); // Create the I2C master device for QMC6309.

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Finalizing I2C master device context.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// No error occurred.
	// It is safe to fill the output runtime context now.
	i2cRuntimeContextOut->i2cMasterBus				= i2cMasterBus;
	i2cRuntimeContextOut->lsm6dsv_I2CDeviceContext	= lsm6dsv_I2CDeviceContext;
	i2cRuntimeContextOut->qmc6309_I2CDeviceContext	= qmc6309_I2CDeviceContext;
	i2cRuntimeContextOut->platformI2CWrite			= platformI2CWrite;
	i2cRuntimeContextOut->platformI2CRead			= platformI2CRead;
	i2cRuntimeContextOut->platformDelay				= platformDelay;

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C runtime context created.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	return ret;

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred: %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Cleaning up resources.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Resource cleanup when error occurred.
	error:

	if (qmc6309_I2CDeviceContext && qmc6309_I2CDeviceContext->deviceHandle)	ESP_ERROR_CHECK(i2c_master_bus_rm_device(qmc6309_I2CDeviceContext->deviceHandle));	// Cleanup the I2C master device handle of QMC6309.
	if (lsm6dsv_I2CDeviceContext && lsm6dsv_I2CDeviceContext->deviceHandle)	ESP_ERROR_CHECK(i2c_master_bus_rm_device(lsm6dsv_I2CDeviceContext->deviceHandle));	// Cleanup the I2C master device handle of LSM6DSV.
	if (qmc6309_I2CDeviceContext)															free					(qmc6309_I2CDeviceContext);					// Cleanup the I2C device context of QMC6309.
	if (lsm6dsv_I2CDeviceContext)															free					(lsm6dsv_I2CDeviceContext);					// Cleanup the I2C device context of LSM6DSV.
	if (i2cMasterBus)														ESP_ERROR_CHECK(i2c_del_master_bus		(i2cMasterBus));							// Cleanup the I2C master bus.

	return ret;
}

esp_err_t deleteI2CRuntimeContext(ptr(I2CRuntimeContext) i2cRuntimeContextIn) {
	// We cannot proceed without an initialized context.
	ESP_RETURN_ON_FALSE(i2cRuntimeContextIn != NULL, ESP_ERR_INVALID_ARG, TAG, "No I2CRuntimeContext provided when deleting I2C runtime context.");

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Deleting I2C runtime context.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Dereferencing all fields that needs to be cleaned up.
	i2c_master_bus_handle_t	i2cMasterBus				= i2cRuntimeContextIn->i2cMasterBus;
	ptr(I2CDeviceContext)	lsm6dsv_I2CDeviceContext	= i2cRuntimeContextIn->lsm6dsv_I2CDeviceContext;
	ptr(I2CDeviceContext)	qmc6309_I2CDeviceContext	= i2cRuntimeContextIn->qmc6309_I2CDeviceContext;

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Deleting I2C handles.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Cleanup all ESP I2C master driver related handles.
	ESP_RETURN_ON_ERROR(i2c_master_bus_rm_device(lsm6dsv_I2CDeviceContext->deviceHandle),	TAG, "Failed to remove I2C master device of LSM6DSV from the I2C master bus.");	// Cleanup the I2C master device handle of LSM6DSV.
	ESP_RETURN_ON_ERROR(i2c_master_bus_rm_device(qmc6309_I2CDeviceContext->deviceHandle),	TAG, "Failed to remove I2C master device of QMC6309 from the I2C master bus.");	// Cleanup the I2C master device handle of QMC6309.
	ESP_RETURN_ON_ERROR(i2c_del_master_bus		(i2cMasterBus),								TAG, "Failed to delete I2C master bus.");										// Cleanup the I2C master bus.

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Deleting I2C device contexts.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Free all I2C device contexts allocations.
	free(qmc6309_I2CDeviceContext);
	free(lsm6dsv_I2CDeviceContext);

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Detaching all fields in I2C runtime context.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	// Detaching I2C master bus.
	i2cRuntimeContextIn->i2cMasterBus = NULL;

	// Detaching all I2C device contexts.
	i2cRuntimeContextIn->lsm6dsv_I2CDeviceContext = NULL;
	i2cRuntimeContextIn->qmc6309_I2CDeviceContext = NULL;

	// Detaching all implementation functions.
	i2cRuntimeContextIn->platformI2CWrite	= NULL;
	i2cRuntimeContextIn->platformI2CRead	= NULL;
	i2cRuntimeContextIn->platformDelay		= NULL;

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C runtime context deleted.");
	#endif // CONFIG_I2C_MASTER_DEBUG_LOGGING

	return ESP_OK;
}