#include "assert.h"
#include "float.h"
#include "stdbool.h"
#include "stdlib.h"
#include "tgmath.h"
#include "vqf.h"

#define PI		3.14159265358979323846264338327950288
#define SQRT2	1.41421356237309504880168872420969808

const vqf_params_t vqf_params_default = {
	.tau_acc							= 3.0f,
	.tau_mag							= 9.0f,
	.motion_bias_est_enabled			= true,
	.rest_bias_est_enabled				= true,
	.mag_dist_rejection_enabled			= true,
	.bias_sigma_init					= 0.5f,
	.bias_forgetting_time				= 100.0f,
	.bias_clip							= 2.0f,
#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	.bias_sigma_motion					= 0.1f,
	.bias_vertical_forgetting_factor	= 0.0001f,
#endif
	.bias_sigma_rest					= 0.03f,
	.rest_min_t							= 1.5f,
	.rest_filter_tau					= 0.5f,
	.rest_th_gyr						= 2.0f,
	.rest_th_acc						= 0.5f,
	.mag_current_tau					= 0.05f,
	.mag_ref_tau						= 20.0f,
	.mag_nrm_th							= 0.1f,
	.mag_dip_th							= 10.0f,
	.mag_new_time						= 20.0f,
	.mag_new_first_time					= 5.0f,
	.mag_new_min_gyr					= 20.0f,
	.mag_min_undisturbed_time			= 0.5f,
	.mag_max_rejection_time				= 60.0f,
	.mag_rejection_factor				= 2.0f
};

static inline float_t square(float_t x) {
	return x * x;
}

void vqf_update_gyr(
	vqf_context_t*		vqf_context,
	vqf_matrix_handle_t	gyr
) {
	// Cache the VQF context to stack.
	const	vqf_linear_algebra_t*	vqf_linear_algebra	= &vqf_context->linear_algebra;
	const	vqf_coefficients_t*		vqf_coeff			= &vqf_context->coefficients;
	const	vqf_params_t*			vqf_param			= &vqf_context->params;
			vqf_state_t*			vqf_state			= &vqf_context->state;

	// rest detection
	if (	vqf_param->rest_bias_est_enabled
		||	vqf_param->mag_dist_rejection_enabled
	) {
		vqf_filter_vec(
			/* vqf_context	= */ vqf_context,
			/* x			= */ gyr,
			/* tau			= */ vqf_param->rest_filter_tau,
			/* Ts			= */ vqf_coeff->gyr_ts,
			/* b			= */ vqf_coeff->rest_gyr_lp_B,
			/* a			= */ vqf_coeff->rest_gyr_lp_A,
			/* state		= */ vqf_state->rest_gyr_lp_state,
			/* out			= */ vqf_state->rest_last_gyr_lp
		);

		vqf_linear_algebra->subtract_matrix(
			/* left_matrix			= */ gyr,
			/* right_matrix			= */ vqf_state->rest_last_gyr_lp,
			/* destination_matrix	= */ vqf_state->gyr_mean_deviation
		);

		vqf_state->rest_last_squared_deviations[0U] = vqf_linear_algebra->get_vector_squared_norm(vqf_state->gyr_mean_deviation);

		const float_t bias_clip = vqf_param->bias_clip * (float_t) (PI / 180.0);

		if (		vqf_state->rest_last_squared_deviations[0U] >= square(vqf_param->rest_th_gyr * (float_t) (PI / 180.0))
				||	fabs(vqf_linear_algebra->get_matrix_coefficient(vqf_state->rest_last_gyr_lp, 0U, 0U)) > bias_clip
				||	fabs(vqf_linear_algebra->get_matrix_coefficient(vqf_state->rest_last_gyr_lp, 1U, 0U)) > bias_clip
				||	fabs(vqf_linear_algebra->get_matrix_coefficient(vqf_state->rest_last_gyr_lp, 2U, 0U)) > bias_clip
		) {
			vqf_state->rest_t			= 0.0f;
			vqf_state->rest_detected	= false;
		}
	}

	// remove estimated gyro bias
	vqf_linear_algebra->subtract_matrix(
		/* left_matrix			= */ gyr,
		/* right_matrix			= */ vqf_state->bias,
		/* destination_matrix	= */ vqf_state->gyr_no_bias
	);

	// gyroscope prediction step
	const float_t gyro_norm	= vqf_linear_algebra->get_vector_norm(vqf_state->gyr_no_bias);
	const float_t angle		= gyro_norm * vqf_coeff->gyr_ts;

	if (gyro_norm > FLT_EPSILON) {
		// Normalize the axis.
		vqf_linear_algebra->multiply_matrix_scalar_in_place(
			/* source_matrix	= */ vqf_state->gyr_no_bias,
			/* value			= */ 1 / gyro_norm
		);

		vqf_linear_algebra->set_quaternion_rotation(
			/* rotation_angle_radians	= */ angle,
			/* rotation_axis			= */ vqf_state->gyr_no_bias,
			/* destination_quaternion	= */ vqf_state->gyr_step_quat
		);

		vqf_linear_algebra->multiply_quaternion(
			/* left_quaternion			= */ vqf_state->gyr_quat,
			/* right_quaternion			= */ vqf_state->gyr_step_quat,
			/* destination_quaternion	= */ vqf_state->gyr_quat
		);

		vqf_linear_algebra->normalize_quaternion_in_place(vqf_state->gyr_quat);
	}
}

void vqf_update_acc(
	vqf_context_t*		vqf_context,
	vqf_matrix_handle_t	acc
) {
	// Cache the VQF context to stack.
	const	vqf_linear_algebra_t*	vqf_linear_algebra	= &vqf_context->linear_algebra;
	const	vqf_coefficients_t*		vqf_coeff			= &vqf_context->coefficients;
	const	vqf_params_t*			vqf_param			= &vqf_context->params;
			vqf_state_t*			vqf_state			= &vqf_context->state;

	// ignore [0 0 0] samples
	if (	vqf_linear_algebra->get_matrix_coefficient(acc, 0U, 0U) == 0.0f
		&&	vqf_linear_algebra->get_matrix_coefficient(acc, 1U, 0U) == 0.0f
		&&	vqf_linear_algebra->get_matrix_coefficient(acc, 2U, 0U) == 0.0f
	) {
		return;
	}

	// rest detection
	if (vqf_param->rest_bias_est_enabled) {
		vqf_filter_vec(
			/* vqf_context	= */ vqf_context,
			/* x			= */ acc,
			/* tau			= */ vqf_param->rest_filter_tau,
			/* Ts			= */ vqf_coeff->acc_ts,
			/* b			= */ vqf_coeff->rest_acc_lp_B,
			/* a			= */ vqf_coeff->rest_acc_lp_A,
			/* state		= */ vqf_state->rest_acc_lp_state,
			/* out			= */ vqf_state->rest_last_acc_lp
		);

		vqf_linear_algebra->subtract_matrix(
			/* left_matrix			= */ acc,
			/* right_matrix			= */ vqf_state->rest_last_acc_lp,
			/* destination_matrix	= */ vqf_state->acc_mean_deviation
		);

		vqf_state->rest_last_squared_deviations[1U] = vqf_linear_algebra->get_vector_squared_norm(vqf_state->acc_mean_deviation);

		if (vqf_state->rest_last_squared_deviations[1U] >= square(vqf_param->rest_th_acc)) {
			vqf_state->rest_t			= 0.0f;
			vqf_state->rest_detected	= false;
		} else {
			vqf_state->rest_t += vqf_coeff->acc_ts;

			if (vqf_state->rest_t >= vqf_param->rest_min_t) {
				vqf_state->rest_detected = true;
			}
		}
	}

	// filter acc in inertial frame
	vqf_linear_algebra->quaternion_rotate_vector(
		/* src_vector			= */ acc,
		/* quaternion			= */ vqf_state->gyr_quat,
		/* destination_vector	= */ vqf_state->acc_earth
	);

	vqf_filter_vec(
		/* vqf_context	= */ vqf_context,
		/* x			= */ vqf_state->acc_earth,
		/* tau			= */ vqf_param->tau_acc,
		/* Ts			= */ vqf_coeff->acc_ts,
		/* b			= */ vqf_coeff->acc_lp_B,
		/* a			= */ vqf_coeff->acc_lp_A,
		/* state		= */ vqf_state->acc_lp_state,
		/* out			= */ vqf_state->last_acc_lp
	);

	// transform to 6D earth frame and normalize
	vqf_linear_algebra->quaternion_rotate_vector(
		/* src_vector			= */ vqf_state->last_acc_lp,
		/* quaternion			= */ vqf_state->acc_quat,
		/* destination_vector	= */ vqf_state->acc_earth
	);
	vqf_linear_algebra->normalize_matrix_in_place(vqf_state->acc_earth);

	const float_t acc_earth_2 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->acc_earth, 2U, 0U);

	// inclination correction
	const float_t q_w = sqrt((acc_earth_2 + 1) / 2);

	if (q_w > 1e-6f) {
		const float acc_earth_0 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->acc_earth, 0U, 0U);
		const float acc_earth_1 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->acc_earth, 1U, 0U);

		vqf_linear_algebra->set_quaternion_w(vqf_state->acc_corr_quat,		q_w);
		vqf_linear_algebra->set_quaternion_x(vqf_state->acc_corr_quat,		0.5f * acc_earth_1 / q_w);
		vqf_linear_algebra->set_quaternion_y(vqf_state->acc_corr_quat, -	0.5f * acc_earth_0 / q_w);
		vqf_linear_algebra->set_quaternion_z(vqf_state->acc_corr_quat,		0);
	} else {
        // to avoid numeric issues when acc is close to [0 0 -1], i.e. the correction step is close (<= 0.00011°) to 180°:
		vqf_linear_algebra->set_quaternion_w(vqf_state->acc_corr_quat, 0);
		vqf_linear_algebra->set_quaternion_x(vqf_state->acc_corr_quat, 1);
		vqf_linear_algebra->set_quaternion_y(vqf_state->acc_corr_quat, 0);
		vqf_linear_algebra->set_quaternion_z(vqf_state->acc_corr_quat, 0);
	}

	vqf_linear_algebra->multiply_quaternion(
		/* left_quaternion			= */ vqf_state->acc_corr_quat,
		/* right_quaternion			= */ vqf_state->acc_quat,
		/* destination_quaternion	= */ vqf_state->acc_quat
	);

	vqf_linear_algebra->normalize_quaternion_in_place(vqf_state->acc_quat);

	// calculate correction angular rate to facilitate debugging
	vqf_state->last_acc_corr_angular_rate = acos(acc_earth_2) / vqf_coeff->acc_ts;

	// bias estimation
