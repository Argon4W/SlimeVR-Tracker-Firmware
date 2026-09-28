#include "stdlib.h"
#include "esp_log.h"
#include "lsm6dsv_reg.h"
#include "qmc6309_reg.h"
#include "slime_i2c.h"

/**
 * @brief The log tag of the Slime I2C.
 */
static const char* TAG = "slime_i2c";

int32_t slime_i2c_write_register(
			void*		i2c_device_handle,
			uint8_t		register_address,
	const	uint8_t*	write_buffer,
			uint16_t	write_length
) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(i2c_device_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "No i2c_device_handle provided when performing writing to registers.");

	// Describe all buffers we are going to send to given I2C device.
	i2c_master_transmit_multi_buffer_info_t buffers[2] = {
		{
			// First buffer is the register address. Tell the device which register we are going to write data into.
			.write_buffer	= &register_address,
			.buffer_size	= 1
		},
		{
			// Second buffer is the actual data buffer to write into the register.
			.write_buffer	= write_buffer,
			.buffer_size	= write_length
		},
	};
	// Log the operation if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C context is performing writing %" PRIu16 " byte(s) to register 0x%02" PRIX8 ".",
			/* PRIu16	*/ write_length,
			/* PRIX8	*/ register_address
		);
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Cast the opaque pointer to I2C master device handle.
	const i2c_master_dev_handle_t device_handle = (i2c_master_dev_handle_t) i2c_device_handle;

	// Send all buffers to the given I2C device.
	ESP_RETURN_ON_ERROR(i2c_master_multi_buffer_transmit(
		/* i2c_device	= */ device_handle,
		/* buffer_array	= */ buffers,
		/* buffer_size	= */ 2,
		/* timeout_ms	= */ -1
	), TAG, "Failed to send buffers to I2C device.");

	return (int32_t) ESP_OK;
}

int32_t slime_i2c_read_register(
	void*		i2c_device_handle,
	uint8_t		register_address,
	uint8_t*	read_buffer,
	uint16_t	read_length
) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(i2c_device_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "No i2c_device_handle provided when performing writing to registers.");

	// Log the operation if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C context is performing reading %" PRIu16 " byte(s) from register 0x%02" PRIX8 ".",
			/* PRIu16	*/ read_length,
			/* PRIX8	*/ register_address
		);
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Cast the opaque pointer to I2C master device handle.
	const i2c_master_dev_handle_t device_handle = (i2c_master_dev_handle_t) i2c_device_handle;

	// Send the register address to the given I2C device then read data from the device.
	ESP_RETURN_ON_ERROR(i2c_master_transmit_receive(
		/* i2c_device	= */ device_handle,
		/* write_buffer	= */ &register_address,
		/* write_size	= */ 1,
		/* read_buffer	= */ read_buffer,
		/* read_size	= */ read_length,
		/* timeout_ms	= */ -1
	), TAG, "Failed to read buffers from I2C device.");

	return (int32_t) ESP_OK;
}

esp_err_t slime_i2c_set_device_address(
	const	slime_i2c_context_t*	i2c_context,
			uint8_t					imu_device_address,
			uint8_t					mag_device_address
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(i2c_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No i2c_context provided when performing setting device addresses.");

	// Log the operation if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C context is setting IMU address to 0x%02" PRIX8 ", Magnetometer address to 0x%02" PRIX8 ".",
			/* PRIX8 */ imu_device_address,
			/* PRIX8 */ mag_device_address
		);
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Set the I2C device addresses of the
	ESP_RETURN_ON_ERROR(i2c_master_device_change_address(i2c_context->imu_device_handle, imu_device_address, -1), TAG, "Failed to set IMU I2C device address.");
	ESP_RETURN_ON_ERROR(i2c_master_device_change_address(i2c_context->mag_device_handle, mag_device_address, -1), TAG, "Failed to set Magnetometer I2C device address.");

	return ESP_OK;
}

