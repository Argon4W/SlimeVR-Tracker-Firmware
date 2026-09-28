#ifndef VQF_H
#define VQF_H

#include "math.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The definition of the linear algebra backend handle type of matrices.
 */
#ifndef VQF_MATRIX_HANDLE_TYPE
/**
 * @brief	If the type is not set, set the default linear algebra handle type of matrices to opaque handle type void*.
 *			Let backend decide the type of the handle at runtime.
 */
#define VQF_MATRIX_HANDLE_TYPE void*
#endif // VQF_MATRIX_HANDLE_TYPE

/**
 * The definition of the linear algebra backend handle type of double precision matrices.
 */
#ifndef VQF_MATRIX_DOUBLE_HANDLE_TYPE
/**
 * @brief	If the type is not set, set the default linear algebra handle type of double precision matrices to opaque
 *			handle type void*. Let backend decide the type of the handle at runtime.
 */
#define VQF_MATRIX_DOUBLE_HANDLE_TYPE void*
#endif // VQF_MATRIX_DOUBLE_HANDLE_TYPE

/**
 * The definition of the linear algebra backend handle type of quaternions.
 */
#ifndef VQF_QUATERNION_HANDLE_TYPE
/**
 * @brief	If the type is not set, set the default linear algebra handle type of quaternions to opaque handle type
 *			void*. Let backend decide the type of the handle at runtime.
 */
#define VQF_QUATERNION_HANDLE_TYPE void*
#endif // VQF_QUATERNION_HANDLE_TYPE

/**
 * @brief		The matrix handle type of the VQF linear algebra backend functions.
 * @attention	The matrix should store the coefficients in float_t precision.
 */
typedef VQF_MATRIX_HANDLE_TYPE vqf_matrix_handle_t;

/**
 * @brief		The double precision matrix handle type of the VQF linear algebra backend functions.
 * @attention	The double precision matrix should store the coefficients in double_t precision.
 */
typedef VQF_MATRIX_DOUBLE_HANDLE_TYPE vqf_matrix_double_handle_t;

/**
 * @brief		The quaternion handle type of the VQF linear algebra backend functions.
 * @attention	The quaternion should store the coefficients in float_t precision.
 */
typedef VQF_QUATERNION_HANDLE_TYPE vqf_quaternion_handle_t;

/**
 * @brief Struct containing pointers to all linear algebra functions used by the VQF class.
 *
 * This allows VQF to be integrated into any linear algebra library backend that supports the functions listed in the
 * vqf_linear_algebra_t. It helps the devs to integrate the VQF into projects more easily or implementing high performance
 * backend when SIMD is available.
 */
