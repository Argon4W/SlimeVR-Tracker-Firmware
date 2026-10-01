#ifndef SLIME_FUSION_VQF_H
#define SLIME_FUSION_VQF_H

#include "slime_fusion.h"

/**
 * @brief Define the VQF matrix handle type to avoid casting void* before calling CEigen functions.
 */
#define VQF_MATRIX_HANDLE_TYPE ceigen_matrix_handle_t

/**
 * @brief Define the VQF double precision matrix handle type to avoid casting void* before calling CEigen functions.
 */
#define VQF_MATRIX_DOUBLE_HANDLE_TYPE ceigen_matrix_double_handle_t

/**
 * @brief Define the VQF quaternion handle type to avoid casting void* before calling CEigen functions.
 */
#define VQF_QUATERNION_HANDLE_TYPE ceigen_quaternion_handle_t

#include "vqf.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief						Create a new VQF fusion context.
 * @param fusion_context_out	The handle to receive the created VQF fusion context.
 * @param fusion_context_name	The name of the fusion context.
 * @param fusion_context_config	The configuration of the fusion context.
 * @param sensor_context		The sensor context of the fusion context.
 * @return						The status of the creation.
 */
esp_err_t slime_vqf_fusion_context_new(
			slime_fusion_context_t**	fusion_context_out,
	const	char*						fusion_context_name,
	const	void*						fusion_context_config,
	const	slime_sensor_context_t*		sensor_context
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_FUSION_VQF_H
