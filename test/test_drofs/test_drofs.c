#include <unity.h>

#include "mock_test_data.h"
#include "mock_test_compressed_data.h"

#include "mock_test_compressor_compressed_data.h"
#include "mock_test_compressor_uncompressed_data.h"

#include <drofs.h>

#include <string.h>
#include <stdlib.h>

#include "drofs_timestamp_helper.h"
#include "drofs_compression_helper.h"

void setUp(){}
void tearDown(){}

void when_verifying_valid_data_return_true(){
    TEST_ASSERT_TRUE(drofs_verify(mock_test_data, mock_test_data_len));
}

void when_verifying_bad_header_return_false(){
    size_t bad_data_length = mock_test_data_len;
    uint8_t * bad_data = malloc(bad_data_length);
    memcpy(bad_data, mock_test_data, bad_data_length);

    //change header 
    const char * bad_header = "BADHD";
    memcpy(bad_data, bad_header, strlen(bad_header));

    bool is_valid = drofs_verify(bad_data, bad_data_length);
    free (bad_data);

    TEST_ASSERT_FALSE(is_valid);
}

void when_verifying_bad_crc_return_false(){

    size_t bad_data_length = mock_test_data_len;
    uint8_t * bad_data = malloc(bad_data_length);
    memcpy(bad_data, mock_test_data, bad_data_length);

    //change header 
    uint32_t bad_crc32 = 0xBADBEEF;
    memcpy(bad_data + 5, &bad_crc32, sizeof(bad_crc32));

    bool is_valid = drofs_verify(bad_data, bad_data_length);
    free (bad_data);

    TEST_ASSERT_FALSE(is_valid);
}

void when_parsing_root_return_root_directory(){
    struct drofs_entry_t entry;
    bool found = drofs_get_entry(mock_test_data, mock_test_data_len, "/",&entry );
    drofs_print_entry(entry);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL(ENTRY_TYPE_DIRECTORY, entry.type);
}

void when_reading_file1_txt_verify_contents(){
    struct drofs_entry_t entry;
    bool found = drofs_get_entry(mock_test_data, mock_test_data_len, "/file1.txt",&entry );
    drofs_print_entry(entry);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL(ENTRY_TYPE_FILE, entry.type);
    TEST_ASSERT_EQUAL(21, entry.data_length);
    TEST_ASSERT_EQUAL_STRING_LEN("Hello, this is file1.", entry.data, entry.data_length);
}

void when_reading_file2_txt_verify_contents(){
    struct drofs_entry_t entry;
    bool found = drofs_get_entry(mock_test_data, mock_test_data_len, "/subdir/file2.txt",&entry );
    drofs_print_entry(entry);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL(ENTRY_TYPE_FILE, entry.type);
    TEST_ASSERT_EQUAL(32, entry.data_length);
    TEST_ASSERT_EQUAL_STRING_LEN("This is file2 in a subdirectory.", entry.data, entry.data_length);

    bool verified = drofs_verify_entry(&entry);
    TEST_ASSERT_TRUE(verified);
}

void when_listing_files_under_subdir_verify_file2_exists(){
    struct drofs_entry_t entry;
    bool found = drofs_get_entry(mock_test_data, mock_test_data_len, "/subdir",&entry );
    drofs_print_entry(entry);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL(ENTRY_TYPE_DIRECTORY, entry.type);
    TEST_ASSERT_EQUAL(1, entry.children_length);

    struct drofs_entry_t first_child;
    drofs_get_nth_child(mock_test_data, mock_test_data_len, 0,&entry,  &first_child);
    drofs_print_entry(first_child);
    TEST_ASSERT_EQUAL_STRING("file2.txt", first_child.name);
    bool verified = drofs_verify_entry(&first_child);
    TEST_ASSERT_TRUE(verified);

    struct drofs_metadata_t original_size_metadata;
    bool got_original_size_metadata = drofs_get_type_metadata(&first_child,METADATA_TYPE_ORIGINAL_SIZE, &original_size_metadata );
    TEST_ASSERT_TRUE(got_original_size_metadata);
    TEST_ASSERT_EQUAL(first_child.data_length, (uint32_t)(*((uint32_t*)original_size_metadata.data)));

    struct drofs_metadata_t modify_date_metadata;
    bool got_modify_date_metadata = drofs_get_type_metadata(&first_child,METADATA_TYPE_TIMESTAMP, &modify_date_metadata );
    TEST_ASSERT_TRUE(got_modify_date_metadata);

   
    uint32_t timestamp = (uint32_t)(*((uint32_t*)modify_date_metadata.data));
    TEST_ASSERT_GREATER_OR_EQUAL(1758000000, timestamp);
}

void when_converting_timestamp_to_string_return_date(){
    uint32_t timestamp = 1759254090;

    char timestamp_buffer[26];
    format_timestamp(timestamp, timestamp_buffer, sizeof(timestamp_buffer));
    TEST_ASSERT_EQUAL_STRING("2025-09-30 17:41:30", timestamp_buffer);
}


void verify_mock_test_compressed_crc32(){
    crc32_context_t crc32_ctx;
    crc32_init(&crc32_ctx);
    crc32_update(&crc32_ctx,mock_test_compressor_compressed_data, mock_test_compressor_compressed_data_len);
    uint32_t data_crc = crc32_get(&crc32_ctx);
    TEST_ASSERT_EQUAL_HEX32(mock_test_compressor_compressed_data_crc32, data_crc);
}