esp_err_t slime_i2c_context_new(
			slime_i2c_context_t**		i2c_context_out,
	const	slime_i2c_context_config_t*	i2c_context_config
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration and a handle to receive the created I2C context.
	ESP_RETURN_ON_FALSE(i2c_context_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No i2c_context_out handle provided when creating I2C context.");
	ESP_RETURN_ON_FALSE(i2c_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_i2c_context_config_t handle provided when creating I2C context.");

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating I2C context.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Reserving handles of I2C context.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Reserve the handles for I2C context.
	slime_i2c_context_t*	i2c_context			= NULL;
	i2c_master_bus_handle_t	context_master_bus	= NULL;
	i2c_master_dev_handle_t	context_imu_device	= NULL;
	i2c_master_dev_handle_t	context_mag_device	= NULL;

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating I2C master bus.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Create the I2C master bus using the master bus configuration.
	ESP_GOTO_ON_ERROR(i2c_new_master_bus(
		/* bus_config		= */ &i2c_context_config->i2c_master_bus_config,
		/* ret_bus_handle	= */ &context_master_bus
	), error, TAG, "Failed to create I2C master bus.");

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Adding I2C master devices.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Create the I2C master devices using the master device configurations then fill the handles into device contexts.
	ESP_GOTO_ON_ERROR(i2c_master_bus_add_device(context_master_bus, &i2c_context_config->imu_device_config, &context_imu_device), error, TAG, "Failed to add I2C master device of IMU.");
	ESP_GOTO_ON_ERROR(i2c_master_bus_add_device(context_master_bus, &i2c_context_config->mag_device_config, &context_mag_device), error, TAG, "Failed to add I2C master device of magnetometer.");

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating I2C context struct.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Create the I2C context struct handle.
	i2c_context = calloc(1, sizeof(slime_i2c_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(i2c_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create the I2C context struct.");

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Finalizing I2C context.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Fill the I2C context.
	i2c_context->i2c_master_bus		= context_master_bus;
	i2c_context->imu_device_handle	= context_imu_device;
	i2c_context->mag_device_handle	= context_mag_device;

	// Return the created I2C context.
	*i2c_context_out = i2c_context;

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C context has been created.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred: %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Cleaning up resources.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	if (i2c_context)						free					(i2c_context);			// Cleanup the I2C context struct.
	if (context_mag_device)	ESP_ERROR_CHECK(i2c_master_bus_rm_device(context_mag_device));	// Cleanup the I2C master device handle of Magnetometer.
	if (context_imu_device)	ESP_ERROR_CHECK(i2c_master_bus_rm_device(context_imu_device));	// Cleanup the I2C master device handle of IMU.
	if (context_master_bus)	ESP_ERROR_CHECK(i2c_del_master_bus		(context_master_bus));	// Cleanup the I2C master bus.

	return ret;
}

esp_err_t slime_i2c_context_del(slime_i2c_context_t* i2c_context_in) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(i2c_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No i2c_context_out handle provided when releasing I2C context.");

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing I2C context.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Removing I2C master devices.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Remove all I2C master devices.
	ESP_RETURN_ON_ERROR(i2c_master_bus_rm_device(i2c_context_in->imu_device_handle), TAG, "Failed to remove I2C master device of IMU.");
	ESP_RETURN_ON_ERROR(i2c_master_bus_rm_device(i2c_context_in->mag_device_handle), TAG, "Failed to remove I2C master device of Magnetometer.");

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing I2C master bus.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Release the I2C master bus.
	ESP_RETURN_ON_ERROR(i2c_del_master_bus(i2c_context_in->i2c_master_bus), TAG, "Failed to release I2C master bus.");

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Detaching all fields of the I2C context.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Detaching all fields.
	i2c_context_in->i2c_master_bus		= NULL;
	i2c_context_in->imu_device_handle	= NULL;
	i2c_context_in->mag_device_handle	= NULL;

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing I2C context struct.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	// Free the I2C context struct.
	free(i2c_context_in);

	// Log the progress if I2C master debug logging is enabled.
	#ifdef CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING
		ESP_LOGD(TAG, "I2C context has been deleted.");
	#endif // CONFIG_SLIME_I2C_MASTER_DEBUG_LOGGING

	return ESP_OK;
}