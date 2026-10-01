#ifndef SLIME_FUSION_H
#define SLIME_FUSION_H

#include "math.h"
#include "esp_check.h"
#include "ceigen.h"
#include "slime_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The type declaration of the fusion context struct.
 */
typedef struct slime_fusion_context slime_fusion_context_t;

typedef struct {
	/**
	 * @brief						Function to create a new fusion context.
	 * @attention					The sensor context is ONLY for coefficient initialization, e.g. sample times.
	 * @param fusion_context_out	The handle to receive the created fusion context.
	 * @param fusion_context_name	The name of the fusion context.
	 * @param fusion_context_config	The configuration of the fusion context.
	 * @param sensor_context		The sensor context of the fusion context.
	 * @return						The status of the creation.
	 */
	esp_err_t (*fusion_context_new)(
				slime_fusion_context_t**	fusion_context_out,
		const	char*						fusion_context_name,
		const	void*						fusion_context_config,
		const	slime_sensor_context_t*		sensor_context
	);

	/**
	 * @brief		The coefficients of the quaternion to rotate an NED frame vector to a fusion frame vector.
	 * @attention	the coefficients are stored in W-X-Y-Z order. ned_to_fusion_quaternion[0] = W. The quaternion must
	 *				be a unit quaternion.
	 */
	float_t ned_to_fusion_quaternion[4];

	/**
	 * @brief The scale factor to scale the gyroscope input in mdps to gyroscope input in fusion unit.
	 */
	float_t gyroscope_ned_to_fusion_scale;

	/**
	 * @brief The scale factor to scale the accelerometer input in mg to accelerometer input in fusion unit.
	 */
	float_t accelerometer_ned_to_fusion_scale;

	/**
	 * @brief The scale factor to scale the magnetometer input in gauss to magnetometer input in fusion unit.
	 */
	float_t magnetometer_ned_to_fusion_scale;

	/**
	 * @brief The scale factor to scale the timestamp input in seconds to timestamp input in fusion unit.
	 */
	float_t timestamp_ned_to_fusion_scale;

	/**
	 * @brief The name of the type of fusion context.
	 */
	const char* name;

	/**
	 * @brief The configuration of the type of fusion context.
	 */
	const void* config;
} slime_fusion_context_type_t;

struct slime_fusion_context {
	/**
	 * @brief					Function to do the gyroscope update of the fusion.
	 * @param fusion_context	The fusion context to do the gyroscope update.
	 * @param gyroscope_fusion	The angular speed in fusion frame and unit.
	 * @return					The status of the gyroscope update.
	 */
	esp_err_t (*update_gyroscope)(
		slime_fusion_context_t*	fusion_context,
		ceigen_matrix_handle_t	gyroscope_fusion
	);

	/**
	 * @brief						Function to do the accelerometer update of the fusion.
	 * @param fusion_context		The fusion context to do the accelerometer update.
	 * @param accelerometer_fusion	The gravity in fusion frame and unit.
	 * @return						The status of the accelerometer update.
	 */
	esp_err_t (*update_accelerometer)(
		slime_fusion_context_t*	fusion_context,
		ceigen_matrix_handle_t	accelerometer_fusion
	);

	/**
	 * @brief						Function to do the magnetometer update of the fusion.
	 * @param fusion_context		The fusion context to do the magnetometer update.
	 * @param magnetometer_fusion	Th magnetic flux density in fusion frame and unit.
	 * @return						The status of the magnetometer update.
	 */
	esp_err_t (*update_magnetometer)(
		slime_fusion_context_t*	fusion_context,
		ceigen_matrix_handle_t	magnetometer_fusion
	);

	/**
	 * @brief							Function to do the timestamp update of the fusion.
	 * @param fusion_context			The fusion context to do the timestamp update.
	 * @param delta_timestamp_fusion	The timestamp increment in fusion unit.
	 * @return							The status of the timestamp update.
	 */
	esp_err_t (*update_timestamp)(
		slime_fusion_context_t*	fusion_context,
		float_t					delta_timestamp_fusion
	);