void verify_mock_test_uncompressed_crc32(){
    crc32_context_t crc32_ctx;
    crc32_init(&crc32_ctx);
    crc32_update(&crc32_ctx,mock_test_compressor_uncompressed_data, mock_test_compressor_uncompressed_data_len);
    uint32_t data_crc = crc32_get(&crc32_ctx);
    TEST_ASSERT_EQUAL_HEX32(mock_test_compressor_uncompressed_data_crc32, data_crc);
}

void when_uncompressing_data_crc32_should_be_equal_to_uncompressed_crc32(){
    drofs_decompression_context_t * ctx = drofs_decompress_create(mock_test_compressor_compressed_data, mock_test_compressor_compressed_data_len);

    crc32_context_t crc32_ctx;
    crc32_init(&crc32_ctx);

    size_t input_index = 0;
    while (true){
        uint8_t buf[1024];
        size_t buf_len = sizeof(buf);
        
        tinfl_status status = drofs_decompress_chunk(ctx, buf, &buf_len);
        crc32_update(&crc32_ctx, buf, buf_len);
        if (buf_len == 0){
            break;
        }
    }

    drofs_decompress_free(ctx);

    uint32_t data_crc = crc32_get(&crc32_ctx);
    TEST_ASSERT_EQUAL_HEX32(mock_test_compressor_uncompressed_data_crc32, data_crc);
}

void when_uncompressing_data_in_chunks_validate_output(){
    drofs_decompression_context_t * ctx = drofs_decompress_create(mock_test_compressor_compressed_data, mock_test_compressor_compressed_data_len);

    size_t input_index = 0;
    size_t bytes_left = mock_test_compressor_uncompressed_data_len;
    while (bytes_left > 0){
        uint8_t buf[1024];
        size_t buf_len = sizeof(buf);
        
        tinfl_status status = drofs_decompress_chunk(ctx, buf, &buf_len);
        printf("comparing %d - %d, left %d, actual %d bytes, status %d\n", input_index, sizeof(buf), bytes_left, buf_len, status);

        TEST_ASSERT_EQUAL_UINT8_ARRAY(&mock_test_compressor_uncompressed_data[input_index], buf, buf_len);
        input_index+= buf_len;
        bytes_left-=buf_len;

    }

    drofs_decompress_free(ctx);
}

void when_reading_file2_txt_verify_contents_using_original_crc32(){
    struct drofs_entry_t entry;
    bool found = drofs_get_entry(mock_test_compressed_data, mock_test_compressed_data_len, "/drofs2s.png",&entry );
    drofs_print_entry(entry);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL(ENTRY_TYPE_FILE, entry.type);
    TEST_ASSERT_EQUAL(9534, entry.data_length);
    
    drofs_decompression_context_t * ctx = drofs_decompress_create(entry.data, entry.data_length);

    struct drofs_metadata_t original_size_metadata;
    bool got_original_size_metadata = drofs_get_type_metadata(&entry,METADATA_TYPE_ORIGINAL_SIZE, &original_size_metadata );
    TEST_ASSERT_TRUE(got_original_size_metadata);
    uint32_t original_size_value = (uint32_t)(*((uint32_t*)original_size_metadata.data));

    crc32_context_t crc32_ctx;
    crc32_init(&crc32_ctx);

    size_t input_index = 0;
    size_t bytes_left = original_size_value;
    while (bytes_left > 0){
        uint8_t buf[1024];
        size_t buf_len = sizeof(buf);
        
        tinfl_status status = drofs_decompress_chunk(ctx, buf, &buf_len);
        printf("hashing %d - %d, left %d, actual %d bytes, status %d\n", input_index, sizeof(buf), bytes_left, buf_len, status);

        crc32_update(&crc32_ctx, buf, buf_len);

        if (buf_len == 0){
            break;
        }
        
        input_index+= buf_len;
        bytes_left-=buf_len;

    }

    drofs_decompress_free(ctx);

    uint32_t original_data_crc = crc32_get(&crc32_ctx);

    struct drofs_metadata_t original_crc32;
    bool got_original_crc32_metadata = drofs_get_type_metadata(&entry,METADATA_TYPE_ORIGINAL_CRC32, &original_crc32 );
    TEST_ASSERT_TRUE(got_original_crc32_metadata);
    uint32_t original_crc32_value = (uint32_t)(*((uint32_t*)original_crc32.data));
    TEST_ASSERT_EQUAL_HEX32(original_crc32_value, original_data_crc);

}

int main(void) {
    UNITY_BEGIN(); // Start Unity test framework
    RUN_TEST(when_verifying_valid_data_return_true);
    RUN_TEST(when_verifying_bad_header_return_false);
    RUN_TEST(when_verifying_bad_crc_return_false);
    RUN_TEST(when_parsing_root_return_root_directory);
    RUN_TEST(when_reading_file1_txt_verify_contents);
    RUN_TEST(when_reading_file2_txt_verify_contents);
    RUN_TEST(when_listing_files_under_subdir_verify_file2_exists);
    RUN_TEST(when_converting_timestamp_to_string_return_date);
    RUN_TEST(verify_mock_test_compressed_crc32);
    RUN_TEST(verify_mock_test_uncompressed_crc32);
    RUN_TEST(when_uncompressing_data_crc32_should_be_equal_to_uncompressed_crc32);
    RUN_TEST(when_uncompressing_data_in_chunks_validate_output);
    RUN_TEST(when_reading_file2_txt_verify_contents_using_original_crc32);
    return UNITY_END(); // End Unity test framework
}

void app_main() {
    main();
}
