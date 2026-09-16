#ifndef SLIME_I2C_H
#define SLIME_I2C_H

#include "esp_check.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The configuration struct of the I2C context.
 */
typedef struct {
	i2c_master_bus_config_t	i2c_master_bus_config;	/*!< The master bus configuration of the I2C. */
	i2c_device_config_t		i2c_imu_device_config;	/*!< The I2C master device configuration of the IMU. */
	i2c_device_config_t		i2c_mag_device_config;	/*!< The I2C master device configuration of the Magnetometer. */
} slime_i2c_context_config_t;

/**
 * @brief The I2C context struct.
 */
typedef struct {
	i2c_master_bus_handle_t i2c_master_bus;			/*!< The I2C master bus handle. */
	i2c_master_dev_handle_t i2c_imu_device_handle;	/*!< I2C master device handle for IMU. */
	i2c_master_dev_handle_t i2c_mag_device_handle;	/*!< I2C master device handle for magnetometer. */
} slime_i2c_context_t;

/**
 * @brief					Write data to a register of given I2C device handle, compatible to ST's PID driver format.
 * @param i2c_device_handle	The I2C device to write the data into.
 * @param register_address	The register to write the data into.
 * @param write_buffer		The buffer of the data to write.
 * @param write_length		The length of the data to write.
 * @return					The status of the data write.
 */
int32_t slime_i2c_write_register(
			void*		i2c_device_handle,
			uint8_t		register_address,
	const	uint8_t*	write_buffer,
			uint16_t	write_length
);

/**
 * @brief					Read data from a register of given I2C device handle, compatible to ST's PID driver format.
 * @param i2c_device_handle	The I2C device to read the data from.
 * @param register_address	The register to read the data from.
 * @param read_buffer		The buffer of the data to read.
 * @param read_length		The length of the data to read.
 * @return					The status of the data read.
 */
int32_t slime_i2c_read_register(
	void*		i2c_device_handle,
	uint8_t		register_address,
	uint8_t*	read_buffer,
	uint16_t	read_length
);

/**
 * @brief						Create the I2C context.
 * @param i2c_context_out		The handle to receive the created I2C context.
 * @param i2c_context_config	The configuration of the I2C context.
 * @return						The status of the creation.
 */
esp_err_t slime_i2c_context_new(
			slime_i2c_context_t**		i2c_context_out,
	const	slime_i2c_context_config_t*	i2c_context_config
);

/**
 * @brief					Release the I2C context.
 * @param i2c_context_in	The I2C context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_i2c_context_del(slime_i2c_context_t* i2c_context_in);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_I2C_H
