#include "endian.h"
#include "stdlib.h"
#include "string.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "slime_buffer.h"

/**
 * @brief The log tag of the Slime Buffer.
 */
static const char* TAG = "slime_buffer";

/**
 * @brief The definition of the debug logging function of the Slime Buffer.
 */
#ifdef CONFIG_SLIME_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_BUFFER_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_BUFFER_LOG(format, ...)
#endif // CONFIG_SLIME_DEBUG_LOGGING

/**
 * @brief				Read a byte from the given offset of the given buffer, then increment the offset by 1.
 * @param src_offset	The handle of the offset.
 * @param src_buffer	The buffer to be read.
 * @param src_length	The length of the buffer to be read.
 * @param dst_value		The handle to receive the value of the byte.
 * @return				The status of reading.
 */
esp_err_t buffer_read_byte(
			uint32_t*	src_offset,
	const	uint8_t*	src_buffer,
	const	uint32_t	src_length,
			uint8_t*	dst_value
) {
	// We cannot proceed without a source buffer slice and a destination buffer.
	ESP_RETURN_ON_FALSE(dst_value	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No destination buffer provided when reading byte.");
	ESP_RETURN_ON_FALSE(src_buffer	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No source buffer provided when reading byte.");
	ESP_RETURN_ON_FALSE(src_offset	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No source offset provided when reading byte.");

	// We cannot proceed if there is an overflow.
	ESP_RETURN_ON_FALSE(*src_offset + 1 <= src_length, ESP_ERR_INVALID_ARG, TAG, "Buffer overflowed when reading bytes.");

	// Read the byte value from the source buffer. write the value into the destination value.
	*dst_value = src_buffer[*src_offset];

	// Increment the offset by 1 byte.
	*src_offset += 1U;

	return ESP_OK;
}

/**
 * @brief				Read given number of bytes from the given offset of the given buffer, then increment the offset by the length of the bytes read.
 * @param src_offset	The handle of the offset.
 * @param src_buffer	The buffer to be read.
 * @param src_length	The length of the buffer to be read.
 * @param dst_buffer	The buffer to receive the value of the read bytes.
 * @param dst_length	The length of the bytes to be read.
 * @return				The status of reading.
 */
esp_err_t buffer_read_bytes(
			uint32_t*	src_offset,
	const	uint8_t*	src_buffer,
	const	uint32_t	src_length,
			void*		dst_buffer,
	const	uint32_t	dst_length
) {
	// We cannot proceed without a source buffer slice and a destination buffer.
	ESP_RETURN_ON_FALSE(dst_buffer != NULL, ESP_ERR_INVALID_ARG, TAG, "No destination buffer provided when reading bytes.");
	ESP_RETURN_ON_FALSE(src_buffer != NULL, ESP_ERR_INVALID_ARG, TAG, "No source buffer provided when reading bytes.");
	ESP_RETURN_ON_FALSE(src_offset != NULL, ESP_ERR_INVALID_ARG, TAG, "No source offset provided when reading bytes.");

	// We cannot proceed if there is an overflow.
	ESP_RETURN_ON_FALSE(*src_offset + dst_length <= src_length, ESP_ERR_INVALID_ARG, TAG, "Buffer overflowed when reading bytes.");

	// Skip if the length is 0.
	if (dst_length == 0U) {
		return ESP_OK;
	}

	// Copy the bytes in the source buffer into the given offset of the destination buffer.
	memcpy(
		/* dst_buffer	= */ dst_buffer,
		/* src_buffer	= */ src_buffer + (*src_offset),
		/* length		= */ dst_length
	);

	// Increment the offset by the byte length of the destination buffer.
	*src_offset += dst_length;

	return ESP_OK;
}

/**
 * @brief				Write a byte to the given offset of the given buffer, then increment the offset by 1.
 * @param dst_offset	The handle of the offset.
 * @param dst_buffer	The buffer that the byte to be written to.
 * @param dst_length	The length of the buffer to be written to.
 * @param src_value		The value of the byte to be written to the buffer.
 * @return				The status of writing.
 */
esp_err_t buffer_write_byte(
			uint32_t*	dst_offset,
			uint8_t*	dst_buffer,
	const	uint32_t	dst_length,
	const	uint8_t		src_value
) {
	// We cannot proceed without a source buffer slice and a destination buffer.
	ESP_RETURN_ON_FALSE(dst_buffer != NULL, ESP_ERR_INVALID_ARG, TAG, "No destination buffer provided when writing bytes.");
	ESP_RETURN_ON_FALSE(dst_offset != NULL, ESP_ERR_INVALID_ARG, TAG, "No destination offset provided when writing bytes.");

	// We cannot proceed if there is an overflow.
	ESP_RETURN_ON_FALSE(*dst_offset + 1 <= dst_length, ESP_ERR_INVALID_ARG, TAG, "Buffer overflowed when reading bytes.");

	// Write the byte value into the given offset of the destination buffer.
	dst_buffer[*dst_offset] = src_value;

	// Increment the offset by 1 byte.
	*dst_offset += 1U;

	return ESP_OK;
}

/**
 * @brief				Write given number of bytes to the given offset of the given buffer, then increment the offset by the length of the bytes written.
 * @param dst_offset	The handle of the offset.
 * @param dst_length	The length of the buffer to be written to.
 * @param dst_buffer	The buffer that the bytes to be written to.
 * @param src_buffer	The buffer of the bytes to be written.
 * @param src_length	The length of the bytes to be written.
 * @return				The status of writing.
 */
esp_err_t buffer_write_bytes(
			uint32_t*	dst_offset,
			uint8_t*	dst_buffer,
	const	uint32_t	dst_length,
	const	void*		src_buffer,
	const	uint32_t	src_length
) {
	// We cannot proceed without a source buffer slice and a destination buffer.
	ESP_RETURN_ON_FALSE(dst_buffer != NULL, ESP_ERR_INVALID_ARG, TAG, "No destination buffer provided when writing bytes.");
	ESP_RETURN_ON_FALSE(dst_offset != NULL, ESP_ERR_INVALID_ARG, TAG, "No destination offset provided when writing bytes.");
	ESP_RETURN_ON_FALSE(src_buffer != NULL, ESP_ERR_INVALID_ARG, TAG, "No source buffer provided when writing bytes.");

	// We cannot proceed if there is an overflow.
	ESP_RETURN_ON_FALSE(*dst_offset + src_length <= dst_length, ESP_ERR_INVALID_ARG, TAG, "Buffer overflowed when writing bytes.");

	// Skip if the length is 0.
	if (src_length == 0U) {
		return ESP_OK;
	}

	// Copy the bytes in the source buffer into the given offset of the destination buffer.
	memcpy(
		/* dst_buffer	= */ dst_buffer + (*dst_offset),
		/* src_buffer	= */ src_buffer,
		/* length		= */ src_length
	);

	// Increment the offset by the byte length of the source buffer.
	*dst_offset += src_length;

	return ESP_OK;
}

esp_err_t slime_buffer_read_buffer(
			slime_buffer_context_t*	buffer_context,
			void*					dst_buffer,
	const	uint32_t				dst_length
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when reading buffer context.");

	// Read the buffer from the reader index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_read_bytes(
		/* src_offset = */ &	buffer_context->reader_index,
		/* src_buffer = */		buffer_context->buffer,
		/* src_length = */		buffer_context->writer_index,
		/* dst_buffer = */		dst_buffer,
		/* dst_length = */		dst_length
	), TAG, "Failed to read buffer from buffer context.");

	return ESP_OK;
}

esp_err_t slime_buffer_read_uint8(slime_buffer_context_t* buffer_context, uint8_t* dst_value) {
	// We cannot proceed without a context and a handle to receive the result.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when reading buffer context.");

	// Read the uint8_t from the reader index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_read_byte(
		/* src_offset	= */ &	buffer_context->reader_index,
		/* src_buffer	= */	buffer_context->buffer,
		/* src_length	= */	buffer_context->writer_index,
		/* dst_value	= */	dst_value
	), TAG, "Failed to read uint8_t from buffer context.");

	return ESP_OK;
}

