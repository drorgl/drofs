#include "drofs_compression_helper.h"

#include <miniz.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct drofs_decompression_context
{
    tinfl_decompressor decompressor;
    uint8_t dict[TINFL_LZ_DICT_SIZE];

    // --- Input Tracking ---
    const uint8_t *input_ptr; // Pointer to the start of the current input chunk
    size_t input_available;   // Total bytes in the current input chunk

    // --- Output Tracking ---
    size_t opos;  // Next byte to read from the dictionary (0 to TINFL_LZ_DICT_SIZE - 1)
    size_t osize; // Bytes available for user to read from the dictionary
} drofs_decompression_context_t;

drofs_decompression_context_t *drofs_decompress_create(
    const uint8_t *input_buf,
    size_t input_buf_len)
{
    drofs_decompression_context_t *ctx = malloc(sizeof(drofs_decompression_context_t));
    if (ctx == NULL)
    {
        // error allocating decompression context (about 32KB)
        printf("error allocating decompression context\n");
        return NULL;
    }
    tinfl_init(&ctx->decompressor);
    ctx->input_ptr = input_buf;
    ctx->input_available = input_buf_len;

    ctx->opos = 0;
    ctx->osize = 0;

    return ctx;
}

void drofs_decompress_free(drofs_decompression_context_t *ctx)
{
    free(ctx);
}

tinfl_status drofs_decompress_chunk(
    drofs_decompression_context_t *ctx,
    uint8_t *output_buffer,
    size_t *output_buffer_len) // IN: Capacity, OUT: Bytes written
{
    size_t out_capacity = *output_buffer_len;
    size_t out_bytes_written = 0;
    tinfl_status status = TINFL_STATUS_HAS_MORE_OUTPUT;

    while (out_bytes_written < out_capacity)
    {
        // 1. Transfer data from the dictionary (if any) to the user's buffer
        if (ctx->osize > 0)
        {
            size_t copy_len = out_capacity - out_bytes_written;
            if (copy_len > ctx->osize)
                copy_len = ctx->osize;

            memcpy(output_buffer + out_bytes_written, ctx->dict + ctx->opos, copy_len);

            ctx->opos += copy_len;
            ctx->osize -= copy_len;
            out_bytes_written += copy_len;

            // If the user's buffer is now full, we exit
            if (out_bytes_written == out_capacity)
            {
                printf("output buffer is full, breaking\n");
                break;
            }
        }

        // 2. Decompress new data (only if input is still available)
        if (ctx->input_available > 0)
        {
            // Set the pointers/sizes for tinfl_decompress based on context state
            const uint8_t *pIn_buf_next = ctx->input_ptr;
            size_t current_in_size = ctx->input_available; // Passed by reference

            mz_uint8 *pOut_buf_next = ctx->dict;
            size_t current_out_size = TINFL_LZ_DICT_SIZE; // The full dictionary size

            status = tinfl_decompress(
                &ctx->decompressor,
                pIn_buf_next, &current_in_size,
                ctx->dict,
                pOut_buf_next, &current_out_size,
                TINFL_FLAG_PARSE_ZLIB_HEADER | TINFL_FLAG_HAS_MORE_INPUT | TINFL_FLAG_COMPUTE_ADLER32);

            // CRITICAL: Update the context's internal input state
            ctx->input_ptr += current_in_size;
            ctx->input_available -= current_in_size;

            // 3. Process status and decompressed output
            if (status < 0)
            {
                printf("decompression failed %d\n", status);
                break; // Decompression failed
            }

            if (current_out_size > 0)
            {
                // New data was written to the dictionary.
                printf("new data available\n");
                ctx->opos = 0;
                ctx->osize = current_out_size;
                // Loop back to Step 1 to transfer this new data.
            }

            if (status == TINFL_STATUS_DONE)
            {
                printf("stream finished\n");
                // break; // Stream finished.
            }
        }
        else if (ctx->osize == 0)
        {
            // Input buffer is empty AND dictionary is empty. Nothing left to do.
            printf("no more input\n");
            status = TINFL_STATUS_NEEDS_MORE_INPUT;
            break;
        }
    }

    // Update the OUT parameter before returning
    *output_buffer_len = out_bytes_written;

    // If we exited the loop because input ran out, return NEEDS_MORE_INPUT
    if (status == TINFL_STATUS_DONE)
    {
        return TINFL_STATUS_DONE;
    }
    else if (ctx->input_available == 0 && ctx->osize == 0)
    {
        return TINFL_STATUS_NEEDS_MORE_INPUT;
    }
    return status;
}