#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	if (vqf_param->motion_bias_est_enabled || vqf_param->rest_bias_est_enabled) {
		const float_t bias_clip = vqf_param->bias_clip * (float_t) (PI / 180.0);

		// get rotation matrix corresponding to accGyrQuat
		vqf_get_quat_6D(vqf_context, vqf_state->acc_gyr_quat);

		vqf_linear_algebra->quaternion_to_rotation_matrix(
			/* src_quaternion		= */ vqf_state->acc_gyr_quat,
			/* destination_matrix	= */ vqf_state->motion_bias_est_R
		);

		// calculate R*b_hat (only the x and y component, as z is not needed)
		vqf_linear_algebra->multiply_matrix(
			/* left_matrix			= */ vqf_state->motion_bias_est_R,
			/* right_matrix			= */ vqf_state->bias,
			/* destination_matrix	= */ vqf_state->motion_bias_est_bias_lp
		);

		// low-pass filter R and R*b_hat
		vqf_filter_vec(
			/* vqf_context	= */ vqf_context,
			/* x			= */ vqf_state->motion_bias_est_R,
			/* tau			= */ vqf_param->tau_acc,
			/* Ts			= */ vqf_coeff->acc_ts,
			/* b			= */ vqf_coeff->acc_lp_B,
			/* a			= */ vqf_coeff->acc_lp_A,
			/* state		= */ vqf_state->motion_bias_est_R_lp_state,
			/* out			= */ vqf_state->motion_bias_est_R
		);

		vqf_filter_vec(
			/* vqf_context	= */ vqf_context,
			/* x			= */ vqf_state->motion_bias_est_bias_lp,
			/* tau			= */ vqf_param->tau_acc,
			/* Ts			= */ vqf_coeff->acc_ts,
			/* b			= */ vqf_coeff->acc_lp_B,
			/* a			= */ vqf_coeff->acc_lp_A,
			/* state		= */ vqf_state->motion_bias_est_bias_lp_state,
			/* out			= */ vqf_state->motion_bias_est_bias_lp
		);

		// set measurement error and covariance for the respective Kalman filter update
		if (vqf_state->rest_detected && vqf_param->rest_bias_est_enabled) {
			vqf_linear_algebra->copy_matrix(
				/* source_matrix		= */ vqf_state->rest_last_gyr_lp,
				/* destination_matrix	= */ vqf_state->motion_bias_est_e
			);

			vqf_linear_algebra->subtract_matrix(
				/* left_matrix			= */ vqf_state->motion_bias_est_e,
				/* right_matrix			= */ vqf_state->bias,
				/* destination_matrix	= */ vqf_state->motion_bias_est_e
			);

			vqf_linear_algebra->set_matrix_scaled_identity_in_place(vqf_state->motion_bias_est_R, 1);
			vqf_linear_algebra->set_matrix_scaled_identity_in_place(vqf_state->motion_bias_est_W, vqf_coeff->bias_rest_W);
		} else if (vqf_param->motion_bias_est_enabled) {
			vqf_linear_algebra->multiply_matrix(
				/* left_matrix			= */ vqf_state->motion_bias_est_R,
				/* right_matrix			= */ vqf_state->bias,
				/* destination_matrix	= */ vqf_state->motion_bias_est_e
			);

			vqf_linear_algebra->multiply_matrix_scalar_in_place(
				/* source_matrix	= */ vqf_state->motion_bias_est_e,
				/* value			= */ -1
			);

			// Get the error of the earth frame acceleration by cross-product.
			vqf_linear_algebra->multiply_matrix(
				/*left_matrix			= */ vqf_coeff->motion_bias_est_skew_ez,
				/*right_matrix			= */ vqf_state->acc_earth,
				/*destination_matrix	= */ vqf_state->acc_earth
			);

			vqf_linear_algebra->multiply_matrix_scalar_in_place(
				/* source_matrix	= */		vqf_state->acc_earth,
				/* value			= */ 1 /	vqf_coeff->acc_ts
			);

			// Add it to the measurement error.
			vqf_linear_algebra->add_matrix(
				/* left_matrix			= */ vqf_state->acc_earth,
				/* right_matrix			= */ vqf_state->motion_bias_est_e,
				/* destination_matrix	= */ vqf_state->motion_bias_est_e
			);

			// We don't need the Z in motion bias estimation.
			vqf_linear_algebra->set_matrix_coefficient(
				/* matrix	= */ vqf_state->motion_bias_est_bias_lp,
				/* row		= */ 2U,
				/* column	= */ 0U,
				/* value	= */ 0
			);

			// Add the XY of the low-pass filtered bias to measurement error.
			vqf_linear_algebra->add_matrix(
				/* left_matrix			= */ vqf_state->motion_bias_est_bias_lp,
				/* right_matrix			= */ vqf_state->motion_bias_est_e,
				/* destination_matrix	= */ vqf_state->motion_bias_est_e
			);

			// Set the diagonal of the W.
			vqf_linear_algebra->set_matrix_coefficient(vqf_state->motion_bias_est_W, 0U, 0U, vqf_coeff->bias_motion_W);
			vqf_linear_algebra->set_matrix_coefficient(vqf_state->motion_bias_est_W, 1U, 1U, vqf_coeff->bias_motion_W);
			vqf_linear_algebra->set_matrix_coefficient(vqf_state->motion_bias_est_W, 2U, 2U, vqf_coeff->bias_vertical_W);
		} else {
			// NEVER set non-digonal coefficients of the W.
			vqf_linear_algebra->set_matrix_scaled_identity_in_place(
				/* source_matrix	= */ vqf_state->motion_bias_est_W,
				/* scale			= */ -1 // disable update
			);
		}

		// Kalman filter update
		// step 1: P = P + V (also increase covariance if there is no measurement update!)

		if (vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 0U, 0U) < vqf_coeff->bias_P0) {
			vqf_linear_algebra->add_matrix_coefficient(vqf_state->bias_P, 0U, 0U, vqf_coeff->bias_V);
		}

		if (vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 1U, 1U) < vqf_coeff->bias_P0) {
			vqf_linear_algebra->add_matrix_coefficient(vqf_state->bias_P, 1U, 1U, vqf_coeff->bias_V);
		}

		if (vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 2U, 2U) < vqf_coeff->bias_P0) {
			vqf_linear_algebra->add_matrix_coefficient(vqf_state->bias_P, 2U, 2U, vqf_coeff->bias_V);
		}

		if (vqf_linear_algebra->get_matrix_coefficient(vqf_state->motion_bias_est_W, 0U, 0U) >= 0) {
			// clip disagreement to -2..2 °/s
			// (this also effectively limits the harm done by the first inclination correction step)
			vqf_linear_algebra->clip_vector_in_place(
				/* source_vector	= */	vqf_state->motion_bias_est_e,
				/* min_value		= */ -	bias_clip,
				/* max_value		= */	bias_clip
			);

			// Transpose the R.
			vqf_linear_algebra->copy_matrix(
				/* source_matrix		= */ vqf_state->motion_bias_est_R,
				/* destination_matrix	= */ vqf_state->motion_bias_est_Rt
			);

			vqf_linear_algebra->transpose_matrix_in_place(vqf_state->motion_bias_est_Rt);

			// step 2: K = P R^T inv(W + R P R^T)

			// K = P R^T
			vqf_linear_algebra->multiply_matrix(
				/* left_matrix			= */ vqf_state->bias_P,
				/* right_matrix			= */ vqf_state->motion_bias_est_Rt,
				/* destination_matrix	= */ vqf_state->motion_bias_est_K
			);

			// K = R P R^T
			vqf_linear_algebra->multiply_matrix(
				/* left_matrix			= */ vqf_state->motion_bias_est_R,
				/* right_matrix			= */ vqf_state->motion_bias_est_K,
				/* destination_matrix	= */ vqf_state->motion_bias_est_K
			);

			// K = W + R P R^T
			vqf_linear_algebra->add_matrix(
				/* left_matrix			= */ vqf_state->motion_bias_est_W,
				/* right_matrix			= */ vqf_state->motion_bias_est_K,
				/* destination_matrix	= */ vqf_state->motion_bias_est_K
			);

			// K = inv(W + R P R^T)
			vqf_linear_algebra->invert_matrix_in_place(vqf_state->motion_bias_est_K);

			// K = R^T inv(W + R P R^T)
			vqf_linear_algebra->multiply_matrix(
				/* left_matrix			= */ vqf_state->motion_bias_est_Rt,
				/* right_matrix			= */ vqf_state->motion_bias_est_K,
				/* destination_matrix	= */ vqf_state->motion_bias_est_K
			);

			// K = P R^T inv(W + R P R^T)
			vqf_linear_algebra->multiply_matrix(
				/* left_matrix			= */ vqf_state->bias_P,
				/* right_matrix			= */ vqf_state->motion_bias_est_K,
				/* destination_matrix	= */ vqf_state->motion_bias_est_K
			);

			// step 3: bias = bias + K (y - R bias) = bias + K e

			vqf_linear_algebra->multiply_matrix(
				/* left_matrix			= */ vqf_state->motion_bias_est_K,
				/* right_matrix			= */ vqf_state->motion_bias_est_e,
				/* destination_matrix	= */ vqf_state->motion_bias_est_corr
			);

			vqf_linear_algebra->add_matrix(
				/* left_matrix			= */ vqf_state->bias,
				/* right_matrix			= */ vqf_state->motion_bias_est_corr,
				/* destination_matrix	= */ vqf_state->bias
			);

			// step 4: P = P - K R P

			// K = K R
			vqf_linear_algebra->multiply_matrix(
				/* left_matrix			= */ vqf_state->motion_bias_est_K,
				/* right_matrix			= */ vqf_state->motion_bias_est_R,
				/* destination_matrix	= */ vqf_state->motion_bias_est_K
			);

			// K = K R P
			vqf_linear_algebra->multiply_matrix(
				/* left_matrix			= */ vqf_state->motion_bias_est_K,
				/* right_matrix			= */ vqf_state->bias_P,
				/* destination_matrix	= */ vqf_state->motion_bias_est_K
			);

			vqf_linear_algebra->subtract_matrix(
				/* left_matrix			= */ vqf_state->bias_P,
				/* right_matrix			= */ vqf_state->motion_bias_est_K,
				/* destination_matrix	= */ vqf_state->bias_P
			);

            // clip bias estimate to -2..2 °/s
			vqf_linear_algebra->clip_vector_in_place(
				/* source_vector	= */ vqf_state->bias,
				/* min_value		= */ -	bias_clip,
				/* max_value		= */	bias_clip
			);
		}
	}
