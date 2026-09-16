#ifndef SLIME_NVS_H
#define SLIME_NVS_H

#include "esp_check.h"
#include "nvs.h"
#include "nvs_flash.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The configuration struct of the NVS context.
 */
typedef struct {
	const	char*	nvs_partition; /*!< The partition name of the NVS context. */
	const	char*	nvs_namespace; /*!< The namespace of the NVS context. */
			uint8_t	nvs_skip_init; /*!< True if the NVS flash is already initialized. */
} slime_nvs_context_config_t;

/**
 * @brief The key struct of an NVS blob.
 */
typedef struct {
	const	char*	blob_name;		/*!< The name key of the blob. */
	const	void*	blob_default;	/*!< The default data of the blob if not found. */
			size_t	blob_size;		/*!< The size of the blob. */
} slime_nvs_blob_key_t;

/**
 * @brief The NVS context struct.
 */
typedef struct {
	nvs_handle_t nvs_handle; /*!< The NVS handle of the NVS context. */
} slime_nvs_context_t;

/**
 * @brief				Load a blob from the NVS context.
 * @param nvs_context	The NVS context to load the blob.
 * @param nvs_blob_key	The key of the blob.
 * @param nvs_blob_out	The handle to receive the loaded blob from NVS.
 * @return				The status of loading blob.
 */
esp_err_t slime_nvs_load_blob(
	const	slime_nvs_context_t*	nvs_context,
	const	slime_nvs_blob_key_t*	nvs_blob_key,
			void*					nvs_blob_out
);

/**
 * @brief				Save a blob to the NVS context.
 * @param nvs_context	The NVS context to save the blob.
 * @param nvs_blob_key	The key of the blob.
 * @param nvs_blob_in	The handle of the blob to save.
 * @return				The status of saving blob.
 */
esp_err_t slime_nvs_save_blob(
	const slime_nvs_context_t*	nvs_context,
	const slime_nvs_blob_key_t*	nvs_blob_key,
	const void*					nvs_blob_in
);

/**
 * @brief						Create the NVS context.
 * @param nvs_context_out		The handle to receive the created NVS context.
 * @param nvs_context_config	The configuration of the NVS context.
 * @return						The status of the creation.
 */
esp_err_t slime_nvs_context_new(
			slime_nvs_context_t**		nvs_context_out,
	const	slime_nvs_context_config_t*	nvs_context_config
);

/**
 * @brief					Release the NVS context.
 * @param nvs_context_in	The NVS context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_nvs_context_del(slime_nvs_context_t* nvs_context_in);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_NVS_H