	/**
	 * @brief							Function to get the fused orientations of the fusion.
	 * @attention						Identity quaternions will be returned if the context does not support getting orientations.
	 * @param fusion_context			The fusion context to get the fused orientations.
	 * @param quaternion_fusion_6D_out	The output 6D (gyro+accel) orientation in quaternion form and fusion frame, can be NULL.
	 * @param quaternion_fusion_9D_out	The output 9D (gyro+accel+mag) orientation in quaternion form and fusion frame, can be NULL.
	 * @return							The status of getting fused orientations.
	 */
	esp_err_t (*get_orientation)(
		const	slime_fusion_context_t*		fusion_context,
				ceigen_quaternion_handle_t	quaternion_fusion_6D_out,
				ceigen_quaternion_handle_t	quaternion_fusion_9D_out
	);

	/**
	 * @brief					Function to get the states of the fusion.
	 * @attention				False will be returned if the context does not support getting states.
	 * @param fusion_context	The fusion context to get the states.
	 * @param mag_disturbed_out	The handle to receive the state of magnetic disturbance.
	 * @param rest_detected_out	The handle to receive the state of rest detection.
	 * @return					The status of getting states.
	 */
	esp_err_t (*get_state)(
		const	slime_fusion_context_t*	fusion_context,
				uint8_t*				mag_disturbed_out,
				uint8_t*				rest_detected_out
	);

	/**
	 * @brief							Function to reset the fusion.
	 * @param fusion_context			The fusion context to reset.
	 * @return							The status of the resetting.
	 */
	esp_err_t (*reset)(slime_fusion_context_t* fusion_context);

	/**
	 * @brief					Function to release the fusion context.
	 * @param fusion_context_in	The fusion context to be released.
	 * @return					The status of releasing.
	 */
	esp_err_t (*delete)(slime_fusion_context_t* fusion_context_in);

	/**
	 * @brief The vector to hold the rotated fusion frame vector.
	 */
	ceigen_matrix_handle_t ned_to_fusion_vector;

	/**
	 * @brief The quaternion to rotate an NED frame vector to a fusion frame vector.
	 */
	ceigen_quaternion_handle_t ned_to_fusion_quaternion;

	/**
	 * @brief The quaternion to rotate a fusion frame vector to an NED frame vector.
	 */
	ceigen_quaternion_handle_t fusion_to_ned_quaternion;

	/**
	 * @brief The scaled rotation matrix to rotate an NED frame gyroscope data to a fusion frame gyroscope data in fusion unit.
	 */
	ceigen_matrix_handle_t ned_to_fusion_gyroscope_matrix;

	/**
	 * @brief The scaled rotation matrix to rotate an NED frame accelerometer data to a fusion frame accelerometer data in fusion unit.
	 */
	ceigen_matrix_handle_t ned_to_fusion_accelerometer_matrix;

	/**
	 * @brief The scaled rotation matrix to rotate an NED frame magnetometer data to a fusion frame magnetometer data in fusion unit.
	 */
	ceigen_matrix_handle_t ned_to_fusion_magnetometer_matrix;

	/**
	 * @brief The scale factor to scale the timestamp input in seconds to timestamp input in fusion unit.
	 */
	float_t timestamp_ned_to_fusion_scale;

	/**
	 * @brief The name of the fusion context.
	 */
	const char* name;
};

/**
 * @brief						Do the gyroscope update of the fusion.
 * @param fusion_context		The fusion context to do the gyroscope update.
 * @param gyroscope_mdps_frd	The angular speed in millidegree(s)-per-second and FRD body frame.
 * @return						The status of the gyroscope update.
 */
esp_err_t slime_fusion_update_gyroscope(
	slime_fusion_context_t*	fusion_context,
	ceigen_matrix_handle_t	gyroscope_mdps_frd
);

