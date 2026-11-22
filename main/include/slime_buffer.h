#ifndef SLIME_BUFFER_H
#define SLIME_BUFFER_H

#include "math.h"
#include "stdint.h"
#include "esp_check.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The configuration struct of the buffer context.
 */
typedef struct {
	uint32_t length;	/*!< The length of the buffer. */
	uint32_t flags;		/*!< The extra capability flags when allocating buffer. */
} slime_buffer_context_config_t;

/**
 * @brief The buffer context struct.
 */
typedef struct {
	uint8_t* buffer;		/*!< The buffer holding the data. */
	uint32_t length;		/*!< The length of the buffer. */
	uint32_t writer_index;	/*!< The index of the next first byte in the buffer to be written. */
	uint32_t reader_index;	/*!< The index of the next first byte in the buffer to be read. */
} slime_buffer_context_t;

/**
 * @brief					Read given number of bytes from the reader index of the buffer context then increase the
 *							reader index by the count of bytes read.
 * @param buffer_context	The buffer context to read bytes.
 * @param dst_buffer		The buffer to receive the value of the read bytes.
 * @param dst_length		The length of the bytes to be read.
 * @return					The status of reading.
 */
esp_err_t slime_buffer_read_buffer(
	slime_buffer_context_t*	buffer_context,
	void*					dst_buffer,
	uint32_t				dst_length
);

/**
 * @brief					Read an uint8 from the reader index of the buffer context then increase the reader index by 1.
 * @param buffer_context	The buffer context to read the uint8.
 * @param dst_value			The handle to receive the value of the read uint8.
 * @return					The status of reading.
 */
esp_err_t slime_buffer_read_uint8(slime_buffer_context_t* buffer_context, uint8_t* dst_value);

/**
 * @brief					Read an uint16 from the reader index of the buffer context then increase the reader index by 2.
 * @param buffer_context	The buffer context to read the uint16.
 * @param dst_value			The handle to receive the value of the read uint16.
 * @return					The status of reading.
 */
esp_err_t slime_buffer_read_uint16(slime_buffer_context_t* buffer_context, uint16_t* dst_value);

/**
 * @brief					Read an uint32 from the reader index of the buffer context then increase the reader index by 4.
 * @param buffer_context	The buffer context to read the uint32.
 * @param dst_value			The handle to receive the value of the read uint32.
 * @return					The status of reading.
 */
esp_err_t slime_buffer_read_uint32(slime_buffer_context_t* buffer_context, uint32_t* dst_value);

/**
 * @brief					Read an uint64 from the reader index of the buffer context then increase the reader index by 8.
 * @param buffer_context	The buffer context to read the uint64.
 * @param dst_value			The handle to receive the value of the read uint64.
 * @return					The status of reading.
 */
esp_err_t slime_buffer_read_uint64(slime_buffer_context_t* buffer_context, uint64_t* dst_value);

/**
 * @brief					Read a float from the reader index of the buffer context then increase the reader index by 4.
 * @param buffer_context	The buffer context to read the float.
 * @param dst_value			The handle to receive the value of the read float.
 * @return					The status of reading.
 */
esp_err_t slime_buffer_read_float(slime_buffer_context_t* buffer_context, float_t* dst_value);

/**
 * @brief					Increases the reader index to skip reading by given number of bytes of the buffer context.
 * @param buffer_context	The buffer context to skip reading.
 * @param count				The count of bytes to be skipped.
 * @return					The status of skipping.
 */
esp_err_t slime_buffer_read_skip(slime_buffer_context_t* buffer_context, uint32_t count);

/**
 * @brief					Write given number of bytes to the writer index of the buffer context then increase the
 *							writer index by the count of bytes written.
 * @param buffer_context	The buffer context to write the bytes.
 * @param src_buffer		The buffer of the bytes to be written.
 * @param src_length		The length of the bytes to be written.
 * @return					The status of writing.
 */
esp_err_t slime_buffer_write_buffer(
			slime_buffer_context_t*	buffer_context,
	const	void*					src_buffer,
			uint32_t				src_length
);

/**
 * @brief					Write an uint8 to the writer index of the buffer context then increase the writer index by 1.
 * @param buffer_context	The buffer context to write the uint8.
 * @param src_value			The value of the uint8 to be written.
 * @return					The status of writing.
 */
esp_err_t slime_buffer_write_uint8(slime_buffer_context_t* buffer_context, uint8_t src_value);

/**
 * @brief					Write an uint16 to the writer index of the buffer context then increase the writer index by 2.
 * @param buffer_context	The buffer context to write the uint16.
 * @param src_value			The value of the uint16 to be written.
 * @return					The status of writing.
 */
esp_err_t slime_buffer_write_uint16(slime_buffer_context_t* buffer_context, uint16_t src_value);

/**
 * @brief					Write an uint32 to the writer index of the buffer context then increase the writer index by 4.
 * @param buffer_context	The buffer context to write the uint32.
 * @param src_value			The value of the uint32 to be written.
 * @return					The status of writing.
 */
esp_err_t slime_buffer_write_uint32(slime_buffer_context_t* buffer_context, uint32_t src_value);

/**
 * @brief					Write an uint64 to the writer index of the buffer context then increase the writer index by 8.
 * @param buffer_context	The buffer context to write the uint64.
 * @param src_value			The value of the uint64 to be written.
 * @return					The status of writing.
 */
esp_err_t slime_buffer_write_uint64(slime_buffer_context_t* buffer_context, uint64_t src_value);

/**
 * @brief					Write a float to the writer index of the buffer context then increase the writer index by 4.
 * @param buffer_context	The buffer context to write the float.
 * @param src_value			The value of the float to be written.
 * @return					The status of writing.
 */
esp_err_t slime_buffer_write_float(slime_buffer_context_t* buffer_context, float_t src_value);

/**
 * @brief					Write a short string (less than 255 characters) to the writer index of the buffer context
 *							then increase the writer index by 1 + strlen(src_string).
 * @param buffer_context	The buffer context to append the short string.
 * @param src_string		The short string to be written.
 * @return					The status of writing.
 */
esp_err_t slime_buffer_write_short_string(slime_buffer_context_t* buffer_context, const char* src_string);

/**
 * @brief					Increases the writer index to skip writing by given number of bytes of the buffer context.
 * @param buffer_context	The buffer context to skip writing.
 * @param count				The count of bytes to be skipped.
 * @return					The status of skipping.
 */
esp_err_t slime_buffer_write_skip(slime_buffer_context_t* buffer_context, uint32_t count);

/**
 * @brief					Reset the buffer context, set reader and writer index of the buffer context to 0.
 * @param buffer_context	The buffer context to be reset.
 * @return					The status of resetting
 */
esp_err_t slime_buffer_reset(slime_buffer_context_t* buffer_context);

/**
 * @brief					Rewind the buffer context, set reader index of the buffer context to 0.
 * @param buffer_context	The buffer context to be rewinded.
 * @return					The status of rewinding
 */
esp_err_t slime_buffer_rewind(slime_buffer_context_t* buffer_context);

/**
 * @brief						Create the buffer context.
 * @param buffer_context_out	The handle to receive the created buffer context.
 * @param buffer_context_config	The configuration of the buffer context.
 * @return						The status of the creation.
 */
esp_err_t slime_buffer_context_new(slime_buffer_context_t** buffer_context_out, const slime_buffer_context_config_t* buffer_context_config);

/**
 * @brief					Release the buffer context.
 * @param buffer_context_in	The buffer context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_buffer_context_del(slime_buffer_context_t* buffer_context_in);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_BUFFER_H
