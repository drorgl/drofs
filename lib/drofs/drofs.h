#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <inttypes.h>

#include <crc32.h>

/**
 * @brief Enumeration for the type of a DROFS entry.
 */
enum drofs_entry_type{
    ENTRY_TYPE_FILE = 1, /**< Represents a file entry. */
    ENTRY_TYPE_DIRECTORY = 2 /**< Represents a directory entry. */
};

/**
 * @brief Enumeration for flags associated with a DROFS entry.
 */
enum drofs_entry_flags{
    COMPRESSED = 1 << 0 /**< Flag indicating if the entry data is compressed. */
};

/**
 * @brief Enumeration for the type of metadata associated with a DROFS entry.
 */
enum drofs_entry_metadata_type{
    METADATA_TYPE_ORIGINAL_SIZE = 1, /**< Metadata type for the original size of a file. */
    METADATA_TYPE_TIMESTAMP = 2, /**< Metadata type for the timestamp of an entry. */
    METADATA_TYPE_ORIGINAL_CRC32 = 3 /**< Metadata type for the original crc32 of a file. */
};

/**
 * @brief Structure to represent a DROFS metadata item.
 */
struct drofs_metadata_t{
    uint8_t type; /**< The type of metadata (e.g., METADATA_TYPE_ORIGINAL_SIZE). */
    uint16_t length; /**< The length of the metadata data in bytes. */
    const uint8_t * data; /**< Pointer to the raw metadata data. */
};

/**
 * @brief Structure to represent a DROFS entry (file or directory).
 */
struct drofs_entry_t{
    enum drofs_entry_type type; /**< The type of the entry (file or directory). */
    const char * name; /**< Pointer to the name of the entry. */
    size_t name_length; /**< The length of the entry's name. */
    const uint8_t * data; /**< Pointer to the raw data of the entry (file content or directory metadata). */
    size_t data_length; /**< The length of the entry's data in bytes. */
    uint32_t data_crc32; /**< CRC32 checksum of the entry's data. */
    uint8_t flags; /**< Flags associated with the entry (e.g., COMPRESSED). */
    uint8_t metadata_length; /**< Number of metadata items associated with this entry. */
    const uint8_t * metadata_start_ptr; /**< Pointer to the start of the metadata block in the raw DROFS data. */
    uint32_t offset; /**< The offset of this entry within the DROFS image. */
    uint32_t * children_offsets; /**< Array of offsets to child entries (for directories). */
    size_t children_length; /**< The number of child entries (for directories). */
};

/**
 * @brief Prints the details of a DROFS entry to standard output.
 * @param entry The drofs_entry_t structure to print.
 */
void drofs_print_entry(struct drofs_entry_t entry);

/**
 * @brief Verifies the integrity of a DROFS image.
 * @param data Pointer to the raw DROFS image data.
 * @param data_length The total length of the DROFS image data.
 * @return True if the DROFS image is valid, false otherwise.
 */
bool drofs_verify(const uint8_t * data, size_t data_length);

/**
 * @brief Retrieves the Nth metadata item of a specific type from a DROFS entry.
 * @param entry Pointer to the drofs_entry_t structure.
 * @param type The type of metadata to retrieve.
 * @param metadata Pointer to a drofs_metadata_t structure to populate with the retrieved metadata.
 * @return True if the metadata item was found, false otherwise.
 */
bool drofs_get_type_metadata(struct drofs_entry_t * entry, uint8_t type, struct drofs_metadata_t * metadata);

/**
 * @brief Retrieves the Nth child entry of a directory entry.
 * @param data Pointer to the raw DROFS image data.
 * @param data_length The total length of the DROFS image data.
 * @param nth_child The index of the child entry to retrieve.
 * @param entry Pointer to the parent drofs_entry_t (must be a directory).
 * @param child Pointer to a drofs_entry_t structure to populate with the child's details.
 * @return True if the child entry was successfully retrieved, false otherwise.
 */
bool drofs_get_nth_child(const uint8_t * data, size_t data_length, size_t nth_child, struct drofs_entry_t * entry, struct drofs_entry_t * child);

/**
 * @brief Verifies the CRC32 checksum of an individual DROFS entry's data.
 * @param entry Pointer to the drofs_entry_t structure to verify.
 * @return True if the entry's data CRC32 checksum is valid, false otherwise.
 */
bool drofs_verify_entry(struct drofs_entry_t * entry);

/**
 * @brief Retrieves a DROFS entry by its path.
 * @param data Pointer to the raw DROFS image data.
 * @param data_length The total length of the DROFS image data.
 * @param path The path to the desired entry (e.g., "dir1/file.txt").
 * @param entry Pointer to a drofs_entry_t structure to populate with the found entry's details.
 * @return True if the entry was found, false otherwise.
 */
bool drofs_get_entry(const uint8_t * data, size_t data_length, const char * path,struct drofs_entry_t * entry );
