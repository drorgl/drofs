#include "unity.h"
#include <crc32.h>
#include <string.h>



const uint8_t ALL_ZEROS[32] = { 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00 };

const uint32_t ALL_ZEROS_CHECKSUM = 0x190A55AD;

const uint8_t ALL_ONES[32] =  { 0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF };

const uint32_t ALL_ONES_CHECKSUM = 0xFF6CAB0B;

const uint8_t INCREASING[32] = { 0x00, 0x01, 0x02, 0x03,
                                 0x04, 0x05, 0x06, 0x07,
                                 0x08, 0x09, 0x0A, 0x0B,
                                 0x0C, 0x0D, 0x0E, 0x0F,
                                 0x10, 0x11, 0x12, 0x13,
                                 0x14, 0x15, 0x16, 0x17,
                                 0x18, 0x19, 0x1A, 0x1B,
                                 0x1C, 0x1D, 0x1E, 0x1F };

const uint32_t INCREASING_CHECKSUM = 0x91267E8A;

const uint8_t DECREASING[32] = { 0x1F, 0x1E, 0x1D, 0x1C,
                                 0x1B, 0x1A, 0x19, 0x18,
                                 0x17, 0x16, 0x15, 0x14,
                                 0x13, 0x12, 0x11, 0x10,
                                 0x0F, 0x0E, 0x0D, 0x0C,
                                 0x0B, 0x0A, 0x09, 0x08,
                                 0x07, 0x06, 0x05, 0x04,
                                 0x03, 0x02, 0x01, 0x00 };

const uint32_t DECREASING_CHECKSUM = 0x9AB0EF72;

const uint8_t ISCSI_PDU[48] = { 0x01, 0xC0, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x14, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x04, 0x00,
                                0x00, 0x00, 0x00, 0x14,
                                0x00, 0x00, 0x00, 0x18,
                                0x28, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x02, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00 };

const uint32_t ISCSI_PDU_CHECKSUM = 0x51E17412;

void setUp(void) {}

void tearDown(void) {}

void test_crc32_init(void) {
    crc32_context_t ctx;
    crc32_init(&ctx);
    TEST_ASSERT_EQUAL_UINT32(0x00000000, crc32_get(&ctx));
}

void test_crc32_0x01(){
    crc32_context_t ctx;
    crc32_init(&ctx);
    uint8_t data_single[] = {0x01};
    crc32_update(&ctx, data_single, sizeof(data_single));
    TEST_ASSERT_EQUAL_HEX32(0xa505df1b, crc32_get(&ctx));
}

void test_crc32_A(){
    crc32_context_t ctx_A;
    crc32_init(&ctx_A);
    uint8_t data_A[] = {'A'};
    crc32_update(&ctx_A, data_A, sizeof(data_A));
    TEST_ASSERT_EQUAL_HEX32(0xD3D99E8B, crc32_get(&ctx_A));
}

void test_crc32_hello_world(){
    crc32_context_t ctx_hello;
    crc32_init(&ctx_hello);
    uint8_t hello_data[] = "Hello World";
    crc32_update(&ctx_hello, hello_data, strlen((char*)hello_data));
    TEST_ASSERT_EQUAL_HEX32(0x4A17B156, crc32_get(&ctx_hello));
}


void test_crc_all_zeros(){
    crc32_context_t ctx;
    crc32_init(&ctx);
    crc32_update(&ctx, ALL_ZEROS, sizeof(ALL_ZEROS));
    TEST_ASSERT_EQUAL_HEX32(ALL_ZEROS_CHECKSUM, crc32_get(&ctx));
}

void test_crc_all_ones(){
    crc32_context_t ctx;
    crc32_init(&ctx);
    crc32_update(&ctx, ALL_ONES, sizeof(ALL_ONES));
    TEST_ASSERT_EQUAL_HEX32(ALL_ONES_CHECKSUM, crc32_get(&ctx));
}

void test_crc_increasing(){
    crc32_context_t ctx;
    crc32_init(&ctx);
    crc32_update(&ctx, INCREASING, sizeof(INCREASING));
    TEST_ASSERT_EQUAL_HEX32(INCREASING_CHECKSUM, crc32_get(&ctx));
}

void test_crc_decreasing(){
    crc32_context_t ctx;
    crc32_init(&ctx);
    crc32_update(&ctx, DECREASING, sizeof(DECREASING));
    TEST_ASSERT_EQUAL_HEX32(DECREASING_CHECKSUM, crc32_get(&ctx));
}
    
void test_crc_iscsi_pdu(){
    crc32_context_t ctx;
    crc32_init(&ctx);
    crc32_update(&ctx, ISCSI_PDU, sizeof(ISCSI_PDU));
    TEST_ASSERT_EQUAL_HEX32(ISCSI_PDU_CHECKSUM, crc32_get(&ctx));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_crc32_init);
    RUN_TEST(test_crc32_0x01);
    RUN_TEST(test_crc32_A);
    RUN_TEST(test_crc32_hello_world);
    RUN_TEST(test_crc_all_zeros);
    RUN_TEST(test_crc_all_ones);
    RUN_TEST(test_crc_increasing);
    RUN_TEST(test_crc_decreasing);
    RUN_TEST(test_crc_iscsi_pdu);
    return UNITY_END();
}

void app_main() {
    main();
}