typedef struct {
	/**
	 * @brief			Create a new matrix. All values of the matrix should be 0.0f when initialized.
	 * @param rows		rows of the allocated matrix.
	 * @param columns	columns of the allocated matrix.
	 * @retval			the created matrix.
	 */
	vqf_matrix_handle_t (*new_matrix)(
		uint32_t rows,
		uint32_t columns
	);
	/**
	 * @brief			Delete an existing matrix.
	 * @param matrix	The matrix to be deleted.
	 */
	void (*delete_matrix)(
		vqf_matrix_handle_t matrix
	);
	/**
	 * @brief			Get the value of a coefficient in an existing matrix.
	 * @param matrix	the matrix of the coefficient.
	 * @param row		the row of the coefficient.
	 * @param column	the column of the coefficient.
	 * @retval			the value of the coefficient get from the matrix.
	 */
	float_t (*get_matrix_coefficient)(
		vqf_matrix_handle_t	matrix,
		uint32_t			row,
		uint32_t			column
	);
	/**
	 * @brief			Set the value of a coefficient in an existing matrix.
	 * @param matrix	the matrix of the coefficient.
	 * @param row		the row of the coefficient.
	 * @param column	the column of the coefficient.
	 * @param value		the value of the coefficient.
	 */
	void (*set_matrix_coefficient)(
		vqf_matrix_handle_t	matrix,
		uint32_t			row,
		uint32_t			column,
		float_t				value
	);
	/**
	 * @brief			Add a value to the existing value of a coefficient in an existing matrix.
	 * @param matrix	the matrix of the coefficient.
	 * @param row		the row of the coefficient.
	 * @param column	the column of the coefficient.
	 * @param value		the value to be added to the coefficient.
	 */
	void (*add_matrix_coefficient)(
		vqf_matrix_handle_t	matrix,
		uint32_t			row,
		uint32_t			column,
		float_t				value
	);
	/**
	 * @brief						Copy coefficients of an existing matrix to another matrix.
	 * @param source_matrix			the source matrix to be copied.
	 * @param destination_matrix	destination matrix coefficients are copied to.
	 */
	void (*copy_matrix)(
		vqf_matrix_handle_t source_matrix,
		vqf_matrix_handle_t destination_matrix
	);
	/**
	 * @brief						Multiply two existing matrices. (destination_matrix = left_matrix * right_matrix)
	 * @param left_matrix			the left matrix of the multiplication.
	 * @param right_matrix			the right matrix of the multiplication.
	 * @param destination_matrix	destination matrix to hold the result matrix.
	 */
	void (*multiply_matrix)(
		vqf_matrix_handle_t left_matrix,
		vqf_matrix_handle_t right_matrix,
		vqf_matrix_handle_t destination_matrix
	);
	/**
	 * @brief						Add two existing matrices. (destination_matrix = left_matrix + right_matrix)
	 * @param left_matrix			the left matrix of the addition.
	 * @param right_matrix			the right matrix of the addition.
	 * @param destination_matrix	destination matrix to hold the result matrix.
	 */
	void (*add_matrix)(
		vqf_matrix_handle_t left_matrix,
		vqf_matrix_handle_t right_matrix,
		vqf_matrix_handle_t destination_matrix
	);
	/**
	 * @brief						Subtract two existing matrices. (destination_matrix = left_matrix - right_matrix)
	 * @param left_matrix			the left matrix of the subtraction.
	 * @param right_matrix			the right matrix of the subtraction.
	 * @param destination_matrix	destination matrix to hold the result matrix.
	 */
	void (*subtract_matrix)(
		vqf_matrix_handle_t left_matrix,
		vqf_matrix_handle_t right_matrix,
		vqf_matrix_handle_t destination_matrix
	);
	/**
	 * @brief				Invert an existing matrix in place.
	 * @param source_matrix	the matrix to be inverted.
	 */
	void (*invert_matrix_in_place)(
		vqf_matrix_handle_t source_matrix
	);
	/**
	 * @brief				Transpose an existing matrix in place.
	 * @param source_matrix	the matrix to be transposed.
	 */
	void (*transpose_matrix_in_place)(
		vqf_matrix_handle_t source_matrix
	);
	/**
	 * @brief				Normalize all column vectors of an existing matrix in place.
	 * @param source_matrix	the matrix to be normalized.
	 */
	void (*normalize_matrix_in_place)(
		vqf_matrix_handle_t source_matrix
	);
	/**
	 * @brief				Multiply an existing matrix with a scalar in place.
	 * @param source_matrix	the matrix to be multiplied with scalar.
	 * @param value			the scalar to be multiplied to the matrix.
	 */
	void (*multiply_matrix_scalar_in_place)(
		vqf_matrix_handle_t	source_matrix,
		float_t				value
	);
	/**
	 * @brief				Set values of all coefficients of an existing matrix to 0 in place.
	 * @param source_matrix	the matrix to be set to zeros.
	 */
	void (*set_matrix_zeros_in_place)(
		vqf_matrix_handle_t source_matrix
	);
	/**
	 * @brief				Set an existing matrix to scaled identity matrix in place.
	 * @param source_matrix	the matrix to be set to scaled identity matrix.
	 * @param scale			the scale of the matrix.
	 */
	void (*set_matrix_scaled_identity_in_place)(
		vqf_matrix_handle_t	source_matrix,
		float_t				scale
	);
	/**
	 * @brief				Treat an existing matrix as an n-dimensional vector then calculate the norm.
	 * @param source_vector	The vector to calculate the norm.
	 * @retval				the norm.
	 */
	float_t (*get_vector_norm)(
		vqf_matrix_handle_t source_vector
	);
	/**
	 * @brief				Treat an existing matrix as an n-dimensional vector then calculate the squared norm.
	 * @param source_vector	The vector to calculate the squared norm.
	 * @retval				the squared norm.
	 */
	float_t (*get_vector_squared_norm)(
		vqf_matrix_handle_t source_vector
	);
	/**
	 * @brief				Treat an existing matrix as an n-dimensional vector then clip the coefficients of the vector in place.
	 * @param source_vector	The vector to be clipped.
	 * @param min_value		The minimal allowed value of the coefficients.
	 * @param max_value		The maximum allowed value of the coefficients.
	 */
	void (*clip_vector_in_place)(
		vqf_matrix_handle_t	source_vector,
		float_t				min_value,
		float_t				max_value
	);

	/**
	 * @brief			Create a new double precision matrix. All values of the matrix should be 0.0f when initialized.
	 * @param rows		rows of the allocated matrix.
	 * @param columns	columns of the allocated matrix.
	 * @retval			the created matrix.
	 */
	vqf_matrix_double_handle_t (*new_matrix_double)(
		uint32_t rows,
		uint32_t columns
	);
	/**
	 * @brief			Delete an existing double precision matrix.
	 * @param matrix	The double precision matrix to be deleted.
	 */
	void (*delete_matrix_double)(
		vqf_matrix_double_handle_t matrix
	);
	/**
	 * @brief			Get the value of a coefficient in an existing double precision matrix.
	 * @param matrix	the double precision matrix of the coefficient.
	 * @param row		the row of the coefficient.
	 * @param column	the column of the coefficient.
	 * @retval			the value of the coefficient get from the double precision matrix.
	 */
	double_t (*get_matrix_double_coefficient)(
		vqf_matrix_double_handle_t	matrix,
		uint32_t					row,
		uint32_t					column
	);
	/**
	 * @brief			Set the value of a coefficient in an existing double precision matrix.
	 * @param matrix	the double precision matrix of the coefficient.
	 * @param row		the row of the coefficient.
	 * @param column	the column of the coefficient.
	 * @param value		the value of the coefficient.
	 */
	void (*set_matrix_double_coefficient)(
		vqf_matrix_double_handle_t	matrix,
		uint32_t					row,
		uint32_t					column,
		double_t					value
	);
	/**
	 * @brief			Add a value to the existing value of a coefficient in an existing double precision matrix.
	 * @param matrix	the double precision matrix of the coefficient.
	 * @param row		the row of the coefficient.
	 * @param column	the column of the coefficient.
	 * @param value		the value to be added to the coefficient.
	 */
	void (*add_matrix_double_coefficient)(
		vqf_matrix_double_handle_t	matrix,
		uint32_t					row,
		uint32_t					column,
		double_t					value
	);
	/**
	 * @brief						Copy coefficients of an existing double precision matrix to another double precision matrix.
	 * @param source_matrix			the source double precision matrix to be copied.
	 * @param destination_matrix	destination double precision matrix coefficients are copied to.
	 */
	void (*copy_matrix_double)(
		vqf_matrix_double_handle_t source_matrix,
		vqf_matrix_double_handle_t destination_matrix
	);
	/**
	 * @brief						Add two existing double precision matrices. (destination_matrix = left_matrix + right_matrix)
	 * @param left_matrix			the left double precision matrix of the addition.
	 * @param right_matrix			the right double precision matrix of the addition.
	 * @param destination_matrix	destination double precision matrix to hold the result matrix.
	 */
	void (*add_matrix_double)(
		vqf_matrix_double_handle_t left_matrix,
		vqf_matrix_double_handle_t right_matrix,
		vqf_matrix_double_handle_t destination_matrix
	);
	/**
	 * @brief						Accumulate an existing double precision matrix into another existing double precision matrix. (destination_matrix += weight * source_matrix)
	 * @param source_matrix			the source double precision matrix to be accumulated.
	 * @param source_weight			the weight of the source precision matrix to be accumulated.
	 * @param destination_matrix	destination double precision matrix to accumulate the weighted matrix.
	 */
	void (*accumulate_matrix_double)(
		double_t					source_weight,
		vqf_matrix_double_handle_t	source_matrix,
		vqf_matrix_double_handle_t	destination_matrix
	);
	/**
	 * @brief				Multiply an existing double precision matrix with a scalar in place.
	 * @param source_matrix	the double precision matrix to be multiplied with scalar.
	 * @param value			the double precision scalar to be multiplied to the matrix.
	 */
	void (*multiply_matrix_double_scalar_in_place)(
		vqf_matrix_double_handle_t	source_matrix,
		double_t					value
	);
	/**
	 * @brief				Set values of all coefficients of an existing double precision matrix to 0 in place.
	 * @param source_matrix	the double precision matrix to be set to zeros.
	 */
	void (*set_matrix_double_zeros_in_place)(
		vqf_matrix_double_handle_t source_matrix
	);
	/**
	 * @brief				Set all coefficients of an existing double precision matrix with given constant scalar in place.
	 * @param source_matrix	the double precision matrix to be set.
	 * @param value			the double precision scalar that all coefficients of the double precision matrix will be set to.
	 */
	void (*set_matrix_double_constants_in_place)(
		vqf_matrix_double_handle_t	source_matrix,
		double_t					value
	);
	/**
	 * @brief						Copy coefficients of an existing matrix to another existing double precision matrix.
	 * @param source_matrix			the source matrix to be copied.
	 * @param destination_matrix	destination double precision matrix coefficients are copied to.
	 */
	void (*copy_matrix_to_matrix_double)(
		vqf_matrix_handle_t			source_matrix,
		vqf_matrix_double_handle_t	destination_matrix
	);
	/**
	 * @brief						Copy coefficients of an existing double precision matrix to another existing matrix.
	 * @param source_matrix			the source double precision matrix to be copied.
	 * @param destination_matrix	destination matrix coefficients are copied to.
	 */
	void (*copy_matrix_double_to_matrix)(
		vqf_matrix_double_handle_t	source_matrix,
		vqf_matrix_handle_t			destination_matrix
	);

	/**
	 * @brief	Create a new quaternion. Quaternion should be identity quaternion when initialized.
	 * @retval	the created quaternion.
	 */
	vqf_quaternion_handle_t (*new_quaternion)();
	/**
	 * @brief				Delete an existing quaternion.
	 * @param quaternion	The quaternion to be deleted.
	 */
	void (*delete_quaternion)(
		vqf_quaternion_handle_t quaternion
	);
	/**
	 * @brief				Set the value of the W in an existing quaternion.
	 * @param quaternion	the quaternion to set the W.
	 * @param value			the value of the new W.
	 */
	void (*set_quaternion_w)(
		vqf_quaternion_handle_t	quaternion,
		float_t					value
	);
	/**
	 * @brief				Set the value of the X in an existing quaternion.
	 * @param quaternion	the quaternion to set the X.
	 * @param value			the value of the new X.
	 */
	void (*set_quaternion_x)(
		vqf_quaternion_handle_t	quaternion,
		float_t					value
	);
	/**
	 * @brief				Set the value of the Y in an existing quaternion.
	 * @param quaternion	the quaternion to set the Y.
	 * @param value			the value of the new Y.
	 */
	void (*set_quaternion_y)(
		vqf_quaternion_handle_t	quaternion,
		float_t					value
	);
	/**
	 * @brief				Set the value of the Z in an existing quaternion.
	 * @param quaternion	the quaternion to set the Z.
	 * @param value			the value of the new Z.
	 */
	void (*set_quaternion_z)(
		vqf_quaternion_handle_t	quaternion,
		float_t					value
	);
	/**
	 * @brief							Copy coefficients of an existing quaternion to another quaternion.
	 * @param source_quaternion			the source quaternion to be copied.
	 * @param destination_quaternion	destination quaternion coefficients are copied to.
	 */
	void (*copy_quaternion)(
		vqf_quaternion_handle_t source_quaternion,
		vqf_quaternion_handle_t destination_quaternion
	);
	/**
	 * @brief							Multiply two existing quaternions. (destination_quaternion = left_quaternion * right_quaternion)
	 * @param left_quaternion			the left quaternion of the multiplication.
	 * @param right_quaternion			the right quaternion of the multiplication.
	 * @param destination_quaternion	destination quaternion to hold the result quaternion.
	 */
	void (*multiply_quaternion)(
		vqf_quaternion_handle_t left_quaternion,
		vqf_quaternion_handle_t right_quaternion,
		vqf_quaternion_handle_t destination_quaternion
	);
	/**
	 * @brief							Rotate an existing quaternion by given radians around Z axis. (destination_quaternion = src_quaternion * [cos(rotation_z_radians/2), 0, 0, sin(rotation_z_radians/2)])
	 * @param src_quaternion			the source quaternion to be rotated.
	 * @param rotation_z_radians		the rotation around Z axis in radians.
	 * @param destination_quaternion	destination quaternion to hold the result quaternion.
	 */
	void (*rotate_quaternion_around_z)(
		float_t					rotation_z_radians,
		vqf_quaternion_handle_t	src_quaternion,
		vqf_quaternion_handle_t	destination_quaternion
	);
	/**
	 * @brief							Set the coefficients of an existing quaternion to a given rotation.
	 * @param rotation_angle_radians	The angle of the rotation in radians.
	 * @param rotation_axis				the axis of the rotation, treat the matrix as a 3-dimensional vector.
	 * @param destination_quaternion	destination quaternion to hold the result rotation quaternion.
	 */
	void (*set_quaternion_rotation)(
		float_t					rotation_angle_radians,
		vqf_matrix_handle_t		rotation_axis,
		vqf_quaternion_handle_t	destination_quaternion
	);
	/**
	 * @brief						Treat an existing matrix as a 3-dimension vector then rotate the vector with a given quaternion. (destination_vector = quaternion * src_vector * (quaternion^{-1}))
	 * @param src_vector			the source vector to be rotated.
	 * @param quaternion			the quaternion to rotate the vector.
	 * @param destination_vector	destination vector to hold the result vector.
	 */
	void (*quaternion_rotate_vector)(
		vqf_matrix_handle_t		src_vector,
		vqf_quaternion_handle_t	quaternion,
		vqf_matrix_handle_t		destination_vector
	);
	/**
	 * @brief						Convert an existing quaternion to rotation matrix.
	 * @param src_quaternion		the source quaternion to be converted into rotation matrix.
	 * @param destination_matrix	destination matrix to hold the result rotation matrix.
	 */
	void (*quaternion_to_rotation_matrix)(
		vqf_quaternion_handle_t	src_quaternion,
		vqf_matrix_handle_t		destination_matrix
	);
	/**
	 * @brief					Normalize an existing quaternion in place.
	 * @param source_quaternion	the quaternion to be normalized.
	 */
	void (*normalize_quaternion_in_place)(
		vqf_quaternion_handle_t source_quaternion
	);
	/**
	 * @brief					Set an existing quaternion to identity quaternion in place.
	 * @param source_quaternion	the quaternion to be set to identity quaternion.
	 */
	void (*set_quaternion_identity_in_place)(
		vqf_quaternion_handle_t  source_quaternion
	);
} vqf_linear_algebra_t;

