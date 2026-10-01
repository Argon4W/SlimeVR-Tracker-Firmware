#ifndef SLIME_SENSOR_H
#define SLIME_SENSOR_H

#include "math.h"
#include "esp_check.h"
#include "slime_gpio.h"
#include "slime_i2c.h"
#include "slime_sensor_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The type declaration of the sensor context struct.
 */
typedef struct slime_sensor_context slime_sensor_context_t;

/**
 * Frame convention in X-Y-Z:
 *
 * Body frame: FRD
 *  - X is Forward (Logo side).
 *  - Y is Right (Opposite side of the Type-C charging port side).
 *  - Z is down (Opposite side of the switch side).
 *
 * Nav frame: NED
 *  - X is facing towards North.
 *  - Y is facing towards East.
 *  - Z is facing towards the ground.
 *
 * Angular rate:
 *  When the positive direction of an axis faces towards you, the observed positive rotation of the axis is CCW (counter-clockwise).
 */


/**
 * @brief						Handle type of the function that will be invoked when the sensor receives new gyroscope data.
 * @param sensor_context		The sensor context that receives the gyroscope data.
 * @param gyroscope_mdps_frd_x	The angular speed of the X axis in millidegree(s)-per-second in FRD body frame of the received gyroscope data.
 * @param gyroscope_mdps_frd_y	The angular speed of the Y axis in millidegree(s)-per-second in FRD body frame of the received gyroscope data.
 * @param gyroscope_mdps_frd_z	The angular speed of the Z axis in millidegree(s)-per-second in FRD body frame of the received gyroscope data.
 * @param user_context			Custom used-defined handle.
 */
typedef void (*slime_sensor_gyroscope_callback_t)(
	const	slime_sensor_context_t*	sensor_context,
			float_t					gyroscope_mdps_frd_x,
			float_t					gyroscope_mdps_frd_y,
			float_t					gyroscope_mdps_frd_z,
			void*					user_context
);

/**
 * @brief							Handle type of the function that will be invoked when the sensor receives new accelerometer data.
 * @param sensor_context			The sensor context that receives the accelerometer data.
 * @param accelerometer_mg_frd_x	The gravity of the X axis in milli standard gravity (about 0.00980665 m/s^2) in FRD body frame of the received accelerometer data.
 * @param accelerometer_mg_frd_y	The gravity of the Y axis in milli standard gravity (about 0.00980665 m/s^2) in FRD body frame of the received accelerometer data.
 * @param accelerometer_mg_frd_z	The gravity of the Z axis in milli standard gravity (about 0.00980665 m/s^2) in FRD body frame of the received accelerometer data.
 * @param user_context				Custom used-defined handle.
 */
typedef void (*slime_sensor_accelerometer_callback_t)(
	const	slime_sensor_context_t*	sensor_context,
			float_t					accelerometer_mg_frd_x,
			float_t					accelerometer_mg_frd_y,
			float_t					accelerometer_mg_frd_z,
			void*					user_context
);

/**
 * @brief							Handle type of the function that will be invoked when the sensor receives new magnetometer data.
 * @param sensor_context			The sensor context that receives the magnetometer data.
 * @param magnetometer_gauss_frd_x	The magnetic flux density of the X axis in gauss in FRD body frame of the received magnetometer data.
 * @param magnetometer_gauss_frd_y	The magnetic flux density of the Y axis in gauss in FRD body frame of the received magnetometer data.
 * @param magnetometer_gauss_frd_z	The magnetic flux density of the Z axis in gauss in FRD body frame of the received magnetometer data.
 * @param user_context				Custom used-defined handle.
 */
typedef void (*slime_sensor_magnetometer_callback_t)(
	const	slime_sensor_context_t*	sensor_context,
			float_t					magnetometer_gauss_frd_x,
			float_t					magnetometer_gauss_frd_y,
			float_t					magnetometer_gauss_frd_z,
			void*					user_context
);

/**
 * @brief							Handle type of the function that will be invoked when the sensor receives new timestamp data.
 * @param sensor_context			The sensor context that receives the timestamp data.
 * @param delta_timestamp_seconds	The timestamp increment in seconds of the received timestamp data.
 * @param user_context				Custom used-defined handle.
 */
typedef void (*slime_sensor_timestamp_callback_t) (
	const	slime_sensor_context_t*	sensor_context,
			float_t					delta_timestamp_seconds,
			void*					user_context
);

/**
 * @brief The callbacks configuration struct of the sensor context.
 */
typedef struct {
	/**
	 * @brief The handle of the timestamp callback function.
	 */
	slime_sensor_timestamp_callback_t timestamp_callback;

	/**
	 * @brief The handle of the gyroscope callback function.
	 */
	slime_sensor_gyroscope_callback_t gyroscope_callback;

	/**
	 * @brief The handle of the accelerometer callback function.
	 */
	slime_sensor_accelerometer_callback_t accelerometer_callback;

	/**
	 * @brief The handle of the magnetometer callback function.
	 */
	slime_sensor_magnetometer_callback_t magnetometer_callback;
} slime_sensor_callbacks_config_t;

/**
 * @brief The info struct of a sensor context type.
 */