#else
	// simplified implementation of bias estimation for the special case in which only rest bias estimation is enabled
	if (vqf_param->rest_bias_est_enabled) {

		float_t bias_clip = vqf_param->bias_clip * (float_t) (PI / 180.0);

		if (vqf_state->bias_P < vqf_coeff->bias_P0) {
			vqf_state->bias_P += vqf_coeff->bias_V;
		}

		if (vqf_state->rest_detected) {
			vqf_linear_algebra->copy_matrix(
				/* source_matrix		= */ vqf_state->rest_last_gyr_lp,
				/* destination_matrix	= */ vqf_state->motion_bias_est_e
			);

			vqf_linear_algebra->subtract_matrix(
				/* left_matrix			= */ vqf_state->motion_bias_est_e,
				/* right_matrix			= */ vqf_state->bias,
				/* destination_matrix	= */ vqf_state->motion_bias_est_e
			);

			vqf_linear_algebra->clip_vector_in_place(
				/* source_vector	= */ vqf_state->motion_bias_est_e,
				/* min_value		= */ -	bias_clip,
				/* max_value		= */	bias_clip
			);

			// Kalman filter update, simplified scalar version for rest update
			// (this version only uses the first entry of P as P is diagonal and all diagonal elements are the same)
			// step 1: P = P + V (done above!)
			// step 2: K = P R^T inv(W + R P R^T)

			const float_t k = vqf_state->bias_P / (vqf_coeff->bias_rest_W + vqf_state->bias_P);

			// step 3: bias = bias + K (y - R bias) = bias + K e

			vqf_linear_algebra->multiply_matrix_scalar_in_place(
				/* source_matrix	= */ vqf_state->motion_bias_est_e,
				/* value			= */ k
			);

			vqf_linear_algebra->add_matrix(
				/* left_matrix			= */ vqf_state->motion_bias_est_e,
				/* right_matrix			= */ vqf_state->bias,
				/* destination_matrix	= */ vqf_state->bias
			);

			// step 4: P = P - K R P
			vqf_state->bias_P -= k * vqf_state->bias_P;

			// clip bias estimate to -2..2 °/s
			vqf_linear_algebra->clip_vector_in_place(
				/* source_vector	= */ vqf_state->bias,
				/* min_value		= */ -	bias_clip,
				/* max_value		= */	bias_clip
			);
		}
	}
#endif
}

void vqf_update_mag(
	vqf_context_t*		vqf_context,
	vqf_matrix_handle_t	mag
) {
	// Cache the VQF context to stack.
	const	vqf_linear_algebra_t*	vqf_linear_algebra	= &vqf_context->linear_algebra;
	const	vqf_coefficients_t*		vqf_coeff			= &vqf_context->coefficients;
	const	vqf_params_t*			vqf_param			= &vqf_context->params;
			vqf_state_t*			vqf_state			= &vqf_context->state;

	// ignore [0 0 0] samples
	if (	vqf_linear_algebra->get_matrix_coefficient(mag, 0U, 0U) == 0.0f
		&&	vqf_linear_algebra->get_matrix_coefficient(mag, 1U, 0U) == 0.0f
		&&	vqf_linear_algebra->get_matrix_coefficient(mag, 2U, 0U) == 0.0f
	) {
		return;
	}

	// bring magnetometer measurement into 6D earth frame
	vqf_get_quat_6D(vqf_context, vqf_state->acc_gyr_quat);

	vqf_linear_algebra->quaternion_rotate_vector(
		/* src_vector			= */ mag,
		/* quaternion			= */ vqf_state->acc_gyr_quat,
		/* destination_vector	= */ vqf_state->mag_earth
	);

	const float_t mag_earth_0 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->mag_earth, 0U, 0U);
	const float_t mag_earth_1 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->mag_earth, 1U, 0U);
	const float_t mag_earth_2 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->mag_earth, 2U, 0U);

	if (vqf_param->mag_dist_rejection_enabled) {
		const float_t norm = vqf_linear_algebra->get_vector_norm(vqf_state->mag_earth);

		vqf_linear_algebra->set_matrix_coefficient(vqf_state->mag_nrm_dip, 0U, 0U,						norm);
		vqf_linear_algebra->set_matrix_coefficient(vqf_state->mag_nrm_dip, 1U, 0U, -asin(mag_earth_2 /	norm));

		if (vqf_param->mag_current_tau > 0) {
			vqf_filter_vec(
				/* vqf_context	= */ vqf_context,
				/* x			= */ vqf_state->mag_nrm_dip,
				/* tau			= */ vqf_param->mag_current_tau,
				/* Ts			= */ vqf_coeff->mag_ts,
				/* b			= */ vqf_coeff->mag_nrm_dip_lp_B,
				/* a			= */ vqf_coeff->mag_nrm_dip_lp_A,
				/* state		= */ vqf_state->mag_nrm_dip_lp_state,
				/* out			= */ vqf_state->mag_nrm_dip
			);
		}

		const float_t mag_nrm_lp = vqf_linear_algebra->get_matrix_coefficient(vqf_state->mag_nrm_dip, 0U, 0U);
		const float_t mag_dip_lp = vqf_linear_algebra->get_matrix_coefficient(vqf_state->mag_nrm_dip, 1U, 0U);

		// magnetic disturbance detection
		if (	fabs(mag_nrm_lp - vqf_state->mag_ref_nrm)	< vqf_param->mag_nrm_th * vqf_state->mag_ref_nrm
			&&	fabs(mag_dip_lp - vqf_state->mag_ref_dip)	< vqf_param->mag_dip_th * (float_t) (PI / 180.0)
		) {
			vqf_state->mag_undisturbed_t += vqf_coeff->mag_ts;

			if (vqf_state->mag_undisturbed_t >= vqf_param->mag_min_undisturbed_time) {
				vqf_state->mag_dist_detected = false;

				vqf_state->mag_ref_nrm += vqf_coeff->k_mag_ref * (mag_nrm_lp - vqf_state->mag_ref_nrm);
				vqf_state->mag_ref_dip += vqf_coeff->k_mag_ref * (mag_dip_lp - vqf_state->mag_ref_dip);
            }
        } else {
            vqf_state->mag_undisturbed_t = 0.0f;
        	vqf_state->mag_dist_detected = true;
        }

		// new magnetic field acceptance
		if (	fabs(mag_nrm_lp - vqf_state->mag_candidate_nrm)	< vqf_param->mag_nrm_th * vqf_state->mag_candidate_nrm
			&&	fabs(mag_dip_lp - vqf_state->mag_candidate_dip) < vqf_param->mag_dip_th * (float_t) (PI / 180.0)
		) {
			if (vqf_linear_algebra->get_vector_norm(vqf_state->rest_last_gyr_lp) >= vqf_param->mag_new_min_gyr * (float_t) (PI / 180.0)) {
				vqf_state->mag_candidate_t += vqf_coeff->mag_ts;
			}

			vqf_state->mag_candidate_nrm += vqf_coeff->k_mag_ref * (mag_nrm_lp - vqf_state->mag_candidate_nrm);
			vqf_state->mag_candidate_dip += vqf_coeff->k_mag_ref * (mag_dip_lp - vqf_state->mag_candidate_dip);

			if (vqf_state->mag_dist_detected && (vqf_state->mag_candidate_t >= vqf_param->mag_new_time || (vqf_state->mag_ref_nrm == 0.0f && vqf_state->mag_candidate_t >= vqf_param->mag_new_first_time))) {
				vqf_state->mag_ref_nrm			= vqf_state	->mag_candidate_nrm;
				vqf_state->mag_ref_dip			= vqf_state	->mag_candidate_dip;
				vqf_state->mag_undisturbed_t	= vqf_param->mag_min_undisturbed_time;
				vqf_state->mag_dist_detected	= false;
			}
		} else {
			vqf_state->mag_candidate_t		= 0.0f;
			vqf_state->mag_candidate_nrm	= mag_nrm_lp;
			vqf_state->mag_candidate_dip	= mag_dip_lp;
		}
	}

	// calculate disagreement angle based on current magnetometer measurement
	vqf_state->last_mag_dis_angle = atan2(mag_earth_0, mag_earth_1) - vqf_state->delta;

	// make sure the disagreement angle is in the range [-pi, pi]
	if (vqf_state->last_mag_dis_angle > (float_t) PI) {
		vqf_state->last_mag_dis_angle -= (float_t) (2 * PI);
	} else if (vqf_state->last_mag_dis_angle < (float_t) (-PI)) {
		vqf_state->last_mag_dis_angle += (float_t) (2 * PI);
	}

	float_t k = vqf_coeff->k_mag;

	if (vqf_param->mag_dist_rejection_enabled) {
		// magnetic disturbance rejection
		if (vqf_state->mag_dist_detected) {
			if (vqf_state->mag_reject_t <= vqf_param->mag_max_rejection_time) {
				vqf_state->mag_reject_t += vqf_coeff->mag_ts;
				k = 0;
			} else {
				k /= vqf_param->mag_rejection_factor;
			}
		} else {
			vqf_state->mag_reject_t = fmax(vqf_state->mag_reject_t - vqf_param->mag_rejection_factor * vqf_coeff->mag_ts, 0.0f);
		}
	}

	// ensure fast initial convergence
	if (vqf_state->k_mag_init != 0.0f) {
		// make sure that the gain k is at least 1/N, N=1,2,3,... in the first few samples
		if (k < vqf_state->k_mag_init) {
			k = vqf_state->k_mag_init;
		}

		// iterative expression to calculate 1/N
		vqf_state->k_mag_init = vqf_state->k_mag_init / (vqf_state->k_mag_init + 1);

		// disable if t > tauMag
		if (vqf_state->k_mag_init * vqf_param->tau_mag < vqf_coeff->mag_ts) {
			vqf_state->k_mag_init = 0.0;
		}
	}

	// first-order filter step
	vqf_state->delta += k * vqf_state->last_mag_dis_angle;

	// calculate correction angular rate to facilitate debugging
	vqf_state->last_mag_corr_angular_rate = k * vqf_state->last_mag_dis_angle / vqf_coeff->mag_ts;

	// make sure delta is in the range [-pi, pi]
	if (vqf_state->delta > (float_t) PI) {
		vqf_state->delta -= (float_t) (2 * PI);
	} else if (vqf_state->delta < (float_t) (-PI)) {
		vqf_state->delta += (float_t) (2 * PI);
	}
}

void vqf_update_6D(
	vqf_context_t*		vqf_context,
	vqf_matrix_handle_t	gyr,
	vqf_matrix_handle_t	acc
) {
	vqf_update_gyr(vqf_context, gyr);
	vqf_update_acc(vqf_context, acc);
}

void vqf_update_9D(
	vqf_context_t*		vqf_context,
	vqf_matrix_handle_t	gyr,
	vqf_matrix_handle_t	acc,
	vqf_matrix_handle_t	mag
) {
	vqf_update_gyr(vqf_context, gyr);
	vqf_update_acc(vqf_context, acc);
	vqf_update_mag(vqf_context, mag);
}

void vqf_get_quat_3D(
	const	vqf_context_t*			vqf_context,
			vqf_quaternion_handle_t	out
) {
	vqf_context->linear_algebra.copy_quaternion(
		/* source_quaternion		= */ vqf_context->state.gyr_quat,
		/* destination_quaternion	= */ out
	);
}

void vqf_get_quat_6D(
	const	vqf_context_t*			vqf_context,
			vqf_quaternion_handle_t	out
) {
	vqf_context->linear_algebra.multiply_quaternion(
		/* left_quaternion			= */ vqf_context->state.acc_quat,
		/* right_quaternion			= */ vqf_context->state.gyr_quat,
		/* destination_quaternion	= */ out
	);
}