/**
 * @brief Struct containing all tuning parameters used by the VQF class.
 *
 * The parameters influence the behavior of the algorithm and are independent of the sampling rate of the IMU data. The
 * constructor sets all parameters to the default values.
 *
 * The parameters #motionBiasEstEnabled, #restBiasEstEnabled, and #magDistRejectionEnabled can be used to enable/disable
 * the main features of the VQF algorithm. The time constants #tauAcc and #tauMag can be tuned to change the trust on
 * the accelerometer and magnetometer measurements, respectively. The remaining parameters influence bias estimation
 * and magnetometer rejection.
 */
typedef struct {
	/**
	 * @brief Time constant \f$\tau_\mathrm{acc}\f$ for accelerometer low-pass filtering in seconds.
	 *
	 * Small values for \f$\tau_\mathrm{acc}\f$ imply trust on the accelerometer measurements and while large values of
	 * \f$\tau_\mathrm{acc}\f$ imply trust on the gyroscope measurements.
	 *
	 * The time constant \f$\tau_\mathrm{acc}\f$ corresponds to the cutoff frequency \f$f_\mathrm{c}\f$ of the
	 * second-order Butterworth low-pass filter as follows: \f$f_\mathrm{c} = \frac{\sqrt{2}}{2\pi\tau_\mathrm{acc}}\f$.
	 *
	 * Default value: 3.0 s
	 */
	float_t tau_acc;
	/**
	 * @brief Time constant \f$\tau_\mathrm{mag}\f$ for magnetometer update in seconds.
	 *
	 * Small values for \f$\tau_\mathrm{mag}\f$ imply trust on the magnetometer measurements and while large values of
	 * \f$\tau_\mathrm{mag}\f$ imply trust on the gyroscope measurements.
	 *
	 * The time constant \f$\tau_\mathrm{mag}\f$ corresponds to the cutoff frequency \f$f_\mathrm{c}\f$ of the
	 * first-order low-pass filter for the heading correction as follows:
	 * \f$f_\mathrm{c} = \frac{1}{2\pi\tau_\mathrm{mag}}\f$.
	 *
	 * Default value: 9.0 s
	 */
	float_t tau_mag;

#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	/**
	 * @brief Enables gyroscope bias estimation during motion phases.
	 *
	 * If set to true (default), gyroscope bias is estimated based on the inclination correction only, i.e. without
	 * using magnetometer measurements.
	 */
	uint8_t motion_bias_est_enabled;
#endif
	/**
	 * @brief Enables rest detection and gyroscope bias estimation during rest phases.
	 *
	 * If set to true (default), phases in which the IMU is at rest are detected. During rest, the gyroscope bias
	 * is estimated from the low-pass filtered gyroscope readings.
	 */
	uint8_t rest_bias_est_enabled;
	/**
	 * @brief Enables magnetic disturbance detection and magnetic disturbance rejection.
	 *
	 * If set to true (default), the magnetic field is analyzed. For short disturbed phases, the magnetometer-based
	 * correction is disabled totally. If the magnetic field is always regarded as disturbed or if the duration of
	 * the disturbances exceeds #magMaxRejectionTime, magnetometer-based updates are performed, but with an increased
	 * time constant.
	 */
	uint8_t mag_dist_rejection_enabled;

	/**
	 * @brief Standard deviation of the initial bias estimation uncertainty (in degrees per second).
	 *
	 * Default value: 0.5 °/s
	 */
	float_t bias_sigma_init;
	/**
	 * @brief Time in which the bias estimation uncertainty increases from 0 °/s to 0.1 °/s (in seconds).
	 *
	 * This value determines the system noise assumed by the Kalman filter.
	 *
	 * Default value: 100.0 s
	 */
	float_t bias_forgetting_time;
	/**
	 * @brief Maximum expected gyroscope bias (in degrees per second).
	 *
	 * This value is used to clip the bias estimate and the measurement error in the bias estimation update step. It is
	 * further used by the rest detection algorithm in order to not regard measurements with a large but constant
	 * angular rate as rest.
	 *
	 * Default value: 2.0 °/s
	 */
	float_t bias_clip;
#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	/**
	 * @brief Standard deviation of the converged bias estimation uncertainty during motion (in degrees per second).
	 *
	 * This value determines the trust on motion bias estimation updates. A small value leads to fast convergence.
	 *
	 * Default value: 0.1 °/s
	 */
	float_t bias_sigma_motion;
	/**
	 * @brief Forgetting factor for unobservable bias in vertical direction during motion.
	 *
	 * As magnetometer measurements are deliberately not used during motion bias estimation, gyroscope bias is not
	 * observable in vertical direction. This value is the relative weight of an artificial zero measurement that
	 * ensures that the bias estimate in the unobservable direction will eventually decay to zero.
	 *
	 * Default value: 0.0001
	 */
	float_t bias_vertical_forgetting_factor;
#endif
	/**
	 * @brief Standard deviation of the converged bias estimation uncertainty during rest (in degrees per second).
	 *
	 * This value determines the trust on rest bias estimation updates. A small value leads to fast convergence.
	 *
	 * Default value: 0.03 °
	 */
	float_t bias_sigma_rest;

	/**
	 * @brief Time threshold for rest detection (in seconds).
	 *
	 * Rest is detected when the measurements have been close to the low-pass filtered reference for the given time.
	 *
	 * Default value: 1.5 s
	 */
	float_t rest_min_t;
	/**
	 * @brief Time constant for the low-pass filter used in rest detection (in seconds).
	 *
	 * This time constant characterizes a second-order Butterworth low-pass filter used to obtain the reference for
	 * rest detection.
	 *
	 * Default value: 0.5 s
	 */
	float_t rest_filter_tau;
	/**
	 * @brief Angular velocity threshold for rest detection (in °/s).
	 *
	 * For rest to be detected, the norm of the deviation between measurement and reference must be below the given
	 * threshold. (Furthermore, the absolute value of each component must be below #biasClip).
	 *
	 * Default value: 2.0 °/s
	 */
	float_t rest_th_gyr;
	/**
	 * @brief Acceleration threshold for rest detection (in m/s²).
	 *
	 * For rest to be detected, the norm of the deviation between measurement and reference must be below the given
	 * threshold.
	 *
	 * Default value: 0.5 m/s²
	 */
	float_t rest_th_acc;

	/**
	 * @brief Time constant for current norm/dip value in magnetic disturbance detection (in seconds).
	 *
	 * This (very fast) low-pass filter is intended to provide additional robustness when the magnetometer measurements
	 * are noisy or not sampled perfectly in sync with the gyroscope measurements. Set to -1 to disable the low-pass
	 * filter and directly use the magnetometer measurements.
	 *
	 * Default value: 0.05 s
	 */
	float_t mag_current_tau;
	/**
	 * @brief Time constant for the adjustment of the magnetic field reference (in seconds).
	 *
	 * This adjustment allows the reference estimate to converge to the observed undisturbed field.
	 *
	 * Default value: 20.0 s
	 */
	float_t mag_ref_tau;
	/**
	 * @brief Relative threshold for the magnetic field strength for magnetic disturbance detection.
	 *
	 * This value is relative to the reference norm.
	 *
	 * Default value: 0.1 (10%)
	 */
	float_t mag_nrm_th;
	/**
	 * @brief Threshold for the magnetic field dip angle for magnetic disturbance detection (in degrees).
	 *
	 * Default vaule: 10 °
	 */
	float_t mag_dip_th;
	/**
	 * @brief Duration after which to accept a different homogeneous magnetic field (in seconds).
	 *
	 * A different magnetic field reference is accepted as the new field when the measurements are within the thresholds
	 * #magNormTh and #magDipTh for the given time. Additionally, only phases with sufficient movement, specified by
	 * #magNewMinGyr, count.
	 *
	 * Default value: 20.0
	 */
	float_t mag_new_time;
	/**
	 * @brief Duration after which to accept a homogeneous magnetic field for the first time (in seconds).
	 *
	 * This value is used instead of #magNewTime when there is no current estimate in order to allow for the initial
	 * magnetic field reference to be obtained faster.
	 *
	 * Default value: 5.0
	 */
	float_t mag_new_first_time;
	/**
	 * @brief Minimum angular velocity needed in order to count time for new magnetic field acceptance (in °/s).
	 *
	 * Durations for which the angular velocity norm is below this threshold do not count towards reaching #magNewTime.
	 *
	 * Default value: 20.0 °/s
	 */
	float_t mag_new_min_gyr;
	/**
	 * @brief Minimum duration within thresholds after which to regard the field as undisturbed again (in seconds).
	 *
	 * Default value: 0.5 s
	 */
	float_t mag_min_undisturbed_time;
	/**
	 * @brief Maximum duration of full magnetic disturbance rejection (in seconds).
	 *
	 * For magnetic disturbances up to this duration, heading correction is fully disabled and heading changes are
	 * tracked by gyroscope only. After this duration (or for many small disturbed phases without sufficient time in the
	 * undisturbed field in between), the heading correction is performed with an increased time constant (see
	 * #magRejectionFactor).
	 *
	 * Default value: 60.0 s
	 */
	float_t mag_max_rejection_time;
	/**
	 * @brief Factor by which to slow the heading correction during long disturbed phases.
	 *
	 * After #magMaxRejectionTime of full magnetic disturbance rejection, heading correction is performed with an
	 * increased time constant. This parameter (approximately) specifies the factor of the increase.
	 *
	 * Furthermore, after spending #magMaxRejectionTime/#magRejectionFactor seconds in an undisturbed magnetic field,
	 * the time is reset and full magnetic disturbance rejection will be performed for up to #magMaxRejectionTime again.
	 *
	 * Default value: 2.0
	 */
	float_t mag_rejection_factor;
} vqf_params_t;

