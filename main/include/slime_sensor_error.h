#ifndef SLIME_SENSOR_ERROR_H
#define SLIME_SENSOR_ERROR_H

#include "esp_check.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The enum of the sensor error type.
 */
typedef enum {
	OK,			/*!< No error occurred. */
	IMU_ERROR,	/*!< The IMU related error. */
	MAG_ERROR,	/*!< The magnetometer related error. */
	HOST_ERROR	/*!< The host related error. */
} slime_sensor_error_type_t;

/**
 * @brief The info struct of a sensor error.
 */
typedef struct {
	slime_sensor_error_type_t	error_type; /*!< The type of the error. */
	esp_err_t					error_code; /*!< The code of the error. */
} slime_sensor_error_t;

/**
 * @brief		Helper macro to create a "no error".
 * @param code	The code of the error.
 */
#define SLIME_SENSOR_OK() (slime_sensor_error_t) { .error_type = OK, .error_code = 0 }

/**
 * @brief		Helper macro to create an IMU sensor error.
 * @param code	The code of the error.
 */
#define SLIME_IMU_ERROR(code) (slime_sensor_error_t) { .error_type = IMU_ERROR, .error_code = code }

/**
 * @brief		Helper macro to create an magnetometer sensor error.
 * @param code	The code of the error.
 */
#define SLIME_MAG_ERROR(code) (slime_sensor_error_t) { .error_type = MAG_ERROR, .error_code = code }

/**
 * @brief		Helper macro to create an host sensor error.
 * @param code	The code of the error.
 */
#define SLIME_HOST_ERROR(code) (slime_sensor_error_t) { .error_type = HOST_ERROR, .error_code = code }

/**
 * @brief	Helper macro to set the local slime_sensor_error_t variable err to the result of the X then extract the error_code
 *			for esp_check.h macros.
 * @param x	The provider of slime_sensor_error_t.
 */
#define SLIME_ESP_ERROR(x) ((err = (x)).error_code)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_SENSOR_ERROR_H
