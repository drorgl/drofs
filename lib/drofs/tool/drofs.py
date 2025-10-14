import io
import struct
import zlib
from enum import Enum
from typing import List

# Constants for binary structure
ENTRY_TYPE_BYTES = 1
NAME_LENGTH_BYTES = 1
DATA_LENGTH_BYTES = 4
DATA_CRC32_BYTES = 4
FLAGS_BYTES = 1
NUM_CHILDREN_BYTES = 4
CHILD_OFFSET_BYTES = 4

# File header constants
HEADER_BYTES = 5
OVERALL_CRC32_BYTES = 4
FILE_METADATA_SIZE = HEADER_BYTES + OVERALL_CRC32_BYTES # Total size of header + overall CRC32

class EntryType(Enum):
    FILE = 1
    DIRECTORY = 2

class EntryFlags(Enum):
    COMPRESSED = 1 << 0 # 0x01

class EntryMetadataType(Enum):
    ORIGINAL_SIZE = 1
    TIMESTAMP = 2
    ORIGINAL_CRC32 = 3

class EntryMetadata:
    def __init__(self, metadata_type: EntryMetadataType, data: bytes):
        self.type = metadata_type
        self.length = len(data)
        self.data = data

    def __str__(self):
        return (f"EntryMetadata(Type: {self.type.name}, Length: {self.length}, "
                f"Data: {self.data.hex()})")

class Entry:
    def __init__(self, entry_type: EntryType, name: str, data: bytearray = None, children: list = None, flags: int = 0, metadata: List[EntryMetadata] = None):
        self.type = entry_type
        self.name = name
        self.data = data if data is not None else bytearray()
        self.children : List[Entry] = children if children is not None else []
        self.flags = flags # flags attribute
        self.offset = -1 # To store the offset in the file when serialized
        self.metadata: List[EntryMetadata] = metadata if metadata is not None else []

    def __str__(self):
        for index, child in enumerate(self.children):
            if not isinstance(child, Entry):
                print(f"Warning: Child at index {index} ('{child}') is not an instance of Entry.")

        metadata_str = ", ".join([str(m) for m in self.metadata])
        children_names = [child.name for child in self.children if isinstance(child, Entry)]
        return (f"Entry(Type: {self.type.name}, Name: '{self.name}', "
                f"Data Length: {len(self.data)}, Flags: {self.flags}, "
                f"Metadata: [{metadata_str}], Children Length: {len(self.children)}, "
                f"Children: {children_names}, Offset: {self.offset})")

    def get_metadata_by_type(self, metadata_type: EntryMetadataType) -> EntryMetadata | None:
        for metadata_item in self.metadata:
            if metadata_item.type == metadata_type:
                return metadata_item
        return None

