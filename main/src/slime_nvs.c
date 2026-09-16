#include "stdlib.h"
#include "string.h"
#include "esp_log.h"
#include "slime_nvs.h"

/**
 * @brief The log tag of the Slime NVS.
 */
static const char* TAG = "slime_nvs";

/**
 * @brief The global initialization state of the NVS flash.
 */
static uint8_t nvs_flash_initialized = false;

esp_err_t slime_nvs_load_blob(
	const	slime_nvs_context_t*	nvs_context,
	const	slime_nvs_blob_key_t*	nvs_blob_key,
			void*					nvs_blob_out
) {
	// We cannot proceed without a handle, a key, and the handle to receive the loaded blob.
	ESP_RETURN_ON_FALSE(nvs_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_nvs_context_t handle provided when performing loading NVS blob.");
	ESP_RETURN_ON_FALSE(nvs_blob_key != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_nvs_blob_key_t handle provided when performing loading NVS blob.");
	ESP_RETURN_ON_FALSE(nvs_blob_out != NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to receive the loaded blob when performing loading NVS blob.");

	// Log the operation if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Loading blob \"%s\" of %zu byte(s) from NVS context.",
			/* s	*/ nvs_blob_key->blob_name,
			/* zu	*/ nvs_blob_key->blob_size
		);
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Extract the blob size.
	size_t nvs_size = nvs_blob_key->blob_size;

	// Load the blob from the NVS context.
	const esp_err_t nvs_error = nvs_get_blob(
		/* c_handle		= */ nvs_context->nvs_handle,
		/* key			= */ nvs_blob_key->blob_name,
		/* out_value	= */ nvs_blob_out,
		/* length		= */ &nvs_size
	);

	// Reset to default value if the blob does not exist or the size does not match.
	// Check these errors first instead check ESP_OK first because it will return ESP_OK when the data in NVS is smaller
	// than required.
	if (	nvs_error	==	ESP_ERR_NVS_NOT_FOUND
		||	nvs_error	==	ESP_ERR_NVS_INVALID_LENGTH
		||	nvs_size	<	nvs_blob_key->blob_size
	) {
		// Log the operation if NVS debug logging is enabled.
		#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
			ESP_LOGD(TAG, "Blob does not exist or the size does not match, resetting to default value.");
		#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

		// Copy the default value to the out handle.
		memcpy(
			/* dst_buffer	= */ nvs_blob_out,
			/* src_buffer	= */ nvs_blob_key->blob_default,
			/* length		= */ nvs_blob_key->blob_size
		);

		#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
			ESP_LOGD(TAG, "Saving default value to the NVS context.");
		#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

		// Save the value to NVS context.
		ESP_RETURN_ON_ERROR(slime_nvs_save_blob(
			/* nvs_context	= */ nvs_context,
			/* nvs_blob_key	= */ nvs_blob_key,
			/* nvs_blob_in	= */ nvs_blob_out
		), TAG, "Failed to save default value of blob to NVS context.");

		return ESP_OK;
	} else if (nvs_error == ESP_OK) {
		// Return if the blob was successfully read get from the NVS context.
		return ESP_OK;
	}

	// Log the unexpected error.
	ESP_LOGE(TAG, "Unexpected NVS error \"%s\" occurred when getting blob from NVS context.", esp_err_to_name(nvs_error));
	return nvs_error;
}

esp_err_t slime_nvs_save_blob(
	const slime_nvs_context_t*	nvs_context,
	const slime_nvs_blob_key_t*	nvs_blob_key,
	const void*					nvs_blob_in
) {
	// We cannot proceed without a handle, a key and the data of the blob to save.
	ESP_RETURN_ON_FALSE(nvs_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_nvs_context_t handle provided when performing saving NVS blob.");
	ESP_RETURN_ON_FALSE(nvs_blob_key != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_nvs_blob_key_t handle provided when performing saving NVS blob.");
	ESP_RETURN_ON_FALSE(nvs_blob_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No data handle provided when performing saving NVS blob.");

	// Log the operation if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Saving blob \"%s\" of %zu byte(s) to NVS context.",
			/* s	*/ nvs_blob_key->blob_name,
			/* zu	*/ nvs_blob_key->blob_size
		);
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Set the data of the blob at the NVS partition.
	ESP_RETURN_ON_ERROR(nvs_set_blob(
		/* c_handle	= */ nvs_context->nvs_handle,
		/* key		= */ nvs_blob_key->blob_name,
		/* value	= */ nvs_blob_in,
		/* length	= */ nvs_blob_key->blob_size
	), TAG, "Failed to set NVS blob data.");

	// Commit the data to NVS flash.
	ESP_RETURN_ON_ERROR(nvs_commit(nvs_context->nvs_handle), TAG, "Failed to commit NVS blob data.");

	return ESP_OK;
}

esp_err_t slime_nvs_context_new(
			slime_nvs_context_t**		nvs_context_out,
	const	slime_nvs_context_config_t*	nvs_context_config
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration and a handle to receive the created NVS context.
	ESP_RETURN_ON_FALSE(nvs_context_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No nvs_context_out handle provided when creating NVS context.");
	ESP_RETURN_ON_FALSE(nvs_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No nvs_context_config handle provided when creating NVS context.");

	// Log the progress if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating NVS context.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Skip the NVS flash initialization if the NVS flash is already initialized or need to skip.
	if (nvs_flash_initialized || nvs_context_config->nvs_skip_init) {
		// Log the progress if NVS debug logging is enabled.
		#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
			ESP_LOGD(TAG, "Skip the initialization of NVS flash.");
		#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING
	} else {
		// Log the progress if NVS debug logging is enabled.
		#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
			ESP_LOGD(TAG, "Initializing NVS flash.");
		#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

		// Try initializing the NVS flash.
		const esp_err_t nvs_error = nvs_flash_init();

		if (	nvs_error == ESP_ERR_NVS_NO_FREE_PAGES
			||	nvs_error == ESP_ERR_NVS_NEW_VERSION_FOUND
		) {
			// Log the progress if NVS debug logging is enabled.
			#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
				ESP_LOGD(TAG, "Fail to initialize NVS flash, erase then try again.");
			#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

			// Erase the NVS flash then re-initialize the NVS flash if no free space or incompatible version.
			ESP_RETURN_ON_ERROR(nvs_flash_erase	(), TAG, "Failed to erase NVS flash.");
			ESP_RETURN_ON_ERROR(nvs_flash_init	(), TAG, "Failed to initialize NVS flash.");
		} else if (nvs_error != ESP_OK) {
			// Log the unexpected error.
			ESP_LOGE(TAG, "Unexpected NVS error \"%s\" occurred when initializing NVS flash.", esp_err_to_name(nvs_error));
			// Abort.
			return nvs_error;
		}
	}

	// Mark the NVS flash initialized.
	nvs_flash_initialized = true;

	// Log the progress if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Reserving NVS handle.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Reserve the NVS handle.
	nvs_handle_t context_nvs_handle;

	// Log the progress if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Open NVS partition.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Open the NVS partition.
	ESP_RETURN_ON_ERROR(nvs_open_from_partition(
		/* part_name		= */ nvs_context_config->nvs_partition,
		/* namespace_name	= */ nvs_context_config->nvs_namespace,
		/* open_mode		= */ NVS_READWRITE,
		/* out_handle		= */ &context_nvs_handle
	), TAG, "Failed to open NVS partition.");

	// Log the progress if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Reserving NVS context struct.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Reserve the NVS context struct handle.
	slime_nvs_context_t* nvs_context;

	// Log the progress if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating NVS context struct.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Create the NVS context struct.
	nvs_context = calloc(1, sizeof(slime_nvs_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(nvs_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create NVS context struct.");

	// Log the progress if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Finializing NVS context struct.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Fill the NVS context.
	nvs_context->nvs_handle = context_nvs_handle;

	// Return the created NVS context.
	*nvs_context_out = nvs_context;

	// Log the progress if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "NVS context has been created.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred: %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Cleaning up resources.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Only NVS partition is opened here, close the NVS partition.
	nvs_close(context_nvs_handle);

	return ret;
}

esp_err_t slime_nvs_context_del(slime_nvs_context_t* nvs_context_in) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(nvs_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No nvs_context_in handle provided when releasing NVS context.");

	// Log the progress if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing NVS context.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Log the progress if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Closing NVS partition.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Close the NVS partition.
	nvs_close(nvs_context_in->nvs_handle);

	// Log the progress if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing NVS context struct.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	// Free the NVS context struct.
	free(nvs_context_in);

	// Skip the detaching because theoretically no value is safe.
	// nvs_context_in->nvs_handle = ???;

	// Log the progress if NVS debug logging is enabled.
	#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
		ESP_LOGD(TAG, "NVS context has been released.");
	#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

	return ESP_OK;
}