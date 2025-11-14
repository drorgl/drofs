/**
 * @file drofs_compression_helper.h
 * @brief Helper functions for DROFS (Decompressed Read-Only File System) compression and decompression.
 *
 * This file provides an interface for decompressing data using the miniz library.
 * It defines a context for decompression and functions to create, free, and
 * decompress data in chunks.
 */
#pragma once
#include <miniz.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Opaque structure representing a decompression context.
 *
 * This context holds the state required for incremental decompression of a data stream.
 */
typedef struct drofs_decompression_context drofs_decompression_context_t;

/**
 * @brief Creates a new decompression context and initializes it with input data.
 *
 * This function allocates and initializes a `drofs_decompression_context_t` structure
 * for decompressing a given input buffer. The input buffer is expected to contain
 * zlib-compressed data.
 *
 * @param input_buf Pointer to the zlib-compressed input data buffer.
 * @param input_buf_len Length of the input data buffer in bytes.
 * @return A pointer to the newly created `drofs_decompression_context_t` on success,
 *         or NULL if memory allocation fails or initialization fails.
 */
drofs_decompression_context_t *drofs_decompress_create(
    const uint8_t *input_buf,
    size_t input_buf_len);

/**
 * @brief Frees a decompression context.
 *
 * This function releases all resources associated with the given decompression context.
 *
 * @param ctx Pointer to the `drofs_decompression_context_t` to be freed.
 */
void drofs_decompress_free(drofs_decompression_context_t *ctx);

/**
 * @brief Decompresses a chunk of data from the input buffer into an output buffer.
 *
 * This function attempts to decompress data from the internal input buffer of the
 * context and write it to the provided output buffer. It can be called repeatedly
 * to decompress data incrementally.
 *
 * @param ctx Pointer to the `drofs_decompression_context_t`.
 * @param output_buffer Pointer to the buffer where decompressed data will be written.
 * @param output_buffer_len IN: Capacity of the output buffer in bytes.
 *                          OUT: Number of bytes actually written to the output buffer.
 * @return A `tinfl_status` indicating the status of the decompression operation.
 *         Possible values include TINFL_STATUS_DONE (decompression complete),
 *         TINFL_STATUS_HAS_MORE_OUTPUT (output buffer full, more data to decompress),
 *         TINFL_STATUS_NEEDS_MORE_INPUT (input buffer exhausted, but not done),
 *         or an error code.
 */
tinfl_status drofs_decompress_chunk(
    drofs_decompression_context_t *ctx,
    uint8_t *output_buffer,
    size_t *output_buffer_len);

#ifdef __cplusplus
}
#endif
