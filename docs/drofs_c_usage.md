# DROFS C Library Usage

This section details how to interact with the DROFS file system using its C API.

## How to Verify the Binary Object

To ensure the integrity of the loaded DROFS image, use the `drofs_verify` function:

```c
#include <drofs.h>
#include "mock_test_data.h" // Assuming this contains mock_test_data and mock_test_data_len

bool is_valid = drofs_verify(mock_test_data, mock_test_data_len);
if (is_valid) {
    // Image is valid
} else {
    // Image is corrupted
}
```

## How to Open the Root Entity

The root directory of the DROFS file system can be accessed using the path `"/"` with the `drofs_get_entry` function:

```c
#include <drofs.h>
#include "mock_test_data.h"

struct drofs_entry_t root_entry;
bool found_root = drofs_get_entry(mock_test_data, mock_test_data_len, "/", &root_entry);

if (found_root && root_entry.type == ENTRY_TYPE_DIRECTORY) {
    // root_entry now holds the details of the root directory
}
```

## How to Iterate Over a Directory Object

To iterate over the children of a directory, first obtain the directory entry, then use `drofs_get_nth_child` in a loop:

```c
#include <drofs.h>
#include "mock_test_data.h"

struct drofs_entry_t dir_entry;
// Assume dir_entry is already populated for a directory, e.g., the root or a subdirectory

for (size_t i = 0; i < dir_entry.children_length; i++) {
    struct drofs_entry_t child_entry;
    bool got_child = drofs_get_nth_child(mock_test_data, mock_test_data_len, i, &dir_entry, &child_entry);
    if (got_child) {
        // Process child_entry (e.g., print its name, check its type)
        // drofs_print_entry(child_entry);
    }
}
```

## How to Get a Binary + Length for a Specified Entry

Once you have a `drofs_entry_t` for a file, its raw binary data and length are directly available:

```c
#include <drofs.h>
#include "mock_test_data.h"

struct drofs_entry_t file_entry;
bool found_file = drofs_get_entry(mock_test_data, mock_test_data_len, "/file1.txt", &file_entry);

if (found_file && file_entry.type == ENTRY_TYPE_FILE) {
    const uint8_t * file_data = file_entry.data;
    size_t file_data_length = file_entry.data_length;

    // file_data points to the content, file_data_length is its size
    // If the file is compressed (file_entry.flags & COMPRESSED), you'll need to decompress it.
    // The original size can be retrieved via METADATA_TYPE_ORIGINAL_SIZE metadata.
}
```
Additionally, you can verify the integrity of the entry's data using `drofs_verify_entry`:
```c
bool verified = drofs_verify_entry(&file_entry);
if (verified) {
    // Entry data CRC32 matches
}
```

## How to Open a "File" in the DROFS Filesystem

To "open" a file, you retrieve its `drofs_entry_t` structure using its full path. This structure provides access to its name, data, length, and metadata.

```c
#include <drofs.h>
#include "mock_test_data.h"

struct drofs_entry_t my_file_entry;
const char * file_path = "/subdir/file2.txt";

bool found = drofs_get_entry(mock_test_data, mock_test_data_len, file_path, &my_file_entry);

if (found && my_file_entry.type == ENTRY_TYPE_FILE) {
    // my_file_entry now contains all information about "file2.txt"
    // You can access:
    // my_file_entry.name (file name)
    // my_file_entry.data (pointer to file content)
    // my_file_entry.data_length (length of file content)
    // my_file_entry.flags (e.g., to check for compression)
    // ... and retrieve metadata as shown above.
} else {
    // File not found or is a directory
}
```

### DROFS C Library Usage with Compression

When a file entry is compressed, its `flags` field will have the `COMPRESSED` bit set (`file_entry.flags & COMPRESSED`). To decompress the data, you will need to use a decompression library (e.g., `miniz`). The original uncompressed size of the file can be retrieved from its metadata using `METADATA_TYPE_ORIGINAL_SIZE`.

