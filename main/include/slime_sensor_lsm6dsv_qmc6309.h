#ifndef SLIME_SENSOR_LSM6DSV_QMC6309_H
#define SLIME_SENSOR_LSM6DSV_QMC6309_H

#include "lsm6dsv_reg.h"
#include "qmc6309_reg.h"
#include "slime_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The configuration struct of the LSM6DSV+QMC6309 sensor context.
 */
typedef struct {
	uint8_t					lsm6dsv_device_address;					/*!< The I2C device address of the LSM6DSV. */
	uint8_t					qmc6309_device_address;					/*!< The I2C device address of the QMC6309. */
	lsm6dsv_gy_full_scale_t	lsm6dsv_gyroscope_full_scale;			/*!< The full scale range of the gyroscope of the LSN6DSV. */
	lsm6dsv_xl_full_scale_t	lsm6dsv_accelerometer_full_scale;		/*!< The full scale range of the accelerometer of the LSM6DSV. */
	lsm6dsv_sh_data_rate_t	lsm6dsv_sensor_hub_data_rate;			/*!< The sensor hub data rate of the LSM6DSV. */
	lsm6dsv_data_rate_t		lsm6dsv_gyroscope_data_rate;			/*!< The output data rate of the gyroscope of the LSM6DSV. */
	lsm6dsv_data_rate_t		lsm6dsv_accelerometer_data_rate;		/*!< The output data rate of the accelerometer of the LSM6DSV. */
	lsm6dsv_fifo_gy_batch_t	lsm6dsv_fifo_gyroscope_batch_rate;		/*!< The FIFO batch rate of the gyroscope of the LSM6DSV. */
	lsm6dsv_fifo_xl_batch_t	lsm6dsv_fifo_accelerometer_batch_rate;	/*!< The FIFO batch rate of the accelerometer of the LSM6DSV. */
	qmc6309_setup_t			qmc6309_setup;							/*!< The setup parameters of the QMC6309. */
} slime_lsm6dsv_qmc6309_sensor_context_config_t;

/**
 * @brief						Create a new LSM6DSV+QMC6309 sensor context.
 * @param sensor_context_out	The handle to receive the created LSM6DSV+QMC6309 sensor context.
 * @param sensor_context_name	The name of the sensor context.
 * @param sensor_context_config	The configuration of the sensor context.
 * @param i2c_context			The I2C context of the sensor context.
 * @return						The status of the creation.
 */
slime_sensor_error_t slime_lsm6dsv_qmc6309_sensor_context_new(
			slime_sensor_context_t**	sensor_context_out,
	const	char*						sensor_context_name,
	const	void*						sensor_context_config,
	const	slime_i2c_context_t*		i2c_context
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_SENSOR_LSM6DSV_QMC6309_H