/**
 * @brief						Do the accelerometer update of the fusion.
 * @param fusion_context		The fusion context to do the accelerometer update.
 * @param accelerometer_mg_frd	The gravity in milli standard gravity (about 0.00980665 m/s^2) and FRD body frame.
 * @return						The status of the accelerometer update.
 */
esp_err_t slime_fusion_update_accelerometer(
	slime_fusion_context_t*	fusion_context,
	ceigen_matrix_handle_t	accelerometer_mg_frd
);

/**
 * @brief							Do the magnetometer update of the fusion.
 * @param fusion_context			The fusion context to do the magnetometer update.
 * @param magnetometer_gauss_frd	The magnetic flux density in gauss and FRD body frame.
 * @return							The status of the magnetometer update.
 */
esp_err_t slime_fusion_update_magnetometer(
	slime_fusion_context_t*	fusion_context,
	ceigen_matrix_handle_t	magnetometer_gauss_frd
);

/**
 * @brief							Do the timestamp update of the fusion.
 * @param fusion_context			The fusion context to do the timestamp update.
 * @param delta_timestamp_seconds	The timestamp increment in seconds.
 * @return							The status of the timestamp update.
 */
esp_err_t slime_fusion_update_timestamp(
	slime_fusion_context_t*	fusion_context,
	float_t					delta_timestamp_seconds
);

/**
 * @brief						Get the fused orientations of the fusion.
 * @attention					Identity quaternions will be returned if the context does not support getting orientations.
 * @param fusion_context		The fusion context to get the fused orientations.
 * @param quaternion_ned_6D_out	The output 6D (gyro+accel) orientation in quaternion form in NED earth frame, can be NULL.
 * @param quaternion_ned_9D_out	The output 9D (gyro+accel+mag) orientation in quaternion form in NED earth frame, can be NULL.
 * @return						The status of getting fused orientations.
 */
esp_err_t slime_fusion_get_orientation(
	const	slime_fusion_context_t*		fusion_context,
			ceigen_quaternion_handle_t	quaternion_ned_6D_out,
			ceigen_quaternion_handle_t	quaternion_ned_9D_out
);

/**
 * @brief					Get the states of the fusion.
 * @attention				False will be returned if the context does not support getting states.
 * @param fusion_context	The fusion context to get the states.
 * @param mag_disturbed_out	The handle to receive the state of magnetic disturbance.
 * @param rest_detected_out	The handle to receive the state of rest detection.
 * @return					The status of getting states.
 */
esp_err_t slime_fusion_get_state(
	const	slime_fusion_context_t*	fusion_context,
			uint8_t*				mag_disturbed_out,
			uint8_t*				rest_detected_out
);

/**
 * @brief					Reset the fusion.
 * @param fusion_context	The fusion context to reset.
 * @return					The status of the resetting.
 */
esp_err_t slime_fusion_reset(slime_fusion_context_t* fusion_context);

/**
 * @brief							Create a new fusion context based on the fusion context type ID.
 * @attention						The sensor context is ONLY for coefficient initialization, e.g. sample times.
 * @param fusion_context_out		The handle to receive the created fusion context.
 * @param fusion_context_type_table	The table of fusion context types by fusion context type ID.
 * @param sensor_context			The sensor context of the fusion context.
 * @param fusion_context_type_id	The fusion context type id of the fusion context to create.
 * @return							The status of the creation.
 */
esp_err_t slime_fusion_context_new(
			slime_fusion_context_t**		fusion_context_out,
	const	slime_fusion_context_type_t*	fusion_context_type_table,
	const	slime_sensor_context_t*			sensor_context,
			uint32_t						fusion_context_type_id
);

/**
 * @brief					Release the fusion context.
 * @param fusion_context_in The fusion context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_fusion_context_del(slime_fusion_context_t* fusion_context_in);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_FUSION_H
