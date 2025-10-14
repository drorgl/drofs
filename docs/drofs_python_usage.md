# DROFS Python Usage

This document describes how to use the `drofs` Python library to create, serialize, and deserialize DROFS (Dror Read-Only File System) archives.

## Overview

The `drofs` library provides classes and methods to manage a hierarchical file system structure in a single binary file. It supports directories, files, data compression, and metadata for entries.

In addition the [drofs_cli](drofs_cli.md) can perform creation and verification of images in the command line.

## Core Classes

### `EntryType` Enum

An enumeration defining the type of an entry in the DROFS.

- `FILE`: Represents a file.
- `DIRECTORY`: Represents a directory.

### `EntryFlags` Enum

An enumeration defining flags that can be associated with an entry.

- `COMPRESSED`: Indicates that the entry's data is compressed (value: `1 << 0` or `0x01`).

### `EntryMetadataType` Enum

An enumeration defining the type of metadata that can be stored for an entry.

- `ORIGINAL_SIZE`: The original size of the data before compression.
- `TIMESTAMP`: The creation or modification timestamp of the entry.
- `ORIGINAL_CRC32`: The CRC32 checksum of the original data before compression.

### `EntryMetadata` Class

Represents a single metadata item associated with an `Entry`.

#### Constructor

`EntryMetadata(metadata_type: EntryMetadataType, data: bytes)`

- `metadata_type`: The type of metadata (e.g., `EntryMetadataType.ORIGINAL_SIZE`).
- `data`: The raw byte data of the metadata.

#### Attributes

- `type`: The `EntryMetadataType` of the metadata.
- `length`: The length of the `data` in bytes.
- `data`: The raw byte data of the metadata.

### `Entry` Class

Represents a file or directory within the DROFS structure. These entries form a linked list (tree) structure.

#### Constructor

`Entry(entry_type: EntryType, name: str, data: bytearray = None, children: list = None, flags: int = 0, metadata: List[EntryMetadata] = None)`

- `entry_type`: The type of the entry (`EntryType.FILE` or `EntryType.DIRECTORY`).
- `name`: The name of the file or directory.
- `data`: (Optional) A `bytearray` containing the content of the file. For directories, this is typically empty.
- `children`: (Optional) A list of `Entry` objects (for directories) or child offsets (during deserialization).
- `flags`: (Optional) An integer representing a bitmask of `EntryFlags`.
- `metadata`: (Optional) A list of `EntryMetadata` objects.

#### Attributes

- `type`: The `EntryType` of the entry.
- `name`: The name of the entry.
- `data`: The `bytearray` content of the entry.
- `children`: A list of child `Entry` objects (for directories).
- `flags`: An integer representing the combined `EntryFlags`.
- `offset`: The byte offset of the entry within the serialized DROFS file (set during serialization).
- `metadata`: A list of `EntryMetadata` objects associated with the entry.

#### Methods

- `get_metadata_by_type(metadata_type: EntryMetadataType) -> EntryMetadata | None`:
  Retrieves an `EntryMetadata` object of a specific type from the entry's metadata list. Returns `None` if not found.

### `Drofs` Class

The main class for interacting with DROFS archives. It handles serialization (writing to a binary file) and deserialization (reading from a binary file).

#### Constructor

`Drofs(file_path: str)`

- `file_path`: The path to the DROFS binary file.

#### Attributes

- `file_path`: The path to the DROFS binary file.
- `root`: The root `Entry` of the DROFS tree.

#### Methods

- `serialize()`:
  Serializes the `root` entry and its children into the binary file specified by `file_path`. This method calculates an overall CRC32 checksum for the linked list data and writes it along with a "DROFS" header.

- `deserialize(path: str) -> Entry | None`:
  Deserializes the DROFS archive from `file_path` and retrieves a specific entry by its path (e.g., "/dir1/file.txt"). It verifies the overall CRC32 checksum before proceeding. Returns the `Entry` object if found, otherwise `None`.

- `deserialize_root() -> Entry | None`:
  Deserializes the entire DROFS archive from `file_path` and reconstructs the full `Entry` tree, starting from the root. It verifies the overall CRC32 checksum. Returns the root `Entry` object if successful, otherwise `None`.

## Example Usage

### Creating a DROFS Archive