void vqf_get_quat_9D(
	const	vqf_context_t*			vqf_context,
			vqf_quaternion_handle_t	out
) {
	vqf_context->linear_algebra.multiply_quaternion(
		/* left_quaternion			= */ vqf_context->state.acc_quat,
		/* right_quaternion			= */ vqf_context->state.gyr_quat,
		/* destination_quaternion	= */ out
	);
	vqf_context->linear_algebra.rotate_quaternion_around_z(
		/* rotation_z_radians		= */ vqf_context->state.delta,
		/* src_quaternion			= */ out,
		/* destination_quaternion	= */ out
	);
}

float_t vqf_getDelta(const vqf_context_t* vqf_context) {
	return vqf_context->state.delta;
}

float_t vqf_get_bias_estimate(
	const	vqf_context_t*		vqf_context,
			vqf_matrix_handle_t	out
) {
	// Cache the VQF context to stack.
	const vqf_linear_algebra_t*	vqf_linear_algebra	= &vqf_context->linear_algebra;
	const vqf_coefficients_t*	vqf_coeff			= &vqf_context->coefficients;
	const vqf_state_t*			vqf_state			= &vqf_context->state;

	if (out) {
		vqf_linear_algebra->copy_matrix(
			/* source_matrix		= */ vqf_state->bias,
			/* destination_matrix	= */ out
		);
	}
#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	float_t bias_P_00 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 0U, 0U);
	float_t bias_P_01 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 0U, 1U);
	float_t bias_P_02 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 0U, 2U);

	float_t bias_P_10 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 1U, 0U);
	float_t bias_P_11 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 1U, 1U);
	float_t bias_P_12 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 1U, 2U);

	float_t bias_P_20 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 2U, 0U);
	float_t bias_P_21 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 2U, 1U);
	float_t bias_P_22 = vqf_linear_algebra->get_matrix_coefficient(vqf_state->bias_P, 2U, 2U);

	// use largest absolute row sum as upper bound estimate for largest eigenvalue (Gershgorin circle theorem)
	// and clip output to biasSigmaInit
	float_t sum1 = fabs(bias_P_00) + fabs(bias_P_01) + fabs(bias_P_02);
	float_t sum2 = fabs(bias_P_10) + fabs(bias_P_11) + fabs(bias_P_12);
	float_t sum3 = fabs(bias_P_20) + fabs(bias_P_21) + fabs(bias_P_22);

	float_t P = fmin(fmax(fmax(sum1, sum2), sum3), vqf_coeff->bias_P0);
#else
	float_t P = vqf_state->bias_P;
#endif
	// convert standard deviation from 0.01deg to rad
	return sqrt(P) * (float_t) (PI / 100.0 / 180.0);
}

void vqf_set_bias_estimate(
	const	vqf_context_t*		vqf_context,
			vqf_matrix_handle_t	bias,
	const	float_t				sigma
) {
	// Cache the VQF context to stack.
	const vqf_linear_algebra_t*	vqf_linear_algebra	= &vqf_context->linear_algebra;
	const vqf_state_t*			vqf_state			= &vqf_context->state;

	vqf_linear_algebra->copy_matrix(
		/* source_matrix		= */ bias,
		/* destination_matrix	= */ vqf_state->bias
	);

	if (sigma > 0) {
		float_t P = square(sigma * (float_t) (180.0 * 100.0 / PI));
#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
		vqf_linear_algebra->set_matrix_scaled_identity_in_place(
			/* source_matrix	= */ vqf_state->bias_P,
			/* scale			= */ P
		);
#else
		vqf_state->bias_P = P;
#endif
	}
}

uint8_t vqf_get_rest_detected(const vqf_context_t* vqf_context) {
	return vqf_context->state.rest_detected;
}

uint8_t vqf_get_mag_dist_detected(const vqf_context_t* vqf_context) {
	return vqf_context->state.mag_dist_detected;
}

void vqf_get_relative_rest_deviations(
	const	vqf_context_t*	vqf_context,
			float_t			out[2U]
) {
	out[0U] = sqrt(vqf_context->state.rest_last_squared_deviations[0U]) / (vqf_context->params.rest_th_gyr * (float_t) (PI / 180.0));
	out[1U] = sqrt(vqf_context->state.rest_last_squared_deviations[1U]) / (vqf_context->params.rest_th_acc);
}

float_t vqf_get_mag_ref_norm(const vqf_context_t* vqf_context) {
	return vqf_context->state.mag_ref_nrm;
}

float_t vqf_get_mag_ref_dip(const vqf_context_t* vqf_context) {
	return vqf_context->state.mag_ref_dip;
}

void vqf_set_mag_ref(
			vqf_context_t*	vqf_context,
	const	float_t			norm,
	const	float_t			dip
) {
	vqf_context->state.mag_ref_nrm = norm;
	vqf_context->state.mag_ref_dip = dip;
}

void vqf_set_tau_acc(
			vqf_context_t*	vqf_context,
	const	float_t			tau_acc
) {
	// Cache the VQF context to stack.
	const	vqf_linear_algebra_t*	vqf_linear_algebra	= &vqf_context->linear_algebra;
			vqf_coefficients_t*		vqf_coeff			= &vqf_context->coefficients;
			vqf_params_t*			vqf_param			= &vqf_context->params;
			vqf_state_t*			vqf_state			= &vqf_context->state;

	if (vqf_param->tau_acc == tau_acc) {
		return;
	}

	vqf_param->tau_acc = tau_acc;

	double_t newB[3U];
	double_t newA[2U];

	vqf_filter_coefficients(
		/* tau	= */ tau_acc,
		/* Ts	= */ vqf_coeff->acc_ts,
		/* outB = */ newB,
		/* outA = */ newA
	);

	vqf_filter_adapt_state_for_coeff_change(
		/* vqf_context	= */ vqf_context,
		/* last_y		= */ vqf_state->last_acc_lp,
		/* b_old		= */ vqf_coeff->acc_lp_B,
		/* a_old		= */ vqf_coeff->acc_lp_A,
		/* b_new		= */ newB,
		/* a_new		= */ newA,
		/* state		= */ vqf_state->acc_lp_state
	);

#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	// For R and biasLP, the last value is not saved in the state.
	// // Since b0 is small (at reasonable settings), the last output is close to state[0].
	vqf_linear_algebra->copy_matrix_double_to_matrix(
		/* source_matrix		= */ vqf_state->motion_bias_est_R_lp_state[0],
		/* destination_matrix	= */ vqf_state->motion_bias_est_R
	);

	vqf_filter_adapt_state_for_coeff_change(
		/* vqf_context	= */ vqf_context,
		/* last_y		= */ vqf_state->motion_bias_est_R,
		/* b_old		= */ vqf_coeff->acc_lp_B,
		/* a_old		= */ vqf_coeff->acc_lp_A,
		/* b_new		= */ newB,
		/* a_new		= */ newA,
		/* state		= */ vqf_state->motion_bias_est_R_lp_state
	);


	vqf_linear_algebra->copy_matrix_double_to_matrix(
		/* source_matrix		= */ vqf_state->motion_bias_est_bias_lp_state[0],
		/* destination_matrix	= */ vqf_state->motion_bias_est_bias_lp
	);

	vqf_filter_adapt_state_for_coeff_change(
		/* vqf_context	= */ vqf_context,
		/* last_y		= */ vqf_state->motion_bias_est_bias_lp,
		/* b_old		= */ vqf_coeff->acc_lp_B,
		/* a_old		= */ vqf_coeff->acc_lp_A,
		/* b_new		= */ newB,
		/* a_new		= */ newA,
		/* state		= */ vqf_state->motion_bias_est_bias_lp_state
	);
#endif

	vqf_coeff->acc_lp_B[0U] = newB[0U];
	vqf_coeff->acc_lp_B[1U] = newB[1U];
	vqf_coeff->acc_lp_B[2U] = newB[2U];

	vqf_coeff->acc_lp_A[0U] = newA[0U];
	vqf_coeff->acc_lp_A[1U] = newA[1U];
}

#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
void vqf_set_motion_bias_est_enabled(
			vqf_context_t*	vqf_context,
	const	uint8_t			enabled
) {
	// Cache the VQF context to stack.
	const	vqf_linear_algebra_t*	vqf_linear_algebra	= &vqf_context->linear_algebra;
	const	vqf_state_t*			vqf_state			= &vqf_context->state;
			vqf_params_t*			vqf_param			= &vqf_context->params;

	if (vqf_param->motion_bias_est_enabled == enabled) {
		return;
	}

	vqf_param->motion_bias_est_enabled = enabled;

	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->motion_bias_est_R_lp_state[0U], NAN);
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->motion_bias_est_R_lp_state[1U], NAN);

	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->motion_bias_est_bias_lp_state[0U], NAN);
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->motion_bias_est_bias_lp_state[1U], NAN);
}
#endif

void vqf_set_rest_bias_est_enabled(
			vqf_context_t*	vqf_context,
	const	uint8_t			enabled
) {
	// Cache the VQF context to stack.
	const	vqf_linear_algebra_t*	vqf_linear_algebra	= &vqf_context->linear_algebra;
			vqf_params_t*			vqf_param			= &vqf_context->params;
			vqf_state_t*			vqf_state			= &vqf_context->state;

	if (vqf_param->rest_bias_est_enabled == enabled) {
		return;
	}

	vqf_param->rest_bias_est_enabled	= enabled;
	vqf_state->rest_detected			= false;
	vqf_state->rest_t					= 0.0f;

	vqf_state->rest_last_squared_deviations[0U] = 0.0f;
	vqf_state->rest_last_squared_deviations[1U] = 0.0f;

	vqf_linear_algebra->set_matrix_zeros_in_place(vqf_state->rest_last_gyr_lp);
	vqf_linear_algebra->set_matrix_zeros_in_place(vqf_state->rest_last_acc_lp);

	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->rest_gyr_lp_state[0U], NAN);
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->rest_gyr_lp_state[1U], NAN);

	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->rest_acc_lp_state[0U], NAN);
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->rest_acc_lp_state[1U], NAN);
}

void vqf_set_mag_dist_rejection_enabled(
			vqf_context_t*	vqf_context,
	const	uint8_t			enabled
) {
	// Cache the VQF context to stack.
	const	vqf_linear_algebra_t*	vqf_linear_algebra	= &vqf_context->linear_algebra;
			vqf_params_t*			vqf_param			= &vqf_context->params;
			vqf_state_t*			vqf_state			= &vqf_context->state;

	if (vqf_param->mag_dist_rejection_enabled == enabled) {
		return;
	}

	vqf_param->mag_dist_rejection_enabled	= enabled;
	vqf_state->mag_dist_detected			= true;
	vqf_state->mag_ref_nrm					= 0.0f;
	vqf_state->mag_ref_dip					= 0.0f;
	vqf_state->mag_undisturbed_t			= 0.0f;
	vqf_state->mag_reject_t					= vqf_param->mag_max_rejection_time;
	vqf_state->mag_candidate_nrm			= -1.0f;
	vqf_state->mag_candidate_dip			= 0.0f;
	vqf_state->mag_candidate_t				= 0.0f;

	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->mag_nrm_dip_lp_state[0U], NAN);
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->mag_nrm_dip_lp_state[1U], NAN);
}