/**
 * @brief The default value of the VQF parameters.
 */
extern const vqf_params_t vqf_params_default;

/**
 * @brief Struct containing the filter state of the VQF class.
 *
 * The relevant parts of the state can be accessed via functions of the VQF class, e.g. VQF::getQuat6D(),
 * VQF::getQuat9D(), VQF::getGyrBiasEstimate(), VQF::setGyrBiasEstimate(), VQF::getRestDetected() and
 * VQF::getMagDistDetected(). To reset the state to the initial values, use VQF::resetState().
 *
 * Direct access to the full state is typically not needed but can be useful in some cases, e.g. for debugging. For this
 * purpose, the state can be accessed by VQF::getState() and set by VQF::setState().
 */
typedef struct {
	/**
	 * @brief Angular velocity strapdown integration quaternion \f$^{\mathcal{S}_i}_{\mathcal{I}_i}\mathbf{q}\f$.
	 */
	vqf_quaternion_handle_t gyr_quat;
	/**
	 * @brief The mean deviation of the gyroscope angular velocity used in rest detection.
	 */
	vqf_matrix_handle_t gyr_mean_deviation;
	/**
	 * @brief The gyroscope angular velocity without estimated bias.
	 */
	vqf_matrix_handle_t gyr_no_bias;
	/**
	 * @brief Incremental quat from gyr_no_bias used in gyroscope strapdown integration quaternion update.
	 */
	vqf_quaternion_handle_t gyr_step_quat;

	/**
	 * @brief Inclination correction quaternion \f$^{\mathcal{I}_i}_{\mathcal{E}_i}\mathbf{q}\f$.
	 */
	vqf_quaternion_handle_t acc_quat;
	/**
	 * @brief The mean deviation of the acceleration used in rest detection	.
	 */
	vqf_matrix_handle_t acc_mean_deviation;
	/**
	 * @brief Inertial frame acceleration used in low-pass filtering of accelerometer update.
	 */
	vqf_matrix_handle_t acc_earth;
	/**
	 * @brief Inclination correction quat used in acc_quat updating of accelerometer update.
	 */
	vqf_quaternion_handle_t acc_corr_quat;

	/**
	 * The 6D quat used in motion bias estimation and magnetometer low-pass filtering.
	 */
	vqf_quaternion_handle_t acc_gyr_quat;

	/**
	 * @brief Magnetometer measurement in 6D earth frame for low-pass filtering in magnetometer update.
	 */
	vqf_matrix_handle_t mag_earth;

	/**
	 * @brief Heading difference \f$\delta\f$ between \f$\mathcal{E}_i\f$ and \f$\mathcal{E}\f$.
	 *
	 * \f$^{\mathcal{E}_i}_{\mathcal{E}}\mathbf{q} = \begin{bmatrix}\cos\frac{\delta}{2} & 0 & 0 &
	 * \sin\frac{\delta}{2}\end{bmatrix}^T\f$.
	 */
	float_t delta;
	/**
	 * @brief True if it has been detected that the IMU is currently at rest.
	 *
	 * Used to switch between rest and motion gyroscope bias estimation.
	 */
	uint8_t rest_detected;
	/**
	 * @brief True if magnetic disturbances have been detected.
	 */
	uint8_t mag_dist_detected;

	/**
	 * @brief Last low-pass filtered acceleration in the \f$\mathcal{I}_i\f$ frame.
	 */
	vqf_matrix_handle_t last_acc_lp; // float_t last_acc_lp[3];
	/**
	 * @brief Internal low-pass filter state for #lastAccLp.
	 */
	vqf_matrix_double_handle_t acc_lp_state[4U]; // double_t acc_lp_state[3 * 2];
	/**
	 * @brief Last inclination correction angular rate.
	 *
	 * Change to inclination correction quaternion \f$^{\mathcal{I}_i}_{\mathcal{E}_i}\mathbf{q}\f$ performed in the
	 * last accelerometer update, expressed as an angular rate (in rad/s).
	 */
	float_t last_acc_corr_angular_rate;

	/**
	 * @brief Gain used for heading correction to ensure fast initial convergence.
	 *
	 * This value is used as the gain for heading correction in the beginning if it is larger than the normal filter
	 * gain. It is initialized to 1 and then updated to 0.5, 0.33, 0.25, ... After VQFParams::tauMag seconds, it is
	 * set to zero.
	 */
	float_t k_mag_init;
	/**
	 * @brief Last heading disagreement angle.
	 *
	 * Disagreement between the heading \f$\hat\delta\f$ estimated from the last magnetometer sample and the state
	 * \f$\delta\f$ (in rad).
	 */
	float_t last_mag_dis_angle;
	/**
	 * @brief Last heading correction angular rate.
	 *
	 * Change to heading \f$\delta\f$ performed in the last magnetometer update,
	 * expressed as an angular rate (in rad/s).
	 */
	float_t last_mag_corr_angular_rate;

	/**
	 * @brief Current gyroscope bias estimate (in rad/s).
	 */
	vqf_matrix_handle_t bias; // float_t bias[3];
#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	/**
	 * @brief Covariance matrix of the gyroscope bias estimate.
	 *
	 * The 3x3 matrix is stored in row-major order. Note that for numeric reasons the internal unit used is 0.01 °/s,
	 * i.e. to get the standard deviation in degrees per second use \f$\sigma = \frac{\sqrt{p_{ii}}}{100}\f$.
	 */
	vqf_matrix_handle_t bias_P; // float_t bias_P[9];

	/**
	 * @brief Internal state of the Butterworth low-pass filter for the rotation matrix coefficients used in motion
	 * bias estimation.
	 */
	vqf_matrix_double_handle_t motion_bias_est_R_lp_state[4U]; // double_t motion_bias_est_R_lp_state[9 * 2];
	/**
	 * @brief Internal low-pass filter state for the rotated bias estimate used in motion bias estimation.
	 */
	vqf_matrix_double_handle_t motion_bias_est_bias_lp_state[4U]; // double_t motion_bias_est_bias_lp_state[2 * 2];

	/**
	 * @brief Rotation matrix used in motion bias estimation.
	 */
	vqf_matrix_handle_t motion_bias_est_R;
	/**
	 * @brief Transpose of the rotation matrix used in motion bias estimation.
	 */
	vqf_matrix_handle_t motion_bias_est_Rt;
	/**
	 * @brief Low-pass filtered R*b_hat used in motion bias estimation.
	 */
	vqf_matrix_handle_t motion_bias_est_bias_lp;
	/**
	 * @brief Measurement noise variance matrix of the kalman filter used in motion bias estimation.
	 */
	vqf_matrix_handle_t motion_bias_est_W;
	/**
	 * @brief Kalman gain of the kalman filter used in motion bias estimation.
	 */
	vqf_matrix_handle_t motion_bias_est_K;
	/**
	 * @brief Correction from the kalman gain and innovation used in motion bias estimation.
	 */
	vqf_matrix_handle_t motion_bias_est_corr;
#else
	// If only rest gyr bias estimation is enabled, P and K of the KF are always diagonal
	// and matrix inversion is not needed. If motion bias estimation is disabled at compile
	// time, storing the full P matrix is not necessary.
	float_t bias_P;
#endif

	/**
	 * @brief Measurement error of the kalman filter used in motion bias estimation.
	 */
	vqf_matrix_handle_t motion_bias_est_e;

	/**
	 * @brief Last (squared) deviations from the reference of the last sample used in rest detection.
	 *
	 * Looking at those values can be useful to understand how rest detection is working and which thresholds are
	 * suitable. The array contains the last values for gyroscope and accelerometer in the respective
	 * units. Note that the values are squared.
	 *
	 * The method VQF::getRelativeRestDeviations() provides an easier way to obtain and interpret those values.
	 */
	float_t rest_last_squared_deviations[2U];
	/**
	 * @brief The current duration for which all sensor readings are within the rest detection thresholds.
	 *
	 * Rest is detected if this value is larger or equal to VQFParams::restMinT.
	 */
	float_t rest_t;
	/**
	 * @brief Last low-pass filtered gyroscope measurement used as the reference for rest detection.
	 *
	 * Note that this value is also used for gyroscope bias estimation when rest is detected.
	 */
	vqf_matrix_handle_t rest_last_gyr_lp; // float_t rest_last_gyr_lp[3];
	/**
	 * @brief Internal low-pass filter state for #restLastGyrLp.
	 */
	vqf_matrix_double_handle_t rest_gyr_lp_state[4U]; // double_t rest_gyr_lp_state[3 * 2];
	/**
	 * @brief Last low-pass filtered accelerometer measurement used as the reference for rest detection.
	 */
	vqf_matrix_handle_t rest_last_acc_lp; // float_t rest_last_acc_lp[3];
	/**
	 * @brief Internal low-pass filter state for #restLastAccLp.
	 */
	vqf_matrix_double_handle_t rest_acc_lp_state[4U]; // double_t rest_acc_lp_state[3 * 2];

	/**
	 * @brief Norm of the currently accepted magnetic field reference.
	 *
	 * A value of -1 indicates that no homogeneous field is found yet.
	 */
	float_t mag_ref_nrm;
	/**
	 * @brief Dip angle of the currently accepted magnetic field reference.
	 */
	float_t mag_ref_dip;
	/**
	 * @brief The current duration for which the current norm and dip are close to the reference.
	 *
	 * The magnetic field is regarded as undisturbed when this value reaches VQFParams::magMinUndisturbedTime.
	 */
	float_t mag_undisturbed_t;
	/**
	 * @brief The current duration for which the magnetic field was rejected.
	 *
	 * If the magnetic field is disturbed and this value is smaller than VQFParams::magMaxRejectionTime, heading
	 * correction updates are fully disabled.
	 */
	float_t mag_reject_t;
	/**
	 * @brief Norm of the alternative magnetic field reference currently being evaluated.
	 */
	float_t mag_candidate_nrm;
	/**
	 * @brief Dip angle of the alternative magnetic field reference currently being evaluated.
	 */
	float_t mag_candidate_dip;
	/**
	 * @brief The current duration for which the norm and dip are close to the candidate.
	 *
	 * If this value exceeds VQFParams::magNewTime (or VQFParams::magNewFirstTime if #magRefNorm < 0), the current
	 * candidate is accepted as the new reference.
	 */
	float_t mag_candidate_t;
	/**
	 * @brief Norm and dip angle of the current magnetometer measurements.
	 *
	 * Slightly low-pass filtered, see VQFParams::magCurrentTau.
	 */
	vqf_matrix_handle_t mag_nrm_dip;
	/**
	 * @brief Internal low-pass filter state for the current norm and dip angle.
	 */
	vqf_matrix_double_handle_t mag_nrm_dip_lp_state[4U]; // double_t mag_norm_dip_lp_state[2 * 2];
} vqf_state_t;