esp_err_t slime_buffer_read_uint16(slime_buffer_context_t* buffer_context, uint16_t* dst_value) {
	// We cannot proceed without a context and a handle to receive the result.
	ESP_RETURN_ON_FALSE(buffer_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when reading buffer context.");
	ESP_RETURN_ON_FALSE(dst_value		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to receive the result when reading buffer context.");

	// Reserve space for the uint16.
	uint16_t dst_buffer = 0U;

	// Read the Big-Endian uint16_t from the reader index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_read_bytes(
		/* src_offset = */ &	buffer_context->reader_index,
		/* src_buffer = */		buffer_context->buffer,
		/* src_length = */		buffer_context->writer_index,
		/* dst_buffer = */ &	dst_buffer,
		/* dst_length = */		sizeof(uint16_t)
	), TAG, "Failed to read uint16 from buffer context.");

	// Convert the uint16 to Little-Endian then return it.
	*dst_value = be16toh(dst_buffer);

	return ESP_OK;
}

esp_err_t slime_buffer_read_uint32(slime_buffer_context_t* buffer_context, uint32_t* dst_value) {
	// We cannot proceed without a context and a handle to receive the result.
	ESP_RETURN_ON_FALSE(buffer_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when reading buffer context.");
	ESP_RETURN_ON_FALSE(dst_value		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to receive the result when reading buffer context.");

	// Reserve space for the uint32.
	uint32_t dst_buffer = 0U;

	// Read the Big-Endian uint32_t from the reader index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_read_bytes(
		/* src_offset = */ &	buffer_context->reader_index,
		/* src_buffer = */		buffer_context->buffer,
		/* src_length = */		buffer_context->writer_index,
		/* dst_buffer = */ &	dst_buffer,
		/* dst_length = */		sizeof(uint32_t)
	), TAG, "Failed to read uint32 from buffer context.");

	// Convert the uint32 to Little-Endian then return it.
	*dst_value = be32toh(dst_buffer);

	return ESP_OK;
}

