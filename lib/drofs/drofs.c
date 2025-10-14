#include "drofs.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

// Constants for binary structure
#define ENTRY_TYPE_BYTES 1
#define NAME_LENGTH_BYTES 1
#define DATA_LENGTH_BYTES 4
#define DATA_CRC32_BYTES 4
#define FLAGS_BYTES 1
#define NUM_CHILDREN_BYTES 4
#define CHILD_OFFSET_BYTES 4

// File header constants
#define HEADER_BYTES 5
#define OVERALL_CRC32_BYTES 4
#define FILE_METADATA_SIZE (HEADER_BYTES + OVERALL_CRC32_BYTES)

#define CONCATENATE_INTERNAL(A, B) A ## B
#define CONCATENATE(A, B) CONCATENATE_INTERNAL(A, B)

// Define the basic types
#define UINT_BASE_1 uint8_t
#define UINT_BASE_2 uint16_t
#define UINT_BASE_4 uint32_t
#define UINT_BASE_8 uint64_t

// The main macro that combines 'UINT_BASE_' with the number N
#define UINT_TYPE(N) CONCATENATE(UINT_BASE_, N)

#define MAX(a, b) ((a) > (b) ? (a) : (b))

void drofs_print_entry(struct drofs_entry_t entry){
    printf("Entry(Type: %d, Name Length: %"PRIu32", Name: '%s', "
            "Data Length: %"PRIu32", Data CRC32 %"PRIx32", Flags: %d, "
            "Children Length: %"PRIu32", "
            "Offset: %"PRIu32")",
        entry.type,
        (uint32_t)entry.name_length,
        entry.name,
        (uint32_t)entry.data_length,
        entry.data_crc32,
        entry.flags,
        (uint32_t)entry.children_length,
        entry.offset
    );
    for (size_t i = 0; i < entry.children_length;i++){
        printf(" Child: %"PRIu32, entry.children_offsets[i]);
    }
    printf("\n");
}

bool drofs_verify(const uint8_t * data, size_t data_length){
    char * header_signature = (char*) &data[0];
    const char * expected_header_signature = "DROFS";
    if (strncmp(expected_header_signature, header_signature, HEADER_BYTES)!= 0){
        //print warning
        printf("different signature\n");
        return false;
    }

    uint32_t expected_crc32 = (uint32_t)(*((uint32_t*)&data[HEADER_BYTES]));

    crc32_context_t crc32_ctx;
    crc32_init(&crc32_ctx);
    crc32_update(&crc32_ctx,data + FILE_METADATA_SIZE, data_length - FILE_METADATA_SIZE);
    uint32_t data_crc = crc32_get(&crc32_ctx);
    if (data_crc != expected_crc32){
        //print warning
        printf("different crc, expected %" PRIx32" actual %" PRIx32" of %d\n", expected_crc32, data_crc,data_length - FILE_METADATA_SIZE);
        return false;
    }

    return true;
}

void _read_entry_at_offset(const uint8_t * data, size_t data_length, size_t offset, struct drofs_entry_t *entry){
    // printf("Reading Entry at 0x%p, length: %zu, offset %zu\n", data, data_length, offset);
    //     """Reads a full entry from the given offset and returns an Entry object."""
    //     # Ensure the file pointer is at the correct offset before reading
    //     print(f"Reading entry at {offset}")
    //     f.seek(offset)
    entry->offset = offset;
    //     entry_type_val = struct.unpack('B', f.read(ENTRY_TYPE_BYTES))[0]
    uint8_t entry_type_val = data[offset];
    //     entry_type = EntryType(entry_type_val)
    entry->type = entry_type_val;
    offset+=ENTRY_TYPE_BYTES;

    //     name_length = struct.unpack('I', f.read(NAME_LENGTH_BYTES).ljust(4, b'\0'))[0]
    size_t name_length_val = *(UINT_TYPE(NAME_LENGTH_BYTES)*)(&data[offset]);
    entry->name_length = name_length_val;
    offset+=NAME_LENGTH_BYTES;
    //     name_bytes_with_null = f.read(name_length)
    //     name = name_bytes_with_null.rstrip(b'\0').decode('ascii') # Decode as ASCII and remove null terminator
    entry->name = (char*)&data[offset];
    offset+= name_length_val;

    //     data_length = struct.unpack('I', f.read(DATA_LENGTH_BYTES))[0]
    size_t data_length_val = *(UINT_TYPE(DATA_LENGTH_BYTES)*)(&data[offset]);
    entry->data_length = data_length_val;
    offset+= DATA_LENGTH_BYTES;

    

    uint32_t stored_data_crc32 = *(UINT_TYPE(DATA_CRC32_BYTES)*)(&data[offset]);
    entry->data_crc32 = stored_data_crc32;
    offset+= DATA_CRC32_BYTES;

    entry->data = &data[offset];
    offset+= data_length_val;


    //     stored_data_crc32 = struct.unpack('I', f.read(DATA_CRC32_BYTES))[0]
    //     data = bytearray(f.read(data_length))
        
    //     calculated_data_crc32 = zlib.crc32(data)
    //     if stored_data_crc32 != calculated_data_crc32:
    //         raise ValueError(f"Data CRC32 checksum mismatch for entry '{name}'. Entry data may be corrupted.")
    uint8_t stored_flags = *(UINT_TYPE(FLAGS_BYTES)*)(&data[offset]);
    entry->flags = stored_flags;
    offset+= FLAGS_BYTES;

    // Read metadata
    entry->metadata_length = *(UINT_TYPE(1)*)(&data[offset]); // Number of metadata items (8-bit)
    offset += 1;
    entry->metadata_start_ptr = &data[offset]; // Store the pointer to the start of metadata block

    // Skip metadata data to get to children offsets
    for (size_t i = 0; i < entry->metadata_length; i++) {
        offset += 1; // Skip metadata type (8-bit)
        uint16_t metadata_length = *(UINT_TYPE(2)*)(&data[offset]); // Metadata length (16-bit)
        offset += 2;
        offset += metadata_length; // Skip metadata data
    }

    size_t stored_num_children = *(UINT_TYPE(NUM_CHILDREN_BYTES)*)(&data[offset]);
    entry->children_length = stored_num_children;
    offset+= NUM_CHILDREN_BYTES;
    entry->children_offsets = (UINT_TYPE(CHILD_OFFSET_BYTES)*)(&data[offset]);
}