/**
 * @brief Struct containing coefficients used by the VQF class.
 *
 * Coefficients are values that depend on the parameters and the sampling times, but do not change during update steps.
 * They are calculated in VQF::setup().
 */
typedef struct {
	/**
	* @brief Sampling time of the gyroscope measurements (in seconds).
	*/
	float_t gyr_ts;
	/**
	* @brief Sampling time of the accelerometer measurements (in seconds).
	*/
	float_t acc_ts;
	/**
	* @brief Sampling time of the magnetometer measurements (in seconds).
	*/
	float_t mag_ts;

	/**
	* @brief Numerator coefficients of the acceleration low-pass filter.
	*
	* The array contains \f$\begin{bmatrix}b_0 & b_1 & b_2\end{bmatrix}\f$.
	*/
	double_t acc_lp_B[3];
	/**
	* @brief Denominator coefficients of the acceleration low-pass filter.
	*
	* The array contains \f$\begin{bmatrix}a_1 & a_2\end{bmatrix}\f$ and \f$a_0=1\f$.
	*/
	double_t acc_lp_A[2];

	/**
	* @brief Gain of the first-order filter used for heading correction.
	*/
	float_t k_mag;

	/**
	* @brief Variance of the initial gyroscope bias estimate.
	*/
	float_t bias_P0;
	/**
	* @brief System noise variance used in gyroscope bias estimation.
	*/
	float_t bias_V;
#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	/**
	 * @brief Cross-product matrix of the ENU up axis used in motion bias estimation.
	 */
	vqf_matrix_handle_t motion_bias_est_skew_ez;
	/**
	* @brief Measurement noise variance for the motion gyroscope bias estimation update.
	*/
	float_t bias_motion_W;
	/**
	* @brief Measurement noise variance for the motion gyroscope bias estimation update in vertical direction.
	*/
	float_t bias_vertical_W;
#endif
	/**
	* @brief Measurement noise variance for the rest gyroscope bias estimation update.
	*/
	float_t bias_rest_W;

	/**
	* @brief Numerator coefficients of the gyroscope measurement low-pass filter for rest detection.
	*/
	double_t rest_gyr_lp_B[3];
	/**
	* @brief Denominator coefficients of the gyroscope measurement low-pass filter for rest detection.
	*/
	double_t rest_gyr_lp_A[2];
	/**
	* @brief Numerator coefficients of the accelerometer measurement low-pass filter for rest detection.
	*/
	double_t rest_acc_lp_B[3];
	/**
	* @brief Denominator coefficients of the accelerometer measurement low-pass filter for rest detection.
	*/
	double_t rest_acc_lp_A[2];

	/**
	* @brief Gain of the first-order filter used for to update the magnetic field reference and candidate.
	*/
	float_t k_mag_ref;
	/**
	* @brief Numerator coefficients of the low-pass filter for the current magnetic norm and dip.
	*/
	double_t mag_nrm_dip_lp_B[3];
	/**
	* @brief Denominator coefficients of the low-pass filter for the current magnetic norm and dip.
	*/
	double_t mag_nrm_dip_lp_A[2];
} vqf_coefficients_t;

