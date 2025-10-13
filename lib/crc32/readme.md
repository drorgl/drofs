# CRC32 Library

This directory contains a CRC32 calculation library, originally copyrighted by [Christopher Baker](https://github.com/bakercp/CRC32).

## Features

- Standard CRC32 calculation.
- Functions for initialization, updating with data blocks, and retrieving the final CRC32 value.
- Optimized for efficient calculation using a precomputed lookup table.

## Usage

### `crc32_context_t`

A structure to hold the CRC32 context.

```c
typedef struct crc32_context_t {
    uint32_t state; ///< The current CRC32 state.
} crc32_context_t;
```

### `void crc32_init(crc32_context_t *ctx)`

Initializes the CRC32 context. The state is set to `0xFFFFFFFF`.

### `void crc32_update(crc32_context_t *ctx, const uint8_t *data, size_t data_length)`

Updates the CRC32 context with a block of data. The data is processed byte by byte.

### `uint32_t crc32_get(crc32_context_t *ctx)`

Retrieves the current CRC32 value from the context. The returned value is the bitwise NOT of the internal state.

## Example

```c
#include "crc32.h"
#include <stdio.h>
#include <string.h>

int main() {
    crc32_context_t ctx;
    crc32_init(&ctx);

    const char* data = "Hello, CRC32!";
    crc32_update(&ctx, (const uint8_t*)data, strlen(data));

    uint32_t crc = crc32_get(&ctx);
    printf("CRC32 of \"%s\": 0x%08X\n", data, crc);

    return 0;
}
```