class Drofs:
    def __init__(self, file_path: str):
        self.file_path = file_path
        self.root = None # The root entry of the linked list

    def serialize(self):
        """Serializes the linked list to the binary file."""
        # Serialize the linked list into a BytesIO buffer first to calculate CRC32
        buffer = io.BytesIO()
        self._write_recursive(buffer, self.root)
        linked_list_bytes = buffer.getvalue()

        # Calculate CRC32
        # print(F'scanning crc of {len(linked_list_bytes)}')
        crc32_value = zlib.crc32(linked_list_bytes)
        # print(F"crc {crc32_value:#010x}")

        with open(self.file_path, 'wb') as f:
            # Write the file header
            f.write(b"DROFS")
            # Write CRC32
            f.write(struct.pack('I', crc32_value))
            # Write the actual linked list data
            f.write(linked_list_bytes)

    def _write_recursive(self, f : io.BytesIO, entry: Entry):
        if not entry:
            return

        # Store current position as the entry's offset
        entry.offset = f.tell()
        # print(f"Writing {entry} at {entry.offset}")

        # Write entry type
        f.write(struct.pack('B', entry.type.value))

        # Write name length and name (ASCII, null-terminated)
        name_bytes = entry.name.encode('ascii') + b'\0' # Add null terminator
        f.write(struct.pack('I', len(name_bytes))[:NAME_LENGTH_BYTES]) # Length includes null terminator
        f.write(name_bytes)

        # Write data length, data CRC32, and data
        data_bytes = entry.data
        data_length = len(data_bytes)
        data_crc32_value = zlib.crc32(data_bytes)

        f.write(struct.pack('I', data_length))
        f.write(struct.pack('I', data_crc32_value))
        f.write(data_bytes)

        # Write flags
        f.write(struct.pack('B', entry.flags))

        # Write metadata
        f.write(struct.pack('B', len(entry.metadata))) # Number of metadata items
        for metadata_item in entry.metadata:
            f.write(struct.pack('B', metadata_item.type.value)) # Metadata type (8-bit)
            f.write(struct.pack('H', metadata_item.length)) # Metadata length (16-bit)
            f.write(metadata_item.data) # Metadata data

        # Placeholder for number of children and children offsets
        # We will come back and fill this after all children are written
        num_children_pos = f.tell()
        f.write(struct.pack('I', 0)) # Placeholder for num_children

        # Store current children offsets for later update
        children_offsets_start_pos = f.tell()
        # Placeholder for children offsets
        for _ in entry.children:
            f.write(struct.pack('I', 0)) # Placeholder for child offset

        # Recursively write children
        actual_children_offsets = []
        for child in entry.children:
            self._write_recursive(f, child)
            actual_children_offsets.append(child.offset)

        # Go back and update num_children and children_offsets
        current_pos = f.tell()
        f.seek(num_children_pos)
        f.write(struct.pack('I', len(actual_children_offsets)))

        f.seek(children_offsets_start_pos)
        for offset in actual_children_offsets:
            f.write(struct.pack('I', offset))

        f.seek(current_pos) # Return to current position

    def deserialize(self, path: str):
        """Deserializes the linked list from the binary file and retrieves an entry by path."""
        with open(self.file_path, 'rb') as f:
            # Read and verify the file header
            header = f.read(HEADER_BYTES)
            if header != b"DROFS":
                raise ValueError("Invalid DROFS file header.")

            # Read stored CRC32
            stored_crc32 = struct.unpack('I', f.read(OVERALL_CRC32_BYTES))[0]

            # Read the rest of the file content for CRC32 calculation
            linked_list_bytes = f.read()
            calculated_crc32 = zlib.crc32(linked_list_bytes)

            if stored_crc32 != calculated_crc32:
                raise ValueError("CRC32 checksum mismatch. File may be corrupted.")

            # Reset file pointer to the beginning of the linked list data (after header and CRC)
            f.seek(FILE_METADATA_SIZE)

            root_entry_from_file = self._read_entry_at_offset(f, f.tell())

            path_components = [comp for comp in path.split('/') if comp]
            current_entry = root_entry_from_file

            for component in path_components:
                found_child = None
                # Base offset for linked list data in the file (after header and CRC)
                base_offset_for_linked_list = FILE_METADATA_SIZE
                for child_offset_relative in current_entry.children: # children now stores offsets (relative to linked_list_bytes)
                    absolute_child_offset = base_offset_for_linked_list + child_offset_relative
                    f.seek(absolute_child_offset)
                    child_entry = self._read_entry_metadata(f) # Read only metadata to check name
                    if child_entry.name == component:
                        found_child = self._read_entry_at_offset(f, absolute_child_offset) # Read full entry
                        break

                if found_child:
                    current_entry = found_child
                else:
                    return None # Path component not found

                return current_entry

    def deserialize_root(self):
        """Deserializes the root entry from the binary file."""
        with open(self.file_path, 'rb') as f:
            # Read and verify the file header
            header = f.read(HEADER_BYTES)
            if header != b"DROFS":
                raise ValueError("Invalid DROFS file header.")

            # Read stored CRC32
            stored_crc32 = struct.unpack('I', f.read(OVERALL_CRC32_BYTES))[0]

            # Read the rest of the file content for CRC32 calculation
            linked_list_bytes = f.read()
            calculated_crc32 = zlib.crc32(linked_list_bytes)

            if stored_crc32 != calculated_crc32:
                raise ValueError("CRC32 checksum mismatch. File may be corrupted.")

            # Reset file pointer to the beginning of the linked list data (after header and CRC)
            f.seek(FILE_METADATA_SIZE)

            root_entry_from_file = self._read_entry_at_offset(f, f.tell())

            # Recursively replace children with entries read at their offsets
            self._recursively_read_children(f, root_entry_from_file, FILE_METADATA_SIZE)

            return root_entry_from_file

    def _recursively_read_children(self, f, entry: Entry, offset: int):
        """Recursively reads children entries based on their offsets and replaces them in the entry's children list."""
        original_children_offsets = list(entry.children) # Make a copy as we'll modify the list
        entry.children = [] # Clear the list to populate with actual Entry objects

        for child_offset in original_children_offsets:
            child_entry = self._read_entry_at_offset(f, child_offset + offset)
            entry.children.append(child_entry)
            if child_entry.type == EntryType.DIRECTORY:
                self._recursively_read_children(f, child_entry, offset)


    def _read_entry_metadata(self, f):
        """Reads only the metadata (type, name, offset) of an entry at the current file position."""
        start_offset = f.tell()

        entry_type_val = struct.unpack('B', f.read(ENTRY_TYPE_BYTES))[0]
        entry_type = EntryType(entry_type_val)

        name_length = struct.unpack('I', f.read(NAME_LENGTH_BYTES).ljust(4, b'\0'))[0]
        name_bytes_with_null = f.read(name_length)
        name = name_bytes_with_null.rstrip(b'\0').decode('ascii') # Decode as ASCII and remove null terminator

        # Skip data length, data CRC32, data, and children info for metadata read
        data_length = struct.unpack('I', f.read(DATA_LENGTH_BYTES))[0]
        f.seek(DATA_CRC32_BYTES, 1) # Skip data CRC32
        f.seek(data_length, 1) # Seek relative to current position

        f.seek(FLAGS_BYTES, 1) # Skip flags

        # Skip metadata for metadata read
        num_metadata = struct.unpack('B', f.read(1))[0]
        for _ in range(num_metadata):
            f.seek(1, 1) # Skip metadata type
            metadata_length = struct.unpack('H', f.read(2))[0] # Read metadata length
            f.seek(metadata_length, 1) # Skip metadata data

        num_children = struct.unpack('I', f.read(NUM_CHILDREN_BYTES))[0]
        f.seek(num_children * CHILD_OFFSET_BYTES, 1) # Seek relative to current position

        entry = Entry(entry_type, name)
        entry.offset = start_offset
        return entry

    def _read_entry_at_offset(self, f, offset: int):
        """Reads a full entry from the given offset and returns an Entry object."""
        # Ensure the file pointer is at the correct offset before reading
        print(f"Reading entry at {offset}")
        f.seek(offset)

        entry_type_val = struct.unpack('B', f.read(ENTRY_TYPE_BYTES))[0]
        entry_type = EntryType(entry_type_val)

        name_length = struct.unpack('I', f.read(NAME_LENGTH_BYTES).ljust(4, b'\0'))[0]
        name_bytes_with_null = f.read(name_length)
        name = name_bytes_with_null.rstrip(b'\0').decode('ascii') # Decode as ASCII and remove null terminator

        data_length = struct.unpack('I', f.read(DATA_LENGTH_BYTES))[0]
        stored_data_crc32 = struct.unpack('I', f.read(DATA_CRC32_BYTES))[0]
        data = bytearray(f.read(data_length))

        calculated_data_crc32 = zlib.crc32(data)
        if stored_data_crc32 != calculated_data_crc32:
            raise ValueError(f"Data CRC32 checksum mismatch for entry '{name}'. Entry data may be corrupted.")

        flags = struct.unpack('B', f.read(FLAGS_BYTES))[0]

        # Read metadata
        metadata_list = []
        num_metadata = struct.unpack('B', f.read(1))[0]
        for _ in range(num_metadata):
            metadata_type_val = struct.unpack('B', f.read(1))[0]
            metadata_type = EntryMetadataType(metadata_type_val)
            metadata_length = struct.unpack('H', f.read(2))[0]
            metadata_data = f.read(metadata_length)
            metadata_list.append(EntryMetadata(metadata_type, metadata_data))

        num_children = struct.unpack('I', f.read(NUM_CHILDREN_BYTES))[0]
        children_offsets = []
        for _ in range(num_children):
            children_offsets.append(struct.unpack('I', f.read(CHILD_OFFSET_BYTES))[0])

        entry = Entry(entry_type, name, data, children_offsets, flags, metadata_list)
        entry.offset = offset
        return entry