/**
 * @brief A Versatile Quaternion-based Filter for IMU Orientation Estimation.
 *
 * \rst
 * This class implements the orientation estimation filter described in the following publication:
 *
 *
 *     D. Laidig and T. Seel. "VQF: Highly Accurate IMU Orientation Estimation with Bias Estimation and Magnetic
 *     Disturbance Rejection." Information Fusion 2023, 91, 187--204.
 *     `doi:10.1016/j.inffus.2022.10.014 <https://doi.org/10.1016/j.inffus.2022.10.014>`_.
 *     [Accepted manuscript available at `arXiv:2203.17024 <https://arxiv.org/abs/2203.17024>`_.]
 *
 * The filter can perform simultaneous 6D (magnetometer-free) and 9D (gyr+acc+mag) sensor fusion and can also be used
 * without magnetometer data. It performs rest detection, gyroscope bias estimation during rest and motion, and magnetic
 * disturbance detection and rejection. Different sampling rates for gyroscopes, accelerometers, and magnetometers are
 * supported as well. While in most cases, the defaults will be reasonable, the algorithm can be influenced via a
 * number of tuning parameters.
 *
 * To use this C++ implementation,
 *
 * 1. create a instance of the class and provide the sampling time and, optionally, parameters
 * 2. for every sample, call one of the update functions to feed the algorithm with IMU data
 * 3. access the estimation results with :meth:`getQuat6D() <VQF.getQuat6D>`, :meth:`getQuat9D() <VQF.getQuat9D>` and
 *    the other getter methods.
 *
 * If the full data is available in (row-major) data buffers, you can use :meth:`updateBatch() <VQF.updateBatch>`.
 *
 * This class is the main C++ implementation of the algorithm. Depending on use case and programming language of choice,
 * the following alternatives might be useful:
 *
 * +------------------------+--------------------------+--------------------------+---------------------------+
 * |                        | Full Version             | Basic Version            | Offline Version           |
 * |                        |                          |                          |                           |
 * +========================+==========================+==========================+===========================+
 * | **C++**                | **VQF (this class)**     | :cpp:class:`BasicVQF`    | :cpp:func:`offlineVQF`    |
 * +------------------------+--------------------------+--------------------------+---------------------------+
 * | **Python/C++ (fast)**  | :py:class:`vqf.VQF`      | :py:class:`vqf.BasicVQF` | :py:meth:`vqf.offlineVQF` |
 * +------------------------+--------------------------+--------------------------+---------------------------+
 * | **Pure Python (slow)** | :py:class:`vqf.PyVQF`    | --                       | --                        |
 * +------------------------+--------------------------+--------------------------+---------------------------+
 * | **Pure Matlab (slow)** | :mat:class:`VQF.m <VQF>` | --                       | --                        |
 * +------------------------+--------------------------+--------------------------+---------------------------+
 * \endrst
 */