```c
#include <drofs.h>
#include "mock_test_data.h" // Or your actual DROFS image data
#include "drofs_compression_helper.h"
#include <stdio.h> // For printf
#include <stdlib.h> // For malloc, free

// Assume 'drofs_image_data' and 'drofs_image_data_len' are available
// For example: const uint8_t *drofs_image_data = mock_test_data;
// size_t drofs_image_data_len = mock_test_data_len;

struct drofs_entry_t file_entry;
const char *compressed_file_path = "/path/to/your_compressed_file.txt"; // Replace with an actual path to a compressed file

bool found_file = drofs_get_entry(drofs_image_data, drofs_image_data_len, compressed_file_path, &file_entry);

if (found_file && file_entry.type == ENTRY_TYPE_FILE && (file_entry.flags & COMPRESSED)) {
    const uint8_t *compressed_data = file_entry.data;
    size_t compressed_length = file_entry.data_length;

    struct drofs_metadata_t original_size_metadata;
    bool got_size = drofs_get_type_metadata(&file_entry, METADATA_TYPE_ORIGINAL_SIZE, &original_size_metadata);

    if (got_size) {
        uint32_t original_size = *((uint32_t*)original_size_metadata.data);

        drofs_decompression_context_t *ctx = drofs_decompress_create(compressed_data, compressed_length);
        if (ctx == NULL) {
            printf("Failed to create decompression context.\n");
            // Handle error
        } else {
            uint8_t *decompressed_buffer = (uint8_t *)malloc(original_size);
            if (decompressed_buffer == NULL) {
                printf("Failed to allocate memory for decompressed data.\n");
                drofs_decompress_free(ctx);
                // Handle error
            } else {
                size_t total_decompressed_bytes = 0;
                tinfl_status status;

                while (total_decompressed_bytes < original_size) {
                    size_t chunk_size = original_size - total_decompressed_bytes;
                    if (chunk_size > 1024) chunk_size = 1024; // Decompress in chunks

                    size_t bytes_written = chunk_size;
                    status = drofs_decompress_chunk(ctx, decompressed_buffer + total_decompressed_bytes, &bytes_written);
                    total_decompressed_bytes += bytes_written;

                    if (status == TINFL_STATUS_DONE) {
                        break;
                    } else if (status < 0) {
                        printf("Decompression error: %d\n", status);
                        // Handle error
                        break;
                    }
                }

                if (total_decompressed_bytes == original_size) {
                    printf("Decompression successful. Original size: %lu bytes\n", (unsigned long)original_size);
                    // Use decompressed_buffer (e.g., print, process)
                } else {
                    printf("Decompression incomplete or size mismatch.\n");
                }

                free(decompressed_buffer);
            }
            drofs_decompress_free(ctx);
        }
    }
} else {
    printf("File not found, not a file, or not compressed.\n");
}
```

### DROFS C Library Printing the Timestamp to Buffer

To retrieve and print the timestamp associated with an entry, you can access the `METADATA_TYPE_TIMESTAMP` metadata. The timestamp is stored as a `uint32_t` representing seconds since the Unix epoch. You would typically convert this to a human-readable format using standard C library functions like `strftime` and `localtime_r`.

```c
#include <drofs.h>
#include "mock_test_data.h" // Or your actual DROFS image data
#include "drofs_timestamp_helper.h"
#include <stdio.h>  // For printf

// Assume 'drofs_image_data' and 'drofs_image_data_len' are available
// For example: const uint8_t *drofs_image_data = mock_test_data;
// size_t drofs_image_data_len = mock_test_data_len;

struct drofs_entry_t file_entry;
const char *file_path_with_timestamp = "/subdir/file2.txt"; // Replace with an actual path to a file with timestamp metadata

bool found_file = drofs_get_entry(drofs_image_data, drofs_image_data_len, file_path_with_timestamp, &file_entry);

if (found_file && file_entry.type == ENTRY_TYPE_FILE) {
    struct drofs_metadata_t timestamp_metadata;
    bool got_timestamp = drofs_get_type_metadata(&file_entry, METADATA_TYPE_TIMESTAMP, &timestamp_metadata);

    if (got_timestamp) {
        uint32_t timestamp_raw = *((uint32_t*)timestamp_metadata.data);

        char time_buffer[64]; // Buffer to hold the formatted timestamp string
        format_timestamp(timestamp_raw, time_buffer, sizeof(time_buffer));

        printf("Entry timestamp for '%s': %s\n", file_entry.name, time_buffer);
    } else {
        printf("Timestamp metadata not found for '%s'.\n", file_entry.name);
    }
} else {
    printf("File '%s' not found or is not a file.\n", file_path_with_timestamp);
}
```