void vqf_set_tau_mag(
			vqf_context_t*	vqf_context,
	const	float_t			tau_mag
) {
	vqf_context->params			.tau_mag	= tau_mag;
	vqf_context->coefficients	.k_mag		= vqf_gain_from_tau(tau_mag, vqf_context->coefficients.mag_ts);
}

void vqf_set_rest_detection_thresholds(
			vqf_context_t*	vqf_context,
	const	float_t			th_gyr,
	const	float_t			th_acc
) {
	vqf_context->params.rest_th_gyr = th_gyr;
	vqf_context->params.rest_th_acc = th_acc;
}

void vqf_reset_state(vqf_context_t*	vqf_context) {
	// Cache the VQF context to stack.
	const	vqf_linear_algebra_t*	vqf_linear_algebra	= &vqf_context->linear_algebra;
	const	vqf_coefficients_t*		vqf_coeff			= &vqf_context->coefficients;
	const	vqf_params_t*			vqf_param			= &vqf_context->params;
			vqf_state_t*			vqf_state			= &vqf_context->state;

	vqf_linear_algebra->set_quaternion_identity_in_place(vqf_state->gyr_quat);
	vqf_linear_algebra->set_quaternion_identity_in_place(vqf_state->acc_quat);

	vqf_state->delta = 0.0f;

	vqf_state->rest_detected		= false;
	vqf_state->mag_dist_detected	= true;

	vqf_linear_algebra->set_matrix_zeros_in_place(vqf_state->last_acc_lp);

	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->acc_lp_state[0U], NAN);
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->acc_lp_state[1U], NAN);

	vqf_state->k_mag_init					= 1.0;
	vqf_state->last_mag_dis_angle			= 0.0;
	vqf_state->last_mag_corr_angular_rate	= 0.0;
	vqf_state->last_acc_corr_angular_rate	= 0.0f;

	vqf_linear_algebra->set_matrix_zeros_in_place(vqf_state->bias);

#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	vqf_linear_algebra->set_matrix_scaled_identity_in_place(
		/* source_matrix	= */ vqf_state->bias_P,
		/* scale			= */ vqf_coeff->bias_P0
	);
#else
	vqf_state->bias_P = vqf_coefficients->bias_P0;
#endif

#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->motion_bias_est_R_lp_state[0U], NAN);
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->motion_bias_est_R_lp_state[1U], NAN);

	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->motion_bias_est_bias_lp_state[0U], NAN);
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->motion_bias_est_bias_lp_state[1U], NAN);
#endif

	vqf_state->rest_last_squared_deviations[0U] = 0.0f;
	vqf_state->rest_last_squared_deviations[1U] = 0.0f;

	vqf_state->rest_t = 0.0f;

	vqf_linear_algebra->set_matrix_zeros_in_place(vqf_state->rest_last_gyr_lp);
	vqf_linear_algebra->set_matrix_zeros_in_place(vqf_state->rest_last_acc_lp);

	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->rest_gyr_lp_state[0U], NAN);
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->rest_gyr_lp_state[1U], NAN);

	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->rest_acc_lp_state[0U], NAN);
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->rest_acc_lp_state[1U], NAN);

	vqf_state->mag_ref_nrm			= 0.0f;
	vqf_state->mag_ref_dip			= 0.0f;
	vqf_state->mag_undisturbed_t	= 0.0f;
	vqf_state->mag_reject_t			= vqf_param->mag_max_rejection_time;
	vqf_state->mag_candidate_nrm	= -1.0f;
	vqf_state->mag_candidate_dip	= 0.0f;
	vqf_state->mag_candidate_t		= 0.0f;

	vqf_linear_algebra->set_matrix_zeros_in_place(vqf_state->mag_nrm_dip);

	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->mag_nrm_dip_lp_state[0U], NAN);
	vqf_linear_algebra->set_matrix_double_constants_in_place(vqf_state->mag_nrm_dip_lp_state[1U], NAN);
}

float_t vqf_gain_from_tau(
	const float_t tau,
	const float_t Ts
) {
	assert(Ts > 0);

	if (tau < 0.0f) {
		return 0; // k=0 for negative tau (disable update)
	}

	if (tau == 0.0f) {
		return 1; // k=1 for tau=0
	}

	return 1 - exp(-Ts / tau); // fc = 1/(2*pi*tau)
}

void vqf_filter_coefficients(
	const	float_t		tau,
	const	float_t		Ts,
			double_t	outB[3],
			double_t	outA[2]
) {
	assert(tau	> 0);
	assert(Ts	> 0);

	// disable filter and use direct passthrough when tau < Ts/2 to avoid instability
	// (this corresponds to fc exceeding 90% of the Nyquist frequency)
	if (tau < Ts / 2) {
		outB[0U] = 1;
		outB[1U] = 0;
		outB[2U] = 0;
		outA[0U] = 0;
		outA[1U] = 0;

		return;
	}

	// second order Butterworth filter based on https://stackoverflow.com/a/52764064
	const double_t fc	= (SQRT2 / (2.0 * PI)) / (double_t)(tau); // time constant of dampened, non-oscillating part of step response
	const double_t C	= tan(PI * fc * (double_t) (Ts));
	const double_t D	= C * C + SQRT2 * C + 1;
	const double_t b0	= C * C / D;

	outB[0] = b0;
	outB[1] = b0 * 2;
	outB[2] = b0;

	// a0 = 1.0
	outA[0] = 2 * (C * C - 1) / D; // a1
	outA[1] = (1 - SQRT2 * C + C * C) / D; // a2
}

void vqf_filter_adapt_state_for_coeff_change(
	const	vqf_context_t*				vqf_context,
			vqf_matrix_handle_t			last_y,
	const	double_t					b_old	[3U],
	const	double_t					a_old	[2U],
	const	double_t					b_new	[3U],
	const	double_t					a_new	[2U],
			vqf_matrix_double_handle_t	state	[4U]
) {
	// Get the linear algebra context from the VQF context.
	const vqf_linear_algebra_t* vqf_linear_algebra = &vqf_context->linear_algebra;

	if (isnan(vqf_linear_algebra->get_matrix_double_coefficient(state[0U], 0U, 0U))) {
		return;
	}

	const double_t coeff_1 = b_old[0U] - b_new[0U];
	const double_t coeff_2 = b_old[1U] - b_new[1U] - a_old[0U] + a_new[0U];

	// Copy the Y to the state[2].
	vqf_linear_algebra->copy_matrix_to_matrix_double(
		/* source_matrix		= */ last_y,
		/* destination_matrix	= */ state[2]
	);

	// Copy the Y in state[2] tp state[3].
	vqf_linear_algebra->copy_matrix_double(
		/* source_matrix		= */ state[2],
		/* destination_matrix	= */ state[3]
	);

	// Now
	// state[2] = Y.
	// state[3] = Y.

	vqf_linear_algebra->multiply_matrix_double_scalar_in_place(state[2], coeff_1);
	vqf_linear_algebra->multiply_matrix_double_scalar_in_place(state[3], coeff_2);

	// Now
	// state[2] = (b_old[0U] - b_new[0U])							* Y.
	// state[3] = (b_old[1U] - b_new[1U] - a_old[0U] + a_new[0U])	* Y.

	// Add state[2] and state[3] to state[0] and state[1] separately.
	vqf_linear_algebra->add_matrix_double(state[2U], state[0U], state[0U]);
	vqf_linear_algebra->add_matrix_double(state[3U], state[1U], state[1U]);

	// Now
	// state[0] = old_state[0] + (b_old[0U] - b_new[0U])							* Y.
	// state[1] = old_state[1] + (b_old[1U] - b_new[1U] - a_old[0U] + a_new[0U])	* Y.
}

void vqf_filter_vec(
	const	vqf_context_t*				vqf_context,
			vqf_matrix_handle_t			x,
			float_t						tau,
			float_t						Ts,
	const	double_t					b		[3U],
	const	double_t					a		[2U],
			vqf_matrix_double_handle_t	state	[4U],
			vqf_matrix_handle_t			out
) {
	// Get the linear algebra context from the VQF context.
	const vqf_linear_algebra_t* vqf_linear_algebra = &vqf_context->linear_algebra;

	// Disable the assertion because this is the only usage of get_matrix_rows.
	// assert(linear_algebra_context->get_matrix_rows(x) >= 2U);

	// The sentinel and the sum are now stored in (0, 0) and (1, 0) of state[0];
	// The accumulated values are now in state[1].
	// The state[2] and state[3] are the temporary double precision vector.

	// to avoid depending on a single sample, average the first samples (for duration tau)
	// and then use this average to calculate the filter initial state
	if (isnan(vqf_linear_algebra->get_matrix_double_coefficient(state[0U], 0U, 0U))) { // initialization phase
		if (isnan(vqf_linear_algebra->get_matrix_double_coefficient(state[0U], 1U, 0U))) { // first sample
			// state[1] is used to store the sample count
			vqf_linear_algebra->set_matrix_double_coefficient	(state[0U], 1U, 0U, 0);
			vqf_linear_algebra->set_matrix_double_zeros_in_place(state[1U]); // state[1] is used to store the sum
		}

		vqf_linear_algebra->add_matrix_double_coefficient(state[0], 1U, 0U, 1);

		vqf_linear_algebra->copy_matrix_to_matrix_double(
			/* source_matrix		= */ x,
			/* destination_matrix	= */ state[2U]
		);

		vqf_linear_algebra->add_matrix_double(
			/* left_matrix			= */ state[2U],
			/* right_matrix			= */ state[1U],
			/* destination_matrix	= */ state[1U]
		);

		vqf_linear_algebra->copy_matrix_double_to_matrix(
			/* source_matrix		= */ state[1U],
			/* destination_matrix	= */ out
		);

		// Get the sum.
		const double_t count = vqf_linear_algebra->get_matrix_double_coefficient(state[0], 1U, 0U);

		vqf_linear_algebra->multiply_matrix_scalar_in_place(
			/* source_matrix	= */ out,
			/* value			= */ (float_t) (1 / count)
		);

		if (((float_t) count) * Ts >= tau) {
			// Copy the out to state[0].
			vqf_linear_algebra->copy_matrix_to_matrix_double(
				/* source_matrix		= */ out,
				/* destination_matrix	= */ state[0U]
			);

			const double_t coeff_1 = (1		- b[0U]);
			const double_t coeff_2 = (b[2U]	- a[1U]);

			// Copy the out in state[0] to state[1].
			vqf_linear_algebra->copy_matrix_double(
				/* source_matrix		= */ state[0U],
				/* destination_matrix	= */ state[1U]
			);

			// Now both state[0] and state[1] is out, multiply the coefficients to the states.
			vqf_linear_algebra->multiply_matrix_double_scalar_in_place(state[0U], coeff_1);
			vqf_linear_algebra->multiply_matrix_double_scalar_in_place(state[1U], coeff_2);
		}

		return;
	}

	// Convert the X into double precision.
	vqf_linear_algebra->copy_matrix_to_matrix_double(
		/* source_matrix		= */ x,
		/* destination_matrix	= */ state[2U]
	);

	// Copy the X in state[2] to state[3].
	vqf_linear_algebra->copy_matrix_double(
		/* source_matrix		= */ state[2U],
		/* destination_matrix	= */ state[3U]
	);

	// Multiply the X with the b[0U].
	vqf_linear_algebra->multiply_matrix_double_scalar_in_place(
		/* source_matrix	= */ state	[3U],
		/* value			= */ b		[0U]
	);

	// Add the X with the state[0].
	vqf_linear_algebra->add_matrix_double(
		/* left_matrix			= */ state[3],
		/* right_matrix			= */ state[0],
		/* destination_matrix	= */ state[3]
	);

	// Return the result first, then update the states.
	vqf_linear_algebra->copy_matrix_double_to_matrix(
		/* source_matrix		= */ state[3],
		/* destination_matrix	= */ out
	);

	// Now
	// state[2] = X,
	// state[3] = Y.

	// Set state[0] to state[1].
	vqf_linear_algebra->copy_matrix_double(
		/* source_matrix		= */ state[1],
		/* destination_matrix	= */ state[0]
	);

	// Set state[1] to 0.
	vqf_linear_algebra->set_matrix_double_zeros_in_place(state[1U]);

	// Now
	// state[0] = state[1],
	// state[1] = 0.

	// Add b[1U] * X and b[2U] * X to state[0] and state[1] separately.
	vqf_linear_algebra->accumulate_matrix_double(b[1U], state[2], state[0]);
	vqf_linear_algebra->accumulate_matrix_double(b[2U], state[2], state[1]);

	// Now
	// state[0] = b[1U] * X + old_state[1],
	// state[1] = b[2U] * X.

	// Add A[0U] * Y and a[1U] * Y to state[0] and state[1] separately.
	vqf_linear_algebra->accumulate_matrix_double(- a[0U], state[3], state[0]);
	vqf_linear_algebra->accumulate_matrix_double(- a[1U], state[3], state[1]);

	// Now
	// state[0] = b[1U] * X + a[0U] * Y + old_state[1],
	// state[1] = b[2U] * X + a[1U] * Y.
}