typedef struct {
	/**
	 * @brief Contains pointers of implemented linear algebra functions.
	 */
	vqf_linear_algebra_t linear_algebra;

	/**
	 * @brief Contains the current coefficients (calculated in #vqf_context_new).
	 */
	vqf_coefficients_t coefficients;

	/**
	 * @brief Contains the current parameters.
	 *
	 * See #getParams. To set parameters, pass them to the constructor. Part of the parameters can be changed with
	 * #vqf_set_tau_acc, #set_tau_mag, #set_motion_bias_est_enabled, #set_rest_bias_est_enabled, #set_mag_dist_rejection_enabled, and
	 * #set_rest_detection_thresholds.
	 */
	vqf_params_t params;

	/**
	 * @brief Contains the current state.
	 *
	 * See #vqf_reset_state.
	 */
	vqf_state_t state;
} vqf_context_t;

/**
 * @brief Performs gyroscope update step.
 *
 * It is only necessary to call this function directly if gyroscope, accelerometers and magnetometers have
 * different sampling rates. Otherwise, simply use #update().
 *
 * @param vqf_context	the context of the VQF filter.
 * @param gyr			gyroscope measurement in rad/s
 */
void vqf_update_gyr(
	vqf_context_t*		vqf_context,
	vqf_matrix_handle_t	gyr
);

/**
 * @brief Performs accelerometer update step.
 *
 * It is only necessary to call this function directly if gyroscope, accelerometers and magnetometers have
 * different sampling rates. Otherwise, simply use #update().
 *
 * Should be called after #updateGyr and before #updateMag.
 *
 * @param vqf_context	the context of the VQF filter.
 * @param acc			accelerometer measurement in m/s²
 */
void vqf_update_acc(
	vqf_context_t*		vqf_context,
	vqf_matrix_handle_t	acc
);

/**
 * @brief Performs magnetometer update step.
 *
 * It is only necessary to call this function directly if gyroscope, accelerometers and magnetometers have
 * different sampling rates. Otherwise, simply use #update().
 *
 * Should be called after #updateAcc.
 *
 * @param vqf_context	the context of the VQF filter.
 * @param mag			magnetometer measurement in arbitrary units
 */
void vqf_update_mag(
	vqf_context_t*		vqf_context,
	vqf_matrix_handle_t	mag
);

/**
 * @brief				Performs filter update step for one sample (magnetometer-free).
 * @param vqf_context	the context of the VQF filter.
 * @param gyr			gyroscope measurement in rad/s
 * @param acc			accelerometer measurement in m/s²
 */
void vqf_update_6D(
	vqf_context_t*		vqf_context,
	vqf_matrix_handle_t	gyr,
	vqf_matrix_handle_t	acc
);

/**
 * @brief				Performs filter update step for one sample (with magnetometer measurement).
 * @param vqf_context	the context of the VQF filter.
 * @param gyr			gyroscope measurement in rad/s
 * @param acc			accelerometer measurement in m/s²
 * @param mag			magnetometer measurement in arbitrary units
 */
void vqf_update_9D(
	vqf_context_t*		vqf_context,
	vqf_matrix_handle_t	gyr,
	vqf_matrix_handle_t	acc,
	vqf_matrix_handle_t	mag
);

/**
 * @brief				Returns the angular velocity strapdown integration quaternion
 *						\f$^{\mathcal{S}_i}_{\mathcal{I}_i}\mathbf{q}\f$.
 * @param vqf_context	the context of the VQF filter.
 * @param out			output array for the quaternion
 */
void vqf_get_quat_3D(
	const	vqf_context_t*			vqf_context,
			vqf_quaternion_handle_t	out
);

/**
 * @brief				Returns the 6D (magnetometer-free) orientation quaternion
 *						\f$^{\mathcal{S}_i}_{\mathcal{E}_i}\mathbf{q}\f$.
 * @param vqf_context	the context of the VQF filter.
 * @param out			output array for the quaternion
 */
void vqf_get_quat_6D(
	const	vqf_context_t*			vqf_context,
			vqf_quaternion_handle_t	out
);

/**
 * @brief				Returns the 9D (with magnetometers) orientation quaternion
 *						\f$^{\mathcal{S}_i}_{\mathcal{E}}\mathbf{q}\f$.
 * @param vqf_context	the context of the VQF filter.
 * @param out			output array for the quaternion
 */
void vqf_get_quat_9D(
	const	vqf_context_t*			vqf_context,
			vqf_quaternion_handle_t	out
);

/**
 * @brief Returns the heading difference \f$\delta\f$ between \f$\mathcal{E}_i\f$ and \f$\mathcal{E}\f$.
 *
 * \f$^{\mathcal{E}_i}_{\mathcal{E}}\mathbf{q} = \begin{bmatrix}\cos\frac{\delta}{2} & 0 & 0 &
 * \sin\frac{\delta}{2}\end{bmatrix}^T\f$.
 *
 * @param vqf_context	the context of the VQF filter.
 * @return				delta angle in rad (VQFState::delta)
 */
float_t vqf_getDelta(const vqf_context_t* vqf_context);

/**
 * @brief Returns the current gyroscope bias estimate and the uncertainty.
 *
 * The returned standard deviation sigma represents the estimation uncertainty in the worst direction and is based
 * on an upper bound of the largest eigenvalue of the covariance matrix.
 *
 * @param vqf_context	the context of the VQF filter.
 * @param out			output array for the gyroscope bias estimate (rad/s)
 * @return				standard deviation sigma of the estimation uncertainty (rad/s)
 */
float_t vqf_get_bias_estimate(
	const	vqf_context_t*		vqf_context,
			vqf_matrix_handle_t	out
);

/**
 * @brief Sets the current gyroscope bias estimate and the uncertainty.
 *
 * If a value for the uncertainty sigma is given, the covariance matrix is set to a corresponding scaled identity
 * matrix.
 *
 * @param vqf_context	the context of the VQF filter.
 * @param bias			gyroscope bias estimate (rad/s)
 * @param sigma			standard deviation of the estimation uncertainty (rad/s) - set to -1 (default) in order to not
 *						change the estimation covariance matrix
 */
void vqf_set_bias_estimate(
	const	vqf_context_t*		vqf_context,
			vqf_matrix_handle_t	bias,
			float_t				sigma
);

/**
 * @brief				Returns true if rest was detected.
 * @param vqf_context	the context of the VQF filter.
 */
uint8_t vqf_get_rest_detected(const vqf_context_t* vqf_context);

/**
 * @brief				Returns true if a disturbed magnetic field was detected.
 * @param vqf_context	the context of the VQF filter.
 */
uint8_t vqf_get_mag_dist_detected(const vqf_context_t* vqf_context);

/**
 * @brief Returns the relative deviations used in rest detection.
 *
 * Looking at those values can be useful to understand how rest detection is working and which thresholds are
 * suitable. The output array is filled with the last values for gyroscope and accelerometer,
 * relative to the threshold. In order for rest to be detected, both values must stay below 1.
 *
 * @param vqf_context	the context of the VQF filter.
 * @param out			output array of size 2 for the relative rest deviations
  */
void vqf_get_relative_rest_deviations(
	const	vqf_context_t*	vqf_context,
			float_t			out[2]
);

/**
 * @brief				Returns the norm of the currently accepted magnetic field reference.
 * @param vqf_context	the context of the VQF filter.
 */
float_t vqf_get_mag_ref_norm(const vqf_context_t* vqf_context);

/**
 * @brief				Returns the dip angle of the currently accepted magnetic field reference.
 * @param vqf_context	the context of the VQF filter.
 */
float_t vqf_get_mag_ref_dip(const vqf_context_t* vqf_context);

/**
 * @brief				Overwrites the current magnetic field reference.
 * @param vqf_context	the context of the VQF filter.
 * @param norm			norm of the magnetic field reference
 * @param dip			dip angle of the magnetic field reference
 */