esp_err_t slime_buffer_read_uint64(slime_buffer_context_t* buffer_context, uint64_t* dst_value) {
	// We cannot proceed without a context and a handle to receive the result.
	ESP_RETURN_ON_FALSE(buffer_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when reading buffer context.");
	ESP_RETURN_ON_FALSE(dst_value		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to receive the result when reading buffer context.");

	// Reserve space for the uint64.
	uint64_t dst_buffer = 0U;

	// Read the Big-Endian uint64_t from the reader index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_read_bytes(
		/* src_offset = */ &	buffer_context->reader_index,
		/* src_buffer = */		buffer_context->buffer,
		/* src_length = */		buffer_context->writer_index,
		/* dst_buffer = */ &	dst_buffer,
		/* dst_length = */		sizeof(uint64_t)
	), TAG, "Failed to read uint64 from buffer context.");

	// Convert the uint64 to Little-Endian then return it.
	*dst_value = be64toh(dst_buffer);

	return ESP_OK;
}

esp_err_t slime_buffer_read_float(slime_buffer_context_t* buffer_context, float_t* dst_value) {
	// We cannot proceed without a context and a handle to receive the result.
	ESP_RETURN_ON_FALSE(buffer_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when reading buffer context.");
	ESP_RETURN_ON_FALSE(dst_value		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to receive the result when reading buffer context.");

	// Reserve space for the raw bytes of the float in Big-Endian.
	uint32_t dst_bytes = 0U;

	// Read the Big-Endian uint32_t from the reader index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_read_bytes(
		/* src_offset = */ &	buffer_context->reader_index,
		/* src_buffer = */		buffer_context->buffer,
		/* src_length = */		buffer_context->writer_index,
		/* dst_buffer = */ &	dst_bytes,
		/* dst_length = */		sizeof(uint32_t)
	), TAG, "Failed to read raw bytes of a float from buffer context.");

	// Convert the raw bytes to Little-Endian.
	dst_bytes = be32toh(dst_bytes);

	// Return the raw bytes as float.
	memcpy(
		/* dst_buffer	= */ &	dst_value,
		/* src_buffer	= */ &	dst_bytes,
		/* length		= */	sizeof(float_t)
	);

	return ESP_OK;
}

esp_err_t slime_buffer_read_skip(slime_buffer_context_t* buffer_context, const uint32_t count) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when skipping reading bytes.");

	// Calculate the new reader index after skipping.
	const uint32_t reader_index = buffer_context->reader_index + count;

	// We cannot proceed if there is an overflow.
	ESP_RETURN_ON_FALSE(reader_index <= buffer_context->writer_index, ESP_ERR_INVALID_STATE, TAG, "Buffer overflowed when skipping reading bytes.");

	// Update reader index.
	buffer_context->reader_index = reader_index;

	return ESP_OK;
}

esp_err_t slime_buffer_write_buffer(
			slime_buffer_context_t*	buffer_context,
	const	void*					src_buffer,
	const	uint32_t				src_length
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when writing buffer context.");

	// Write the buffer to the writer index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_write_bytes(
		/* dst_offset = */ &	buffer_context->writer_index,
		/* dst_buffer = */		buffer_context->buffer,
		/* dst_length = */		buffer_context->length,
		/* src_buffer = */		src_buffer,
		/* src_length = */		src_length
	), TAG, "Failed to write buffer to buffer context.");

	return ESP_OK;
}