vqf_context_t* vqf_context_new(
	const	vqf_linear_algebra_t*	linear_algebra,
	const	vqf_params_t*			params,
	const	float_t					gyr_ts,
	const	float_t					acc_ts,
	const	float_t					mag_ts
) {
	// Allocate the VQF context handle in heap.
	vqf_context_t* vqf_context = (vqf_context_t*) calloc(1U, sizeof(vqf_context_t));

	// Skip the initialization if the allocation failed.
	if (vqf_context == NULL) {
		return NULL;
	}

	vqf_coefficients_t*	vqf_coeff = &vqf_context->coefficients;
	vqf_state_t*		vqf_state = &vqf_context->state;

	// Allocate the gyroscope update quaternions and matrices.
	vqf_state->gyr_quat				= linear_algebra->new_quaternion();
	vqf_state->gyr_step_quat		= linear_algebra->new_quaternion();
	vqf_state->gyr_mean_deviation	= linear_algebra->new_matrix(3U, 1U);
	vqf_state->gyr_no_bias			= linear_algebra->new_matrix(3U, 1U);

	// Check the allocations of the gyroscope update quaternions and matrices.
	if (vqf_state->gyr_quat				== NULL) goto error;
	if (vqf_state->gyr_step_quat		== NULL) goto error;
	if (vqf_state->gyr_mean_deviation	== NULL) goto error;
	if (vqf_state->gyr_no_bias			== NULL) goto error;

	// Allocate the accelerometer/magnetometer update quaternions and matrices.
	vqf_state->acc_quat				= linear_algebra->new_quaternion();
	vqf_state->acc_corr_quat		= linear_algebra->new_quaternion();
	vqf_state->acc_gyr_quat			= linear_algebra->new_quaternion();
	vqf_state->acc_mean_deviation	= linear_algebra->new_matrix(3U, 1U);
	vqf_state->acc_earth			= linear_algebra->new_matrix(3U, 1U);
	vqf_state->last_acc_lp			= linear_algebra->new_matrix(3U, 1U);
	vqf_state->mag_earth			= linear_algebra->new_matrix(3U, 1U);

	// Check the allocations of the accelerometer/magnetometer update quaternions and matrices.
	if (vqf_state->acc_quat				== NULL) goto error;
	if (vqf_state->acc_corr_quat		== NULL) goto error;
	if (vqf_state->acc_gyr_quat			== NULL) goto error;
	if (vqf_state->acc_mean_deviation	== NULL) goto error;
	if (vqf_state->acc_earth			== NULL) goto error;
	if (vqf_state->last_acc_lp			== NULL) goto error;
	if (vqf_state->mag_earth			== NULL) goto error;

	// Allocate the low-pass filter states of the accelerometer.
	vqf_state->acc_lp_state[0U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->acc_lp_state[1U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->acc_lp_state[2U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->acc_lp_state[3U] = linear_algebra->new_matrix_double(3U, 1U);

	// Check the allocations of the low-pass filter states of the accelerometer.
	if (vqf_state->acc_lp_state[0U] == NULL) goto error;
	if (vqf_state->acc_lp_state[1U] == NULL) goto error;
	if (vqf_state->acc_lp_state[2U] == NULL) goto error;
	if (vqf_state->acc_lp_state[3U] == NULL) goto error;

	// Allocate the bias estimation matrices.
	vqf_state->bias					= linear_algebra->new_matrix(3U, 1U);
	vqf_state->motion_bias_est_e	= linear_algebra->new_matrix(3U, 1U);

	// Check the allocations of the bias estimation matrices.
	if (vqf_state->bias					== NULL) goto error;
	if (vqf_state->motion_bias_est_e	== NULL) goto error;

#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	// Allocate the low-pass filter states of the rotation matrix of the motion bias estimation.
	vqf_state->motion_bias_est_R_lp_state[0U] = linear_algebra->new_matrix_double(3U, 3U);
	vqf_state->motion_bias_est_R_lp_state[1U] = linear_algebra->new_matrix_double(3U, 3U);
	vqf_state->motion_bias_est_R_lp_state[2U] = linear_algebra->new_matrix_double(3U, 3U);
	vqf_state->motion_bias_est_R_lp_state[3U] = linear_algebra->new_matrix_double(3U, 3U);

	// Check the allocations of the low-pass filter states of the rotation matrix of the motion bias estimation.
	if (vqf_state->motion_bias_est_R_lp_state[0U] == NULL) goto error;
	if (vqf_state->motion_bias_est_R_lp_state[1U] == NULL) goto error;
	if (vqf_state->motion_bias_est_R_lp_state[2U] == NULL) goto error;
	if (vqf_state->motion_bias_est_R_lp_state[3U] == NULL) goto error;

	// Allocate the low-pass filter states of the bias of the motion bias estimation.
	vqf_state->motion_bias_est_bias_lp_state[0U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->motion_bias_est_bias_lp_state[1U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->motion_bias_est_bias_lp_state[2U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->motion_bias_est_bias_lp_state[3U] = linear_algebra->new_matrix_double(3U, 1U);

	// Check the allocations of the low-pass filter states of the bias of the motion bias estimation.
	if (vqf_state->motion_bias_est_bias_lp_state[0U] == NULL) goto error;
	if (vqf_state->motion_bias_est_bias_lp_state[1U] == NULL) goto error;
	if (vqf_state->motion_bias_est_bias_lp_state[2U] == NULL) goto error;
	if (vqf_state->motion_bias_est_bias_lp_state[3U] == NULL) goto error;

	// Allocate the motion bias estimation matrices.
	vqf_coeff->motion_bias_est_skew_ez	= linear_algebra->new_matrix(3U, 3U);
	vqf_state->bias_P					= linear_algebra->new_matrix(3U, 3U);
	vqf_state->motion_bias_est_R		= linear_algebra->new_matrix(3U, 3U);
	vqf_state->motion_bias_est_Rt		= linear_algebra->new_matrix(3U, 3U);
	vqf_state->motion_bias_est_bias_lp	= linear_algebra->new_matrix(3U, 1U);
	vqf_state->motion_bias_est_W		= linear_algebra->new_matrix(3U, 3U);
	vqf_state->motion_bias_est_K		= linear_algebra->new_matrix(3U, 3U);
	vqf_state->motion_bias_est_corr		= linear_algebra->new_matrix(3U, 1U);

	// Check the allocations of the motion bias estimation matrices.
	if (vqf_coeff->motion_bias_est_skew_ez	== NULL) goto error;
	if (vqf_state->bias_P					== NULL) goto error;
	if (vqf_state->motion_bias_est_R		== NULL) goto error;
	if (vqf_state->motion_bias_est_Rt		== NULL) goto error;
	if (vqf_state->motion_bias_est_bias_lp	== NULL) goto error;
	if (vqf_state->motion_bias_est_W		== NULL) goto error;
	if (vqf_state->motion_bias_est_K		== NULL) goto error;
	if (vqf_state->motion_bias_est_corr		== NULL) goto error;
#endif

	// Allocate the rest detection and magnetic disturbance detection matrices.
	vqf_state->rest_last_gyr_lp = linear_algebra->new_matrix(3U, 1U);
	vqf_state->rest_last_acc_lp = linear_algebra->new_matrix(3U, 1U);
	vqf_state->mag_nrm_dip		= linear_algebra->new_matrix(2U, 1U);

	// Check the allocations of the rest detection and magnetic disturbance detection matrices.
	if (vqf_state->rest_last_gyr_lp		== NULL) goto error;
	if (vqf_state->rest_last_acc_lp		== NULL) goto error;
	if (vqf_state->mag_nrm_dip			== NULL) goto error;

	// Allocate the low-pass filter states of the angular velocity of rest detection.
	vqf_state->rest_gyr_lp_state[0U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->rest_gyr_lp_state[1U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->rest_gyr_lp_state[2U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->rest_gyr_lp_state[3U] = linear_algebra->new_matrix_double(3U, 1U);

	// Check the allocations of the low-pass filter states of the angular velocity of rest detection.
	if (vqf_state->rest_gyr_lp_state[0U] == NULL) goto error;
	if (vqf_state->rest_gyr_lp_state[1U] == NULL) goto error;
	if (vqf_state->rest_gyr_lp_state[2U] == NULL) goto error;
	if (vqf_state->rest_gyr_lp_state[3U] == NULL) goto error;

	// Allocate the low-pass filter states of the acceleration of rest detection.
	vqf_state->rest_acc_lp_state[0U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->rest_acc_lp_state[1U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->rest_acc_lp_state[2U] = linear_algebra->new_matrix_double(3U, 1U);
	vqf_state->rest_acc_lp_state[3U] = linear_algebra->new_matrix_double(3U, 1U);

	// Check the allocations of the low-pass filter states of the acceleration of rest detection.
	if (vqf_state->rest_acc_lp_state[0U] == NULL) goto error;
	if (vqf_state->rest_acc_lp_state[1U] == NULL) goto error;
	if (vqf_state->rest_acc_lp_state[2U] == NULL) goto error;
	if (vqf_state->rest_acc_lp_state[3U] == NULL) goto error;

	// Allocate the low-pass filter states of the magnetic norm and dip angle of magnetic disturbance detection.
	vqf_state->mag_nrm_dip_lp_state[0U] = linear_algebra->new_matrix_double(2U, 1U);
	vqf_state->mag_nrm_dip_lp_state[1U] = linear_algebra->new_matrix_double(2U, 1U);
	vqf_state->mag_nrm_dip_lp_state[2U] = linear_algebra->new_matrix_double(2U, 1U);
	vqf_state->mag_nrm_dip_lp_state[3U] = linear_algebra->new_matrix_double(2U, 1U);

	// Check the allocations of the low-pass filter states of the magnetic norm and dip angle of magnetic disturbance detection.
	if (vqf_state->mag_nrm_dip_lp_state[0U] == NULL) goto error;
	if (vqf_state->mag_nrm_dip_lp_state[1U] == NULL) goto error;
	if (vqf_state->mag_nrm_dip_lp_state[2U] == NULL) goto error;
	if (vqf_state->mag_nrm_dip_lp_state[3U] == NULL) goto error;

	// Initialize the cross-product matrix.
	linear_algebra->set_matrix_coefficient(vqf_coeff->motion_bias_est_skew_ez, 0U, 1U, -1);
	linear_algebra->set_matrix_coefficient(vqf_coeff->motion_bias_est_skew_ez, 1U, 0U, 1);

	// The matrix is initialized with all coefficients 0.
	// After setting the (0, 1) and (1, 0) of the skew_ez, it becomes:
	// [
	//	+0, -1, +0,
	//	+1, +0, +0
	//	+0, +0, +0
	// ]



	// Fill the linear algebra functions, parameters, and sample times into the VQF context.
	vqf_context	->linear_algebra	= *	linear_algebra;
	vqf_context	->params			= *	params;
	vqf_coeff	->gyr_ts			=	gyr_ts;
	vqf_coeff	->acc_ts			=	acc_ts > 0 ? acc_ts : gyr_ts;
	vqf_coeff	->mag_ts			=	mag_ts > 0 ? mag_ts : gyr_ts;

	const vqf_params_t* vqf_param = &vqf_context->params;

	// Calculate the coefficients of the VQF context based on the sample times and parameters.

	vqf_filter_coefficients(
		/* tau	= */ vqf_param->tau_acc,
		/* Ts	= */ vqf_coeff->acc_ts,
		/* outB	= */ vqf_coeff->acc_lp_B,
		/* outA	= */ vqf_coeff->acc_lp_A
	);

	vqf_coeff->k_mag = vqf_gain_from_tau(
		/* tau	= */ vqf_param->tau_mag,
		/* Ts	= */ vqf_coeff->mag_ts
	);

	vqf_coeff->bias_P0 = square(vqf_param->bias_sigma_init * 100.0f);

	// the system noise increases the variance from 0 to (0.1 °/s)^2 in biasForgettingTime seconds
	vqf_coeff->bias_V = 0.1f * 100.0f * vqf_coeff->acc_ts / vqf_param->bias_forgetting_time;

#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	const float_t p_motion = square(vqf_param->bias_sigma_motion * 100.0f);

	vqf_coeff->bias_motion_W	= square(p_motion) / vqf_coeff->bias_V + p_motion;
	vqf_coeff->bias_vertical_W	= vqf_coeff->bias_motion_W / fmax(vqf_param->bias_vertical_forgetting_factor, 1e-10f);
#endif

	const float_t p_rest = square(vqf_param->bias_sigma_rest * 100.0f);

	vqf_coeff->bias_rest_W = square(p_rest) / vqf_coeff->bias_V + p_rest;

	vqf_filter_coefficients(
		/* tau	= */ vqf_param->rest_filter_tau,
		/* Ts	= */ vqf_coeff->gyr_ts,
		/* outB	= */ vqf_coeff->rest_gyr_lp_B,
		/* outA	= */ vqf_coeff->rest_gyr_lp_A
	);

	vqf_filter_coefficients(
		/* tau	= */ vqf_param->rest_filter_tau,
		/* Ts	= */ vqf_coeff->acc_ts,
		/* outB	= */ vqf_coeff->rest_acc_lp_B,
		/* outA	= */ vqf_coeff->rest_acc_lp_A
	);

	vqf_coeff->k_mag_ref = vqf_gain_from_tau(
		/* tau	= */ vqf_param->mag_ref_tau,
		/* Ts	= */ vqf_coeff->mag_ts
	);

	if (vqf_param->mag_current_tau > 0) {
		vqf_filter_coefficients(
			/* tau	= */ vqf_param->mag_current_tau,
			/* Ts	= */ vqf_coeff->mag_ts,
			/* outB	= */ vqf_coeff->mag_nrm_dip_lp_B,
			/* outA	= */ vqf_coeff->mag_nrm_dip_lp_A
		);
	} else {
		vqf_coeff->mag_nrm_dip_lp_B[0U] = NAN;
		vqf_coeff->mag_nrm_dip_lp_B[1U] = NAN;
		vqf_coeff->mag_nrm_dip_lp_B[2U] = NAN;

		vqf_coeff->mag_nrm_dip_lp_A[0U] = NAN;
		vqf_coeff->mag_nrm_dip_lp_A[1U] = NAN;
	}

	vqf_reset_state(vqf_context);

	return vqf_context;

	error:

	// Cleanup the low-pass filter states of the magnetic norm and dip angle of magnetic disturbance detection.
	if (vqf_state->mag_nrm_dip_lp_state[3U] != NULL) linear_algebra->delete_matrix_double(vqf_state->mag_nrm_dip_lp_state[3U]);
	if (vqf_state->mag_nrm_dip_lp_state[2U] != NULL) linear_algebra->delete_matrix_double(vqf_state->mag_nrm_dip_lp_state[2U]);
	if (vqf_state->mag_nrm_dip_lp_state[1U] != NULL) linear_algebra->delete_matrix_double(vqf_state->mag_nrm_dip_lp_state[1U]);
	if (vqf_state->mag_nrm_dip_lp_state[0U] != NULL) linear_algebra->delete_matrix_double(vqf_state->mag_nrm_dip_lp_state[0U]);

	// Cleanup the low-pass filter states of the acceleration of rest detection.
	if (vqf_state->rest_acc_lp_state[3U] != NULL) linear_algebra->delete_matrix_double(vqf_state->rest_acc_lp_state[3U]);
	if (vqf_state->rest_acc_lp_state[2U] != NULL) linear_algebra->delete_matrix_double(vqf_state->rest_acc_lp_state[2U]);
	if (vqf_state->rest_acc_lp_state[1U] != NULL) linear_algebra->delete_matrix_double(vqf_state->rest_acc_lp_state[1U]);
	if (vqf_state->rest_acc_lp_state[0U] != NULL) linear_algebra->delete_matrix_double(vqf_state->rest_acc_lp_state[0U]);

	// Cleanup the low-pass filter states of the angular velocity of rest detection.
	if (vqf_state->rest_gyr_lp_state[3U] != NULL) linear_algebra->delete_matrix_double(vqf_state->rest_gyr_lp_state[3U]);
	if (vqf_state->rest_gyr_lp_state[2U] != NULL) linear_algebra->delete_matrix_double(vqf_state->rest_gyr_lp_state[2U]);
	if (vqf_state->rest_gyr_lp_state[1U] != NULL) linear_algebra->delete_matrix_double(vqf_state->rest_gyr_lp_state[1U]);
	if (vqf_state->rest_gyr_lp_state[0U] != NULL) linear_algebra->delete_matrix_double(vqf_state->rest_gyr_lp_state[0U]);

	// Cleanup the rest detection and magnetic disturbance detection matrices.
	if (vqf_state->mag_nrm_dip		!= NULL) linear_algebra->delete_matrix(vqf_state->mag_nrm_dip);
	if (vqf_state->rest_last_acc_lp	!= NULL) linear_algebra->delete_matrix(vqf_state->rest_last_acc_lp);
	if (vqf_state->rest_last_gyr_lp	!= NULL) linear_algebra->delete_matrix(vqf_state->rest_last_gyr_lp);

#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	// Cleanup the motion bias estimation matrices.
	if (vqf_state->motion_bias_est_corr		!= NULL) linear_algebra->delete_matrix(vqf_state->motion_bias_est_corr);
	if (vqf_state->motion_bias_est_K		!= NULL) linear_algebra->delete_matrix(vqf_state->motion_bias_est_K);
	if (vqf_state->motion_bias_est_W		!= NULL) linear_algebra->delete_matrix(vqf_state->motion_bias_est_W);
	if (vqf_state->motion_bias_est_bias_lp	!= NULL) linear_algebra->delete_matrix(vqf_state->motion_bias_est_bias_lp);
	if (vqf_state->motion_bias_est_Rt		!= NULL) linear_algebra->delete_matrix(vqf_state->motion_bias_est_Rt);
	if (vqf_state->motion_bias_est_R		!= NULL) linear_algebra->delete_matrix(vqf_state->motion_bias_est_R);
	if (vqf_state->bias_P					!= NULL) linear_algebra->delete_matrix(vqf_state->bias_P);
	if (vqf_coeff->motion_bias_est_skew_ez	!= NULL) linear_algebra->delete_matrix(vqf_coeff->motion_bias_est_skew_ez);

	// Cleanup the low-pass filter states of the bias of the motion bias estimation.
	if (vqf_state->motion_bias_est_bias_lp_state[3U] != NULL) linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_bias_lp_state[3U]);
	if (vqf_state->motion_bias_est_bias_lp_state[2U] != NULL) linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_bias_lp_state[2U]);
	if (vqf_state->motion_bias_est_bias_lp_state[1U] != NULL) linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_bias_lp_state[1U]);
	if (vqf_state->motion_bias_est_bias_lp_state[0U] != NULL) linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_bias_lp_state[0U]);

	// Cleanup the low-pass filter states of the rotation matrix of the motion bias estimation.
	if (vqf_state->motion_bias_est_R_lp_state[3U] != NULL) linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_R_lp_state[3U]);
	if (vqf_state->motion_bias_est_R_lp_state[2U] != NULL) linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_R_lp_state[2U]);
	if (vqf_state->motion_bias_est_R_lp_state[1U] != NULL) linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_R_lp_state[1U]);
	if (vqf_state->motion_bias_est_R_lp_state[0U] != NULL) linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_R_lp_state[0U]);
#endif

	// Cleanup the bias estimation matrices.
	if (vqf_state->motion_bias_est_e	!= NULL) linear_algebra->delete_matrix(vqf_state->motion_bias_est_e);
	if (vqf_state->bias					!= NULL) linear_algebra->delete_matrix(vqf_state->bias);

	// Cleanup the low-pass filter states of the accelerometer.
	if (vqf_state->acc_lp_state[3U] != NULL) linear_algebra->delete_matrix_double(vqf_state->acc_lp_state[3U]);
	if (vqf_state->acc_lp_state[2U] != NULL) linear_algebra->delete_matrix_double(vqf_state->acc_lp_state[2U]);
	if (vqf_state->acc_lp_state[1U] != NULL) linear_algebra->delete_matrix_double(vqf_state->acc_lp_state[1U]);
	if (vqf_state->acc_lp_state[0U] != NULL) linear_algebra->delete_matrix_double(vqf_state->acc_lp_state[0U]);

	// Cleanup the accelerometer/magnetometer update quaternions and matrices.
	if (vqf_state->mag_earth			!= NULL) linear_algebra->delete_matrix		(vqf_state->mag_earth);
	if (vqf_state->last_acc_lp			!= NULL) linear_algebra->delete_matrix		(vqf_state->last_acc_lp);
	if (vqf_state->acc_earth			!= NULL) linear_algebra->delete_matrix		(vqf_state->acc_earth);
	if (vqf_state->acc_mean_deviation	!= NULL) linear_algebra->delete_matrix		(vqf_state->acc_mean_deviation);
	if (vqf_state->acc_gyr_quat			!= NULL) linear_algebra->delete_quaternion	(vqf_state->acc_gyr_quat);
	if (vqf_state->acc_corr_quat		!= NULL) linear_algebra->delete_quaternion	(vqf_state->acc_corr_quat);
	if (vqf_state->acc_quat				!= NULL) linear_algebra->delete_quaternion	(vqf_state->acc_quat);

	// Cleanup the gyroscope update quaternions and matrices.
	if (vqf_state->gyr_no_bias			!= NULL) linear_algebra->delete_matrix		(vqf_state->gyr_no_bias);
	if (vqf_state->gyr_mean_deviation	!= NULL) linear_algebra->delete_matrix		(vqf_state->gyr_mean_deviation);
	if (vqf_state->gyr_step_quat		!= NULL) linear_algebra->delete_quaternion	(vqf_state->gyr_step_quat);
	if (vqf_state->gyr_quat				!= NULL) linear_algebra->delete_quaternion	(vqf_state->gyr_quat);

	// Cleanup the VQF context handle.
	free(vqf_context);

	return NULL;
}

void vqf_context_del(vqf_context_t* vqf_context) {
	const	vqf_linear_algebra_t*	vqf_linear_algebra	= &vqf_context->linear_algebra;
			vqf_coefficients_t*		vqf_coeff			= &vqf_context->coefficients;
			vqf_state_t*			vqf_state			= &vqf_context->state;

	// Cleanup the low-pass filter states of the magnetic norm and dip angle of magnetic disturbance detection.
	vqf_linear_algebra->delete_matrix_double(vqf_state->mag_nrm_dip_lp_state[3U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->mag_nrm_dip_lp_state[2U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->mag_nrm_dip_lp_state[1U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->mag_nrm_dip_lp_state[0U]);

	// Allocate the low-pass filter states of the magnetic norm and dip angle of magnetic disturbance detection.
	vqf_state->mag_nrm_dip_lp_state[3U] = NULL;
	vqf_state->mag_nrm_dip_lp_state[2U] = NULL;
	vqf_state->mag_nrm_dip_lp_state[1U] = NULL;
	vqf_state->mag_nrm_dip_lp_state[0U] = NULL;

	// Cleanup the low-pass filter states of the acceleration of rest detection.
	vqf_linear_algebra->delete_matrix_double(vqf_state->rest_acc_lp_state[3U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->rest_acc_lp_state[2U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->rest_acc_lp_state[1U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->rest_acc_lp_state[0U]);

	// Detach the low-pass filter states of the acceleration of rest detection.
	vqf_state->rest_acc_lp_state[3U] = NULL;
	vqf_state->rest_acc_lp_state[2U] = NULL;
	vqf_state->rest_acc_lp_state[1U] = NULL;
	vqf_state->rest_acc_lp_state[0U] = NULL;

	// Cleanup the low-pass filter states of the angular velocity of rest detection.
	vqf_linear_algebra->delete_matrix_double(vqf_state->rest_gyr_lp_state[3U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->rest_gyr_lp_state[2U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->rest_gyr_lp_state[1U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->rest_gyr_lp_state[0U]);

	// Detach the low-pass filter states of the angular velocity of rest detection.
	vqf_state->rest_gyr_lp_state[3U] = NULL;
	vqf_state->rest_gyr_lp_state[2U] = NULL;
	vqf_state->rest_gyr_lp_state[1U] = NULL;
	vqf_state->rest_gyr_lp_state[0U] = NULL;

	// Cleanup the rest detection and magnetic disturbance detection matrices.
	vqf_linear_algebra->delete_matrix(vqf_state->mag_nrm_dip);
	vqf_linear_algebra->delete_matrix(vqf_state->rest_last_acc_lp);
	vqf_linear_algebra->delete_matrix(vqf_state->rest_last_gyr_lp);

	// Detach the rest detection and magnetic disturbance detection matrices.
	vqf_state->mag_nrm_dip		= NULL;
	vqf_state->rest_last_acc_lp = NULL;
	vqf_state->rest_last_gyr_lp = NULL;

#ifndef VQF_NO_MOTION_BIAS_ESTIMATION
	// Cleanup the motion bias estimation matrices.
	vqf_linear_algebra->delete_matrix(vqf_state->motion_bias_est_corr);
	vqf_linear_algebra->delete_matrix(vqf_state->motion_bias_est_K);
	vqf_linear_algebra->delete_matrix(vqf_state->motion_bias_est_W);
	vqf_linear_algebra->delete_matrix(vqf_state->motion_bias_est_bias_lp);
	vqf_linear_algebra->delete_matrix(vqf_state->motion_bias_est_Rt);
	vqf_linear_algebra->delete_matrix(vqf_state->motion_bias_est_R);
	vqf_linear_algebra->delete_matrix(vqf_state->bias_P);
	vqf_linear_algebra->delete_matrix(vqf_coeff->motion_bias_est_skew_ez);

	// Detach the motion bias estimation matrices.
	vqf_state->motion_bias_est_corr		= NULL;
	vqf_state->motion_bias_est_K		= NULL;
	vqf_state->motion_bias_est_W		= NULL;
	vqf_state->motion_bias_est_bias_lp	= NULL;
	vqf_state->motion_bias_est_Rt		= NULL;
	vqf_state->motion_bias_est_R		= NULL;
	vqf_state->bias_P					= NULL;
	vqf_coeff->motion_bias_est_skew_ez	= NULL;

	// Cleanup the low-pass filter states of the bias of the motion bias estimation.
	vqf_linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_bias_lp_state[3U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_bias_lp_state[2U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_bias_lp_state[1U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_bias_lp_state[0U]);

	// Detach the low-pass filter states of the bias of the motion bias estimation.
	vqf_state->motion_bias_est_bias_lp_state[3U] = NULL;
	vqf_state->motion_bias_est_bias_lp_state[2U] = NULL;
	vqf_state->motion_bias_est_bias_lp_state[1U] = NULL;
	vqf_state->motion_bias_est_bias_lp_state[0U] = NULL;

	// Cleanup the low-pass filter states of the rotation matrix of the motion bias estimation.
	vqf_linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_R_lp_state[3U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_R_lp_state[2U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_R_lp_state[1U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->motion_bias_est_R_lp_state[0U]);

	// Detach the low-pass filter states of the rotation matrix of the motion bias estimation.
	vqf_state->motion_bias_est_R_lp_state[3U] = NULL;
	vqf_state->motion_bias_est_R_lp_state[2U] = NULL;
	vqf_state->motion_bias_est_R_lp_state[1U] = NULL;
	vqf_state->motion_bias_est_R_lp_state[0U] = NULL;
#endif

	// Cleanup the bias estimation matrices.
	vqf_linear_algebra->delete_matrix(vqf_state->motion_bias_est_e);
	vqf_linear_algebra->delete_matrix(vqf_state->bias);

	// Detach the bias estimation matrices.
	vqf_state->motion_bias_est_e	= NULL;
	vqf_state->bias					= NULL;

	// Cleanup the low-pass filter states of the accelerometer.
	vqf_linear_algebra->delete_matrix_double(vqf_state->acc_lp_state[3U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->acc_lp_state[2U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->acc_lp_state[1U]);
	vqf_linear_algebra->delete_matrix_double(vqf_state->acc_lp_state[0U]);

	// Detach the low-pass filter states of the accelerometer.
	vqf_state->acc_lp_state[3U] = NULL;
	vqf_state->acc_lp_state[2U] = NULL;
	vqf_state->acc_lp_state[1U] = NULL;
	vqf_state->acc_lp_state[0U] = NULL;

	// Cleanup the accelerometer/magnetometer update quaternions and matrices.
	vqf_linear_algebra->delete_matrix		(vqf_state->mag_earth);
	vqf_linear_algebra->delete_matrix		(vqf_state->last_acc_lp);
	vqf_linear_algebra->delete_matrix		(vqf_state->acc_earth);
	vqf_linear_algebra->delete_matrix		(vqf_state->acc_mean_deviation);
	vqf_linear_algebra->delete_quaternion	(vqf_state->acc_gyr_quat);
	vqf_linear_algebra->delete_quaternion	(vqf_state->acc_corr_quat);
	vqf_linear_algebra->delete_quaternion	(vqf_state->acc_quat);

	// Detach the accelerometer/magnetometer update quaternions and matrices.
	vqf_state->mag_earth			= NULL;
	vqf_state->last_acc_lp			= NULL;
	vqf_state->acc_earth			= NULL;
	vqf_state->acc_mean_deviation	= NULL;
	vqf_state->acc_gyr_quat			= NULL;
	vqf_state->acc_corr_quat		= NULL;
	vqf_state->acc_quat				= NULL;

	// Cleanup the gyroscope update quaternions and matrices.
	vqf_linear_algebra->delete_matrix		(vqf_state->gyr_no_bias);
	vqf_linear_algebra->delete_matrix		(vqf_state->gyr_mean_deviation);
	vqf_linear_algebra->delete_quaternion	(vqf_state->gyr_step_quat);
	vqf_linear_algebra->delete_quaternion	(vqf_state->gyr_quat);

	// Detach the gyroscope update quaternions and matrices.
	vqf_state->gyr_no_bias			= NULL;
	vqf_state->gyr_mean_deviation	= NULL;
	vqf_state->gyr_step_quat		= NULL;
	vqf_state->gyr_quat				= NULL;

	// Cleanup the VQF context handle.
	free(vqf_context);
}