```python
import os
import zlib
from drofs import Drofs, Entry, EntryFlags, EntryMetadata, EntryMetadataType, EntryType

# Define the output archive path
archive_path = "my_archive.bin"

# 1. Create Entry objects to represent your file system structure
root_dir = Entry(EntryType.DIRECTORY, "root")

# Add a directory
dir1 = Entry(EntryType.DIRECTORY, "my_documents")
root_dir.children.append(dir1)

# Add a file to dir1
file1_data = b"This is the content of my first file."
file1 = Entry(EntryType.FILE, "report.txt", data=bytearray(file1_data))
dir1.children.append(file1)

# Add another directory with a compressed file
dir2 = Entry(EntryType.DIRECTORY, "compressed_data")
root_dir.children.append(dir2)

# Simulate compression
original_file_data = b"A much longer piece of text that we want to compress to save space in the archive." * 10
compressed_file_data = zlib.compress(original_file_data, level=9)

# Add metadata for original size and CRC32 if compressed
metadata_list = [
    EntryMetadata(EntryMetadataType.ORIGINAL_SIZE, len(original_file_data).to_bytes(4, 'little')),
    EntryMetadata(EntryMetadataType.ORIGINAL_CRC32, zlib.crc32(original_file_data).to_bytes(4, 'little'))
]

compressed_file = Entry(
    EntryType.FILE,
    "large_text.z",
    data=bytearray(compressed_file_data),
    flags=EntryFlags.COMPRESSED.value,
    metadata=metadata_list
)
dir2.children.append(compressed_file)

# 2. Create a Drofs instance and set its root
drofs_instance = Drofs(archive_path)
drofs_instance.root = root_dir

# 3. Serialize the DROFS tree to the binary file
drofs_instance.serialize()
print(f"DROFS archive created at {archive_path}")

# Clean up the created file (optional)
# if os.path.exists(archive_path):
#     os.remove(archive_path)
```

### Reading from a DROFS Archive

```python
import os
import zlib
from drofs import Drofs, EntryFlags, EntryMetadataType, EntryType

archive_path = "my_archive.bin" # Assuming 'my_archive.bin' was created as above

# 1. Create a Drofs instance
drofs_instance = Drofs(archive_path)

# 2. Deserialize the root entry to get the full tree
try:
    root_entry = drofs_instance.deserialize_root()
    if root_entry:
        print(f"\nDeserialized Root Entry: {root_entry.name} (Type: {root_entry.type.name})")

        # Function to recursively print the tree
        def print_drofs_tree(entry: Entry, indent=0):
            prefix = "  " * indent
            print(f"{prefix}- {entry.name} (Type: {entry.type.name}, Flags: {entry.flags})")
            if entry.type == EntryType.FILE:
                # Access file data
                file_data = entry.data
                if entry.flags & EntryFlags.COMPRESSED.value:
                    print(f"{prefix}  (Compressed data length: {len(file_data)} bytes)")
                    # Decompress data if flagged
                    decompressed_data = zlib.decompress(file_data)
                    print(f"{prefix}  (Decompressed data length: {len(decompressed_data)} bytes)")
                    # Retrieve original size metadata
                    original_size_meta = entry.get_metadata_by_type(EntryMetadataType.ORIGINAL_SIZE)
                    if original_size_meta:
                        original_size = int.from_bytes(original_size_meta.data, 'little')
                        print(f"{prefix}  (Original size from metadata: {original_size} bytes)")
                    # print(f"{prefix}  Content: {decompressed_data.decode('utf-8', errors='ignore')[:50]}...") # Print first 50 chars
                else:
                    print(f"{prefix}  (Data length: {len(file_data)} bytes)")
                    # print(f"{prefix}  Content: {file_data.decode('utf-8', errors='ignore')[:50]}...") # Print first 50 chars

                # Access other metadata
                timestamp_meta = entry.get_metadata_by_type(EntryMetadataType.TIMESTAMP)
                if timestamp_meta:
                    timestamp = int.from_bytes(timestamp_meta.data, 'little')
                    print(f"{prefix}  (Timestamp: {timestamp})")

            for child in entry.children:
                print_drofs_tree(child, indent + 1)

        print_drofs_tree(root_entry)

        # 3. Alternatively, retrieve a specific entry by path
        specific_file_entry = drofs_instance.deserialize("/my_documents/report.txt")
        if specific_file_entry:
            print(f"\nRetrieved specific file: {specific_file_entry.name}")
            print(f"Content: {specific_file_entry.data.decode('utf-8')}")

        non_existent_entry = drofs_instance.deserialize("/non_existent_path/file.txt")
        if non_existent_entry is None:
            print("\nAttempted to retrieve a non-existent path, as expected.")

    else:
        print("Failed to deserialize root entry.")

except ValueError as e:
    print(f"Error deserializing DROFS archive: {e}")

# Clean up the created file (optional)
# if os.path.exists(archive_path):
#     os.remove(archive_path)
