//
// Copyright (c) 2013 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//

#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// \brief A structure to hold the CRC32 context.
typedef struct crc32_context_t {
    uint32_t state; ///< The current CRC32 state.
} crc32_context_t;

/// \brief Initialize the CRC32 context.
/// \param ctx The CRC32 context to initialize.
void crc32_init(crc32_context_t *ctx);


/// \brief Update the CRC32 context with a block of data.
/// \param ctx The CRC32 context to update.
/// \param data A pointer to the data block.
/// \param data_length The length of the data block.
void crc32_update(crc32_context_t *ctx, const uint8_t *data, size_t data_length);


/// \brief Get the current CRC32 value from the context.
/// \param ctx The CRC32 context.
/// \return The current CRC32 value.
uint32_t crc32_get(crc32_context_t *ctx);

#ifdef __cplusplus
}
#endif
