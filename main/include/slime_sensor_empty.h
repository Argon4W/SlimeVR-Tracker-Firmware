#ifndef SLIME_SENSOR_EMPTY_H
#define SLIME_SENSOR_EMPTY_H

#include "slime_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief						Create a new empty sensor context.
 * @param sensor_context_out	The handle to receive the created empty sensor context.
 * @param sensor_context_name	The name of the sensor context.
 * @param sensor_context_config	The configuration of the sensor context.
 * @param i2c_context			The I2C context of the sensor context.
 * @return						The status of the creation.
 */
slime_sensor_error_t slime_empty_sensor_context_new(
			slime_sensor_context_t**	sensor_context_out,
	const	char*						sensor_context_name,
	const	void*						sensor_context_config,
	const	slime_i2c_context_t*		i2c_context
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_SENSOR_EMPTY_H
