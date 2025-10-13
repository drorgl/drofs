#include "crc32.h"
#include <stddef.h> // For size_t

#include <inttypes.h>

#define FLASH_READ_DWORD(x) (*(uint32_t*)(x))

static const uint32_t crc32_table[] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

void crc32_init(crc32_context_t *ctx){
    ctx->state = 0xFFFFFFFF;
}

static void crc32_update_byte(crc32_context_t *ctx, const uint8_t data){
    // via http://forum.arduino.cc/index.php?topic=91179.0
    uint8_t tbl_idx = 0;

    tbl_idx = ctx->state ^ (data >> (0 * 4));
    ctx->state = FLASH_READ_DWORD(crc32_table + (tbl_idx & 0x0f)) ^ (ctx->state >> 4);
    tbl_idx = ctx->state ^ (data >> (1 * 4));
    ctx->state = FLASH_READ_DWORD(crc32_table + (tbl_idx & 0x0f)) ^ (ctx->state >> 4);
}

void crc32_update(crc32_context_t *ctx, const uint8_t *data, size_t data_length){
    for (size_t i = 0; i < data_length; i++)
    {
        crc32_update_byte(ctx, data[i]);
    }
}

uint32_t crc32_get(crc32_context_t *ctx){
    return ~ctx->state;
}