bool drofs_get_type_metadata(struct drofs_entry_t * entry, uint8_t type, struct drofs_metadata_t * metadata){
    const uint8_t * current_metadata_ptr = entry->metadata_start_ptr;

    for (size_t i = 0; i < entry->metadata_length; i++) {
        uint8_t current_type = *current_metadata_ptr;
        current_metadata_ptr += 1; // Move past type

        uint16_t current_length = *(UINT_TYPE(2)*)(current_metadata_ptr);
        current_metadata_ptr += 2; // Move past length

        if (current_type == type) {
            metadata->type = current_type;
            metadata->length = current_length;
            metadata->data = current_metadata_ptr;
            // printf("type %d length %d data uint32_t %d\n", metadata->type, metadata->length, *(uint32_t*)metadata->data);
            return true;
        }
        
        current_metadata_ptr += current_length; // Move past data
    }
    return false;
}

bool drofs_get_nth_child(const uint8_t * data, size_t data_length, size_t nth_child, struct drofs_entry_t * entry, struct drofs_entry_t * child){
    assert(entry != NULL);
    assert(entry->children_length > nth_child);
    size_t index = FILE_METADATA_SIZE;
    _read_entry_at_offset(data + index , data_length - index, entry->children_offsets[nth_child], child);
    return true;
}

bool drofs_verify_entry(struct drofs_entry_t * entry){
    crc32_context_t crc32_ctx;
    crc32_init(&crc32_ctx);
    crc32_update(&crc32_ctx,entry->data, entry->data_length);
    uint32_t data_crc = crc32_get(&crc32_ctx);
    if (data_crc != entry->data_crc32){
        //print warning
        printf("different crc, expected %" PRIx32" actual %" PRIx32" of %d\n",  entry->data_crc32, data_crc,entry->data_length);
        return false;
    }

    return true;
}

bool drofs_get_entry(const uint8_t * data, size_t data_length, const char * path,struct drofs_entry_t * entry ){
    //  # Reset file pointer to the beginning of the linked list data (after header and CRC)
//             f.seek(FILE_METADATA_SIZE)
    size_t index = FILE_METADATA_SIZE;
            
//             root_entry_from_file = self._read_entry_at_offset(f, f.tell())

    struct drofs_entry_t entry_part;
    _read_entry_at_offset(data + index, data_length - index, 0, &entry_part);
    if (entry_part.type < 1 || entry_part.type > 2){
        return false;
    }
    // drofs_print_entry(entry_part);

    // printf("done printing\n");

    char pathCopy[256]; // Adjust size as necessary
    strncpy(pathCopy, path, sizeof(pathCopy));
    pathCopy[sizeof(pathCopy) - 1] = '\0'; // Ensure null-termination

    // Tokenize the path using '/' as the delimiter
    char *part = strtok(pathCopy, "/");
    int partNumber = 1;

    bool found = true;

    while (part != NULL) {
        // printf("Part %d: %s\n", partNumber++, part);
        
        struct drofs_entry_t possible_part;
        for (size_t i = 0; i < entry_part.children_length;i++){
            // printf("Checking child %d\n", i);
            _read_entry_at_offset(data + index , data_length - index, entry_part.children_offsets[i], &possible_part);
            if (possible_part.type < 1 || possible_part.type > 2){
                return false;
            }
            // drofs_print_entry(possible_part);
            // printf("comparing\n");
            // printf("comparing %s with %s\n", possible_part.name, part);
            if (strcmp(possible_part.name, part) == 0){//, MAX(possible_part.name_length, strnlen(part,255))) == 0){
                // printf("next part found\n");
                entry_part = possible_part;
                found = true;
                break;
            }
            found = false;
        }
        part = strtok(NULL, "/");
    }

    // printf("found?\n");
    *entry = entry_part;
    return found;
}