esp_err_t slime_buffer_write_uint8(slime_buffer_context_t* buffer_context, const uint8_t src_value) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when writing buffer context.");

	// Write the uint8_t to the writer index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_write_byte(
		/* dst_offset	= */ &	buffer_context->writer_index,
		/* dst_buffer	= */	buffer_context->buffer,
		/* dst_length	= */	buffer_context->length,
		/* src_value	= */	src_value
	), TAG, "Failed to write uint8_t to buffer context.");

	return ESP_OK;
}

esp_err_t slime_buffer_write_uint16(slime_buffer_context_t* buffer_context, const uint16_t src_value) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when writing buffer context.");

	// Convert the value into Big-Endian uint16_t.
	const uint16_t src_buffer = htobe16(src_value);

	// Write the Big-Endian uint16_t to the writer index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_write_bytes(
		/* dst_offset = */ &	buffer_context->writer_index,
		/* dst_buffer = */		buffer_context->buffer,
		/* dst_length = */		buffer_context->length,
		/* src_buffer = */ &	src_buffer,
		/* src_length = */		sizeof(uint16_t)
	), TAG, "Failed to write Big-Endian uint32_t to buffer context.");

	return ESP_OK;
}

esp_err_t slime_buffer_write_uint32(slime_buffer_context_t* buffer_context, const uint32_t src_value) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when writing buffer context.");

	// Convert the value into Big-Endian uint32_t.
	const uint32_t src_buffer = htobe32(src_value);

	// Write the Big-Endian uint32_t to the writer index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_write_bytes(
		/* dst_offset = */ &	buffer_context->writer_index,
		/* dst_buffer = */		buffer_context->buffer,
		/* dst_length = */		buffer_context->length,
		/* src_buffer = */ &	src_buffer,
		/* src_length = */		sizeof(uint32_t)
	), TAG, "Failed to write Big-Endian uint32_t to buffer context.");

	return ESP_OK;
}

esp_err_t slime_buffer_write_uint64(slime_buffer_context_t*buffer_context, const uint64_t src_value) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when writing buffer context.");

	// Convert the value into Big-Endian uint64_t.
	const uint64_t src_buffer = htobe32(src_value);

	// Write the Big-Endian uint64_t to the writer index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_write_bytes(
		/* dst_offset = */ &	buffer_context->writer_index,
		/* dst_buffer = */		buffer_context->buffer,
		/* dst_length = */		buffer_context->length,
		/* src_buffer = */ &	src_buffer,
		/* src_length = */		sizeof(uint64_t)
	), TAG, "Failed to write Big-Endian uint64_t to buffer context.");

	return ESP_OK;
}

esp_err_t slime_buffer_write_float(slime_buffer_context_t* buffer_context, const float_t src_value) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when writing buffer context.");

	// Reserve space for the raw bytes of the float in Big-Endian.
	uint32_t src_bytes = 0U;

	// Copy the float to the reserved space as raw bytes.
	memcpy(
		/* dst_buffer	= */ &	src_bytes,
		/* src_buffer	= */ &	src_value,
		/* length		= */	sizeof(float_t)
	);

	// Convert the raw bytes to Big-Endian.
	src_bytes = htobe32(src_bytes);

	// Write the Big-Endian uint32_t to the writer index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_write_bytes(
		/* dst_offset = */ &	buffer_context->writer_index,
		/* dst_buffer = */		buffer_context->buffer,
		/* dst_length = */		buffer_context->length,
		/* src_buffer = */ &	src_bytes,
		/* src_length = */		sizeof(uint32_t)
	), TAG, "Failed to write raw bytes of float to buffer context.");

	return ESP_OK;
}

esp_err_t slime_buffer_write_short_string(slime_buffer_context_t* buffer_context, const char* src_string) {
	// We cannot proceed without a context and a string buffer handle to send.
	ESP_RETURN_ON_FALSE(buffer_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when writing buffer context.");
	ESP_RETURN_ON_FALSE(src_string		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No string provided when writing buffer context.");

	// Calculate the length of the short string to write.
	const uint8_t src_length = (uint8_t) strnlen(src_string, 255U);

	// Write the length of the short string to the writer index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_write_byte(
		/* dst_offset	= */ &	buffer_context->writer_index,
		/* dst_buffer	= */	buffer_context->buffer,
		/* dst_length	= */	buffer_context->length,
		/* src_value	= */	src_length
	), TAG, "Failed to write length of the short string to buffer context.");

	// Write the bytes of the short string to the writer index of the buffer context.
	ESP_RETURN_ON_ERROR(buffer_write_bytes(
		/* dst_offset = */ &	buffer_context->writer_index,
		/* dst_buffer = */		buffer_context->buffer,
		/* dst_length = */		buffer_context->length,
		/* src_buffer = */		src_string,
		/* src_length = */		src_length
	), TAG, "Failed to write the content of the short string to buffer context.");

	return ESP_OK;
}

