#pragma once

#include <stdint.h>
#include <stddef.h> // Required for size_t

/**
 * @brief Formats a Unix timestamp into a human-readable date and time string.
 *
 * This function converts a given Unix timestamp (seconds since the Epoch)
 * into a formatted string "YYYY-MM-DD HH:MM:SS" and stores it in the provided buffer.
 * The conversion uses local time.
 *
 * @param timestamp The Unix timestamp (uint32_t) to format.
 * @param buffer A pointer to the character array where the formatted string will be stored.
 *               The buffer must be large enough to hold the formatted string and a null terminator.
 *               A buffer size of at least 20 bytes is recommended (e.g., "YYYY-MM-DD HH:MM:SS\0").
 * @param buffer_size The maximum number of bytes that can be written to the buffer,
 *                    including the null terminator. If the formatted string, including
 *                    the null terminator, would exceed this size, the string is truncated.
 */
void format_timestamp(uint32_t timestamp, char *buffer, size_t buffer_size);
