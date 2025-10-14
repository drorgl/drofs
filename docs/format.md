# DROFS Image Format

The DROFS file format is designed for efficient storage and retrieval of a hierarchical file system structure. It consists of a header, an overall CRC32 checksum, a series of tree children entry records and TLV metadata records.

## Overall Structure

The binary file begins with:

*   **Header (5 bytes):** The ASCII string "DROFS".
*   **Overall CRC32 (4 bytes):** A CRC32 checksum of the entire linked list data that follows the header and its own CRC32. This ensures the integrity of the file system data.

Following the header and overall CRC32, the root entry of the file system is stored, which then recursively defines the entire structure.

## Entry Structure

Each entry within the DROFS file represents either a file or a directory and has the following structure:


*   **Entry Type (1 byte):**
    *   `0x01`: FILE
    *   `0x02`: DIRECTORY
*   **Name Length (NAME_LENGTH_BYTES):** An unsigned integer indicating the length of the entry's name in bytes (UTF-8 encoded).
*   **Name (variable length):** The UTF-8 encoded name of the entry. Its length is specified by the preceding "Name Length" field.
*   **Data Length (4 bytes):** An unsigned integer indicating the length of the entry's data in bytes. For directories, this will typically be 0.
*   **Data CRC32 (4 bytes):** A CRC32 checksum of the entry's data.
*   **Data (variable length):** The raw byte data of the entry. Its length is specified by the preceding "Data Length" field.
*   **Flags (1 byte):** A byte containing bit flags for various entry properties.
*       `0x01` (bit 0): `COMPRESSED` - Indicates if the data field is compressed.
*   **Metadata Length (1 byte):** A byte indicating the number of metadata entries in the entry
*   **Metadata Array(variable length):** An Array of metadata entries
* Metadata Entry
    * **Type (1 byte):** A byte indicating the type of metadata (original size = 1, timestamp = 2, original crc32 = 3)
    * **Length (2 byte):** A byte indicating the length of the metadata data
    * **Data (variable length):** An array of metadata entry data bytes
*   **Children Length (4 bytes):** An unsigned integer indicating the number of child entries this entry has. For files, this will be 0.
*   **Children Array (variable length):** An array of 4-byte unsigned integers. Each integer represents the absolute offset (from the beginning of the file) of a child entry within the DROFS file. The number of elements in this array is specified by "Children Length".