void vqf_set_mag_ref(
	vqf_context_t*	vqf_context,
	float_t			norm,
	float_t			dip
);

/**
 * @brief Sets the time constant for accelerometer low-pass filtering.
 *
 * For more details, see vqf_params.tau_acc.
 *
 * @param vqf_context	the context of the VQF filter.
 * @param tau_acc		time constant \f$\tau_\mathrm{acc}\f$ in seconds
 */
void vqf_set_tau_acc(
	vqf_context_t*	vqf_context,
	float_t			tau_acc
);

/**
 * @brief Sets the time constant for the magnetometer update.
 *
 * For more details, see vqf_params.tau_mag.
 *
 * @param vqf_context	the context of the VQF filter.
 * @param tau_mag		time constant \f$\tau_\mathrm{mag}\f$ in seconds
 */
void vqf_set_tau_mag(
	vqf_context_t*	vqf_context,
	float_t			tau_mag
);

#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
/**
 * @brief				Enables/disabled gyroscope bias estimation during motion.
 * @param vqf_context	the context of the VQF filter.
 * @param enabled		state of motion bias estimation.
 */
void vqf_set_motion_bias_est_enabled(
	vqf_context_t*	vqf_context,
	uint8_t			enabled
);
#endif

/**
 * @brief				Enables/disables rest detection and bias estimation during rest.
 * @param vqf_context	the context of the VQF filter.
 * @param enabled		state of resst detection and bias estimation during resst.
 */
void vqf_set_rest_bias_est_enabled(
	vqf_context_t*	vqf_context,
	uint8_t			enabled
);

/**
 * @brief				Enables/disables magnetic disturbance detection and rejection.
 * @param vqf_context	the context of the VQF filter.
 * @param enabled		state of magnetic disturbance detection and rejection.
 */
void vqf_set_mag_dist_rejection_enabled(
	vqf_context_t*	vqf_context,
	uint8_t			enabled
);

/**
 * @brief Sets the current thresholds for rest detection.
 *
 * For details about the parameters, see vqf_params.tau_rest_th_gyr and vqf_params.tau_rest_th_acc.
 *
 * @param vqf_context	the context of the VQF filter.
 * @param th_gyr		new angular velocity threshold for rest detection (in °/s).
 * @param th_acc		new acceleration threshold for rest detection (in m/s²).
 */
void vqf_set_rest_detection_thresholds(
	vqf_context_t*	vqf_context,
	float_t			th_gyr,
	float_t			th_acc
);

/**
 * @brief Resets the state to the default values at initialization.
 *
 * Resetting the state is equivalent to creating a new instance of this class.
 *
 * @param vqf_context the context of the VQF filter.
 */
void vqf_reset_state(vqf_context_t*	vqf_context);

/**
 * @brief Calculates the gain for a first-order low-pass filter from the 1/e time constant.
 *
 * \f$k = 1 - \exp\left(-\frac{T_\mathrm{s}}{\tau}\right)\f$
 *
 * The cutoff frequency of the resulting filter is \f$f_\mathrm{c} = \frac{1}{2\pi\tau}\f$.
 *
 * @param tau	time constant \f$\tau\f$ in seconds - use -1 to disable update (\f$k=0\f$) or 0 to obtain
 *				unfiltered values (\f$k=1\f$)
 * @param Ts	sampling time \f$T_\mathrm{s}\f$ in seconds
 * @return		filter gain *k*
 */
float_t vqf_gain_from_tau(
	float_t tau,
	float_t Ts
);
/**
 * @brief Calculates coefficients for a second-order Butterworth low-pass filter.
 *
 * The filter is parametrized via the time constant of the dampened, non-oscillating part of step response and the
 * resulting cutoff frequency is \f$f_\mathrm{c} = \frac{\sqrt{2}}{2\pi\tau}\f$.
 *
 * When \f$\tau < \frac{T_\mathrm{s}}{2}\f$ (which corresponds to \f$f_\mathrm{c}\f$ exceeding 90 % of the
 * Nyquist frequency), a direct passthrough fallback is used to prevent instability.
 *
 * @param tau	time constant \f$\tau\f$ in seconds
 * @param Ts	sampling time \f$T_\mathrm{s}\f$ in seconds
 * @param outB	output array for numerator coefficients
 * @param outA	output array for denominator coefficients (without \f$a_0=1\f$)
 */
void vqf_filter_coefficients(
	float_t		tau,
	float_t		Ts,
	double_t	outB[3],
	double_t	outA[2]
);

/**
 * @brief Adjusts the filter state when changing coefficients.
 *
 * This function assumes that the filter is currently in a steady state, i.e. the last input values and the last
 * output values are all equal. Based on this, the filter state is adjusted to new filter coefficients so that the
 * output does not jump.
 *
 * @param vqf_context	the context of the VQF filter.
 * @param last_y		last filter output values (array of size N)
 * @param b_old			previous numerator coefficients
 * @param a_old			previous denominator coefficients (without \f$a_0=1\f$)
 * @param b_new			new numerator coefficients
 * @param a_new			new denominator coefficients (without \f$a_0=1\f$)
 * @param state			filter state (4 state matrices, will be modified)
 */
void vqf_filter_adapt_state_for_coeff_change(
	const	vqf_context_t*				vqf_context,
			vqf_matrix_handle_t			last_y,
	const	double_t					b_old	[3U],
	const	double_t					a_old	[2U],
	const	double_t					b_new	[3U],
	const	double_t					a_new	[2U],
			vqf_matrix_double_handle_t	state	[4U]
);

/**
 * @brief Performs filter step for vector-valued signal with averaging-based initialization.
 *
 * During the first \f$\tau\f$ seconds, the filter output is the mean of the previous samples. At \f$t=\tau\f$, the
 * initial conditions for the low-pass filter are calculated based on the current mean value and from then on,
 * regular filtering with the rational transfer function described by the coefficients b and a is performed.
 *
 * @param vqf_context	the context of the VQF filter.
 * @param x				input values (array of size N)
 * @param tau			filter time constant \f$\tau\f$ in seconds (used for initialization)
 * @param Ts			sampling time \f$T_\mathrm{s}\f$ in seconds (used for initialization)
 * @param b				numerator coefficients
 * @param a				denominator coefficients (without \f$a_0=1\f$)
 * @param state			filter state (4 state matrices, will be modified)
 * @param out			output array for filtered values (size N)
 */
void vqf_filter_vec(
	const	vqf_context_t*				vqf_context,
			vqf_matrix_handle_t			x,
			float_t						tau,
			float_t						Ts,
	const	double_t					b		[3],
	const	double_t					a		[2],
			vqf_matrix_double_handle_t	state	[4],
			vqf_matrix_handle_t			out
);

/**
 * @brief					Create and setup the VQF context.
 * @param linear_algebra	vqf_linear_algebra_t struct containing linear algebra functions used by the VQF context.
 * @param params			vqf_params_t struct containing the desired parameters
 * @param gyr_ts			sampling time of the gyroscope measurements in seconds
 * @param acc_ts			sampling time of the accelerometer measurements in seconds (the value of `gyrTs` is used if set to -1)
 * @param mag_ts			sampling time of the magnetometer measurements in seconds (the value of `gyrTs` is used if set to -1)
 * @return					created VQF context.
 */
vqf_context_t* vqf_context_new(
	const	vqf_linear_algebra_t*	linear_algebra,
	const	vqf_params_t*			params,
			float_t					gyr_ts,
			float_t					acc_ts,
			float_t					mag_ts
);

/**
 * @brief				Release the VQF context.
 * @param vqf_context	VQF context to be released.
 */
void vqf_context_del(vqf_context_t* vqf_context);

#ifdef __cplusplus
}
#endif

#endif //VQF_H