typedef struct {
	/**
	 * @brief						Function to create a new sensor context.
	 * @param sensor_context_out	The handle to receive the created sensor context.
	 * @param sensor_context_config	The configuration of the sensor context.
	 * @param sensor_context_name	The name of the sensor context.
	 * @param i2c_context			The I2C context of the sensor context.
	 * @return						The status of the creation.
	 */
	slime_sensor_error_t (*sensor_context_new)(
				slime_sensor_context_t**	sensor_context_out,
		const	char*						sensor_context_name,
		const	void*						sensor_context_config,
		const	slime_i2c_context_t*		i2c_context
	);

	/**
	 * @brief The name of the type of sensor context.
	 */
	const char* name;

	/**
	 * @brief The configuration of the type of sensor context.
	 */
	const void* config;
} slime_sensor_context_type_t;

/**
 * @brief The definition of the sensor context struct.
 */
struct slime_sensor_context {
	/**
	 * @brief					Function to register callbacks.
	 * @param sensor_context	The sensor context to register the callbacks into.
	 * @param sensor_callbacks	The handle to the configuration of callbacks.
	 * @param user_context		The custom user-defined handle, passed directly to callback's user_context.
	 * @return					The status of registering callbacks.
	 */
	esp_err_t (*register_callbacks)(
				slime_sensor_context_t*				sensor_context,
		const	slime_sensor_callbacks_config_t*	sensor_callbacks,
				void*								user_context
	);

	/**
	 * @brief								Function to getting sample times of the sensors.
	 * @param sensor_context				The sensor context to get the sample times.
	 * @param gyroscope_sample_time_ms		The handle to receive the gyroscope sample time in milliseconds.
	 * @param accelerometer_sample_time_ms	The handle to receive the accelerometer sample time in milliseconds.
	 * @param magnetometer_sample_time_ms	The handle to receive the magnetometer sample time in milliseconds.
	 * @return								The status of getting sample times.
	 */
	esp_err_t (*get_sample_time)(
		const	slime_sensor_context_t*	sensor_context,
				float_t*				gyroscope_sample_time_ms,
				float_t*				accelerometer_sample_time_ms,
				float_t*				magnetometer_sample_time_ms
	);

	/**
	 * @brief					Function to poll sensor FIFO data.
	 * @param sensor_context	The sensor context to poll the FIFO data.
	 * @return					The status of polling FIFO data.
	 */
	slime_sensor_error_t (*poll_fifo)(slime_sensor_context_t* sensor_context);

	/**
	 * @brief					Function to release the sensor context.
	 * @param sensor_context_in	The sensor context to be released.
	 * @return					The status of releasing.
	 */
	esp_err_t (*delete)(slime_sensor_context_t* sensor_context_in);

	/**
	 * @brief The name of the sensor context.
	 */
	const char* name;
};

/**
 * @brief					Register callbacks.
 * @param sensor_context	The sensor context to register the callback into.
 * @param sensor_callbacks	The handle to the configuration of callbacks.
 * @param user_context		The custom user-defined handle, passed directly to callback's user_context.
 * @return					The status of registering callback.
 */
esp_err_t slime_sensor_register_callbacks(
			slime_sensor_context_t*				sensor_context,
	const	slime_sensor_callbacks_config_t*	sensor_callbacks,
			void*								user_context
);

/**
 * @brief								Get the sample times of the sensors.
 * @param sensor_context				The sensor context to get the sample times.
 * @param gyroscope_sample_time_ms		The handle to receive the gyroscope sample time in milliseconds.
 * @param accelerometer_sample_time_ms	The handle to receive the accelerometer sample time in milliseconds.
 * @param magnetometer_sample_time_ms	The handle to receive the magnetometer sample time in milliseconds.
 * @return								The status of getting sample times.
 */
esp_err_t slime_sensor_get_sample_time(
	const	slime_sensor_context_t*	sensor_context,
			float_t*				gyroscope_sample_time_ms,
			float_t*				accelerometer_sample_time_ms,
			float_t*				magnetometer_sample_time_ms
);

/**
 * @brief					Poll sensor FIFO data.
 * @param sensor_context	The sensor context to poll the FIFO data.
 * @return					The status of polling FIFO data.
 */
slime_sensor_error_t slime_sensor_poll_fifo(slime_sensor_context_t* sensor_context);

/**
 * @brief							Create a new sensor context based on the sensor board ID read from the GPIO context.
 * @param sensor_context_out		The handle to receive the created sensor context.
 * @param sensor_context_type_table	The table of sensor context types by sensor board ID.
 * @param gpio_context				The GPIO context to read the sensor board ID.
 * @param i2c_context				The I2C context of the sensor context.
 * @return							The status of the creation.
 */
slime_sensor_error_t slime_sensor_context_new(
			slime_sensor_context_t**		sensor_context_out,
	const	slime_sensor_context_type_t*	sensor_context_type_table,
	const	slime_gpio_context_t*			gpio_context,
	const	slime_i2c_context_t*			i2c_context
);

/**
 * @brief					Release the sensor context.
 * @param sensor_context_in The sensor context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_sensor_context_del(slime_sensor_context_t* sensor_context_in);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_SENSOR_H