esp_err_t slime_buffer_write_skip(slime_buffer_context_t* buffer_context, const uint32_t count) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when skipping writing bytes.");

	// Calculate the new writer index after skipping.
	const uint32_t writer_index_new = buffer_context->writer_index + count;

	// We cannot proceed if there is an overflow.
	ESP_RETURN_ON_FALSE(writer_index_new <= buffer_context->length, ESP_ERR_INVALID_STATE, TAG, "Buffer overflowed when skipping writing bytes.");

	// Update reader index.
	buffer_context->writer_index = writer_index_new;

	return ESP_OK;
}

esp_err_t slime_buffer_reset(slime_buffer_context_t* buffer_context) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when resetting buffer context.");

	// Reset the reader and writer index to 0.
	buffer_context->writer_index = 0U;
	buffer_context->reader_index = 0U;

	return ESP_OK;
}

esp_err_t slime_buffer_rewind(slime_buffer_context_t* buffer_context) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when rewinding buffer context.");

	// Reset the reader index to 0.
	buffer_context->reader_index = 0U;

	return ESP_OK;
}

esp_err_t slime_buffer_context_new(slime_buffer_context_t** buffer_context_out, const slime_buffer_context_config_t* buffer_context_config) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration and a handle to receive the created buffer context.
	ESP_RETURN_ON_FALSE(buffer_context_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when creating buffer context.");
	ESP_RETURN_ON_FALSE(buffer_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_config_t handle provided when creating buffer context.");

	// Log the progress.
	SLIME_BUFFER_LOG("Creating buffer context.");
	SLIME_BUFFER_LOG("Reserving handles of buffer context.");

	// Reserve the handles of buffer context.
	slime_buffer_context_t*	buffer_context = NULL;
	uint8_t*				context_buffer = NULL;

	// Log the progress.
	SLIME_BUFFER_LOG("Creating buffer.");

	// Get the length and capabilities from configuration.
	const uint32_t length	= buffer_context_config->length;
	const uint32_t flags	= buffer_context_config->flags;

	// Create the buffer with given length and capabilities.
	context_buffer = heap_caps_calloc(length, sizeof(uint8_t), flags);

	// Check the allocations.
	ESP_GOTO_ON_FALSE(context_buffer != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create buffer.");

	// Log the progress.
	SLIME_BUFFER_LOG("Creating buffer context struct.");

	// Create the buffer context struct.
	buffer_context = calloc(1U, sizeof(slime_buffer_context_t));

	// Check the allocations.

	ESP_GOTO_ON_FALSE(buffer_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create buffer context struct.");

	// Log the progress.
	SLIME_BUFFER_LOG("Finalizing buffer context.");

	// Fill the buffer context.
	buffer_context->buffer			= context_buffer;
	buffer_context->length			= length;
	buffer_context->writer_index	= 0U;
	buffer_context->reader_index	= 0U;

	// Return the created buffer context.
	*buffer_context_out = buffer_context;

	// Log the progress.
	SLIME_BUFFER_LOG("Buffer context has been created.");

	return ret;

	error:

	// Log the error.
	SLIME_BUFFER_LOG("Error occurred: %s", esp_err_to_name(ret));
	SLIME_BUFFER_LOG("Cleaning up resources.");

	if (buffer_context) free(buffer_context); // Cleanup the buffer context struct.
	if (context_buffer) free(context_buffer); // Cleanup the buffer.

	return ret;
}

esp_err_t slime_buffer_context_del(slime_buffer_context_t* buffer_context_in) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(buffer_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when releasing buffer context.");

	// Log the progress.
	SLIME_BUFFER_LOG("Releasing buffer context.");
	SLIME_BUFFER_LOG("Releasing buffer.");

	// Free the buffer.
	free(buffer_context_in->buffer);

	// Log the progress.
	SLIME_BUFFER_LOG("Detaching all fields of buffer context.");

	// Detaching all fields.
	buffer_context_in->buffer		= NULL;
	buffer_context_in->length		= 0U;
	buffer_context_in->writer_index	= 0U;
	buffer_context_in->reader_index	= 0U;

	// Log the progress.
	SLIME_BUFFER_LOG("Releasing buffer context struct.");

	// Free the buffer context struct.
	free(buffer_context_in);

	// Log the progress.
	SLIME_BUFFER_LOG("Buffer context has been released.");

	return ESP_OK;
}