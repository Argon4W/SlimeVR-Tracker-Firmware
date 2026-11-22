#include "stdlib.h"
#include "string.h"
#include "esp_log.h"
#include "slime_nvs.h"

/**
 * @brief The log tag of the Slime NVS.
 */
static const char* TAG = "slime_nvs";

/**
 * @brief The definitions of the debug logging function of the Slime NVS.
 */
#ifdef CONFIG_SLIME_NVS_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_NVS_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)

/**
 * @brief			Log the hexdump of the buffer.
 * @param buffer	The buffer to be dumped.
 * @param buff_len	The length of the buffer to be dumped.
 */
#define SLIME_NVS_LOG_HEXDUMP(buffer, buff_len) ESP_LOG_BUFFER_HEXDUMP(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_NVS_LOG(format, ...)

/**
 * @brief			Log the hexdump of the buffer (Inactive).
 * @param buffer	The buffer to be dumped.
 * @param buff_len	The length of the buffer to be dumped.
 */
#define SLIME_NVS_LOG_HEXDUMP(buffer, buff_len)
#endif // CONFIG_SLIME_NVS_DEBUG_LOGGING

esp_err_t slime_nvs_load_blob(
	const	slime_nvs_context_t*	nvs_context,
	const	slime_nvs_blob_key_t*	nvs_blob_key,
			void*					nvs_blob_out
) {
	// We cannot proceed without a handle, a key, and the handle to receive the loaded blob.
	ESP_RETURN_ON_FALSE(nvs_context		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_nvs_context_t handle provided when performing loading NVS blob.");
	ESP_RETURN_ON_FALSE(nvs_blob_key	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_nvs_blob_key_t handle provided when performing loading NVS blob.");
	ESP_RETURN_ON_FALSE(nvs_blob_out	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to receive the loaded blob when performing loading NVS blob.");

	// Log the operation.
	SLIME_NVS_LOG("NVS context is loading blob \"%s\" of %zu byte(s) from partition \"s\".",
		/* s	*/ nvs_blob_key	->blob_name,
		/* zu	*/ nvs_blob_key	->blob_size,
		/* s	*/ nvs_context	->nvs_partition
	);

	// Extract the blob size.
	size_t nvs_size = nvs_blob_key->size;

	// Load the blob from the NVS context.
	const esp_err_t nvs_error = nvs_get_blob(
		/* c_handle		= */ nvs_context->nvs_handle,
		/* key			= */ nvs_blob_key->name,
		/* out_value	= */ nvs_blob_out,
		/* length		= */ &nvs_size
	);

	// Reset to default value if the blob does not exist or the size does not match.
	// Check these errors first instead check ESP_OK first because it will return ESP_OK when the data in NVS is smaller
	// than required.
	if (	nvs_error	==	ESP_ERR_NVS_NOT_FOUND
		||	nvs_error	==	ESP_ERR_NVS_INVALID_LENGTH
		||	nvs_size	<	nvs_blob_key->size
	) {
		// Log the operation.
		SLIME_NVS_LOG("Blob does not exist or the size does not match, resetting to default value.");
		SLIME_NVS_LOG("Default content of the NVS blob: ");
		SLIME_NVS_LOG_HEXDUMP(
			/* buffer	= */ nvs_blob_key->init,
			/* buff_len	= */ nvs_blob_key->size
		);

		// Copy the default value to the out handle.
		memcpy(
			/* dst_buffer	= */ nvs_blob_out,
			/* src_buffer	= */ nvs_blob_key->init,
			/* length		= */ nvs_blob_key->size
		);

		// Log the operation.
		SLIME_NVS_LOG("Saving default value to the NVS context.");

		// Save the value to NVS context.
		ESP_RETURN_ON_ERROR(slime_nvs_save_blob(
			/* nvs_context	= */ nvs_context,
			/* nvs_blob_key	= */ nvs_blob_key,
			/* nvs_blob_in	= */ nvs_blob_out
		), TAG, "Failed to save default value of blob to NVS context.");

		return ESP_OK;
	} else if (nvs_error == ESP_OK) {
		// Log the result.
		SLIME_NVS_LOG("Loaded content of the NVS blob: ");
		SLIME_NVS_LOG_HEXDUMP(
			/* buffer	= */ nvs_blob_out,
			/* buff_len	= */ nvs_blob_key->size
		);

		// Return if the blob was successfully read get from the NVS context.
		return ESP_OK;
	}

	// Log the unexpected error.
	ESP_LOGE(TAG, "Failed to load blob from NVS context: %s.", esp_err_to_name(nvs_error));
	return nvs_error;
}

esp_err_t slime_nvs_save_blob(
	const slime_nvs_context_t*	nvs_context,
	const slime_nvs_blob_key_t*	nvs_blob_key,
	const void*					nvs_blob_in
) {
	// We cannot proceed without a handle, a key and the data of the blob to save.
	ESP_RETURN_ON_FALSE(nvs_context		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_nvs_context_t handle provided when performing saving NVS blob.");
	ESP_RETURN_ON_FALSE(nvs_blob_key	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_nvs_blob_key_t handle provided when performing saving NVS blob.");
	ESP_RETURN_ON_FALSE(nvs_blob_in		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No data handle provided when performing saving NVS blob.");

	// Log the operation.
	SLIME_NVS_LOG("NVS context is saving blob \"%s\" of %zu byte(s) to partition \"%s\".",
		/* s	*/ nvs_blob_key	->name,
		/* zu	*/ nvs_blob_key	->size,
		/* s	*/ nvs_context	->partition
	);

	// Log the buffer.
	SLIME_NVS_LOG("Saved content of the NVS blob: ");
	SLIME_NVS_LOG_HEXDUMP(
		/* buffer	= */ nvs_blob_in,
		/* buff_len	= */ nvs_blob_key->size
	);

	// Set the data of the blob at the NVS partition.
	ESP_RETURN_ON_ERROR(nvs_set_blob(
		/* c_handle	= */ nvs_context->nvs_handle,
		/* key		= */ nvs_blob_key->name,
		/* value	= */ nvs_blob_in,
		/* length	= */ nvs_blob_key->size
	), TAG, "Failed to set NVS blob data.");

	// Commit the data to NVS flash.
	ESP_RETURN_ON_ERROR(nvs_commit(nvs_context->nvs_handle), TAG, "Failed to commit NVS blob data.");

	return ESP_OK;
}

esp_err_t slime_nvs_context_new(slime_nvs_context_t** nvs_context_out, const slime_nvs_context_config_t* nvs_context_config) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration and a handle to receive the created NVS context.
	ESP_RETURN_ON_FALSE(nvs_context_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No nvs_context_out handle provided when creating NVS context.");
	ESP_RETURN_ON_FALSE(nvs_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No nvs_context_config handle provided when creating NVS context.");

	// Log the progress.
	SLIME_NVS_LOG("Creating NVS context.");
	SLIME_NVS_LOG("Initializing NVS flash.");

	// Try initializing the NVS flash of the partition.
	const esp_err_t nvs_error = nvs_flash_init_partition(nvs_context_config->partition);

	if (	nvs_error == ESP_ERR_NVS_NO_FREE_PAGES
		||	nvs_error == ESP_ERR_NVS_NEW_VERSION_FOUND
	) {
		// Log the progress.
		SLIME_NVS_LOG("Fail to initialize NVS flash, erase then try again.");

		// Erase the NVS flash of the partition then re-initialize the NVS flash if no free space or incompatible version.
		ESP_RETURN_ON_ERROR(nvs_flash_erase	(), TAG, "Failed to erase NVS flash.");
		ESP_RETURN_ON_ERROR(nvs_flash_init	(), TAG, "Failed to initialize NVS flash.");
	} else if (nvs_error != ESP_OK) {
		// Log the unexpected error.
		ESP_LOGE(TAG, "Failed to initialize NVS flash: %s.", esp_err_to_name(nvs_error));
		// Abort.
		return nvs_error;
	}

	// Log the progress.
	SLIME_NVS_LOG("Reserving NVS handle.");

	// Reserve the NVS handle.
	nvs_handle_t nvs_handle;

	// Log the progress.
	SLIME_NVS_LOG("Open NVS partition.");

	// Open the NVS partition.
	ESP_RETURN_ON_ERROR(nvs_open_from_partition(
		/* part_name		= */ nvs_context_config->partition,
		/* namespace_name	= */ nvs_context_config->namespace,
		/* open_mode		= */ NVS_READWRITE,
		/* out_handle		= */ &nvs_handle
	), TAG, "Failed to open NVS partition.");

	// Log the progress.
	SLIME_NVS_LOG("Reserving NVS context struct.");

	// Reserve the NVS context struct handle.
	slime_nvs_context_t* nvs_context;

	// Log the progress.
	SLIME_NVS_LOG("Creating NVS context struct.");

	// Create the NVS context struct.
	nvs_context = calloc(1U, sizeof(slime_nvs_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(nvs_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create NVS context struct.");

	// Log the progress.
	SLIME_NVS_LOG(TAG, "Finalizing NVS context struct.");

	// Fill the NVS context.
	nvs_context->partition	= nvs_context_config->partition;
	nvs_context->nvs_handle	= nvs_handle;

	// Return the created NVS context.
	*nvs_context_out = nvs_context;

	// Log the progress.
	SLIME_NVS_LOG("NVS context has been created.");

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error.
	SLIME_NVS_LOG("Error occurred: %s", esp_err_to_name(ret));
	SLIME_NVS_LOG("Cleaning up resources.");

	// Only NVS partition is opened here, close the NVS partition.
	nvs_close(nvs_handle);

	// De-initialize the NVS flash of the partition.
	nvs_flash_deinit_partition(nvs_context_config->partition);

	return ret;
}

esp_err_t slime_nvs_context_del(slime_nvs_context_t* nvs_context_in) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(nvs_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No nvs_context_in handle provided when releasing NVS context.");

	// Log the progress.
	SLIME_NVS_LOG("Releasing NVS context.");
	SLIME_NVS_LOG("Closing NVS partition.");

	// Close the NVS partition.
	nvs_close(nvs_context_in->nvs_handle);

	// Log the progress.
	SLIME_NVS_LOG("De-initializing NVS flash.");

	// De-initialize the NVS flash of the partition.
	nvs_flash_deinit_partition(nvs_context_in->partition);

	// Log the progress.
	SLIME_NVS_LOG("Releasing NVS context struct.");

	// Free the NVS context struct.
	free(nvs_context_in);

	// Skip the detaching because theoretically no value is safe.
	// nvs_context_in->nvs_handle = ???;

	// Log the progress.
	SLIME_NVS_LOG("NVS context has been released.");

	return ESP_OK;
}