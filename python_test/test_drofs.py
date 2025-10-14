import io  # Required for BytesIO in test_corrupted_entry_data_fails
import os
import struct
import zlib  # Required for zlib.crc32 in test_corrupted_entry_data_fails

import pytest

# Add the path to the drofs library to sys.path
# sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'lib', 'drofs', 'tool')))
from drofs import NAME_LENGTH_BYTES, Drofs, Entry, EntryType


@pytest.fixture
def drofs_setup_teardown():
    file_system_path = "test_filesystem.bin"
    drofs_instance = Drofs(file_system_path)

    # Create an Entry tree
    root_dir = Entry(EntryType.DIRECTORY, "root")

    dir1 = Entry(EntryType.DIRECTORY, "dir1")
    file1 = Entry(EntryType.FILE, "file1.txt", data=bytearray(b"Hello from file1"))
    dir1.children.append(file1)

    dir2 = Entry(EntryType.DIRECTORY, "dir2")
    file2 = Entry(EntryType.FILE, "file2.txt", data=bytearray(b"Content of file2"))
    subdir1 = Entry(EntryType.DIRECTORY, "subdir1")
    file3 = Entry(EntryType.FILE, "file3.log", data=bytearray(b"Log data"))
    subdir1.children.append(file3)
    dir2.children.append(file2)
    dir2.children.append(subdir1)

    root_dir.children.append(dir1)
    root_dir.children.append(dir2)

    drofs_instance.root = root_dir
    drofs_instance.serialize()

    yield drofs_instance # Provide the drofs_instance to the tests

    # Teardown: Clean up the test file
    if os.path.exists(file_system_path):
        os.remove(file_system_path)

def test_retrieve_root(drofs_setup_teardown):
    drofs_instance = drofs_setup_teardown
    retrieved_root = drofs_instance.deserialize("/")
    assert retrieved_root is not None
    assert retrieved_root.name == "root"
    assert retrieved_root.type == EntryType.DIRECTORY

def test_retrieve_dir1(drofs_setup_teardown):
    drofs_instance = drofs_setup_teardown
    retrieved_dir1 = drofs_instance.deserialize("/dir1")
    assert retrieved_dir1 is not None
    assert retrieved_dir1.name == "dir1"
    assert retrieved_dir1.type == EntryType.DIRECTORY

def test_retrieve_file1(drofs_setup_teardown):
    drofs_instance = drofs_setup_teardown
    retrieved_file1 = drofs_instance.deserialize("/dir1/file1.txt")
    assert retrieved_file1 is not None
    assert retrieved_file1.name == "file1.txt"
    assert retrieved_file1.type == EntryType.FILE
    assert retrieved_file1.data == bytearray(b"Hello from file1")

def test_retrieve_file2(drofs_setup_teardown):
    drofs_instance = drofs_setup_teardown
    retrieved_file2 = drofs_instance.deserialize("/dir2/file2.txt")
    assert retrieved_file2 is not None
    assert retrieved_file2.name == "file2.txt"
    assert retrieved_file2.type == EntryType.FILE
    assert retrieved_file2.data == bytearray(b"Content of file2")

def test_retrieve_file3(drofs_setup_teardown):
    drofs_instance = drofs_setup_teardown
    retrieved_file3 = drofs_instance.deserialize("/dir2/subdir1/file3.log")
    assert retrieved_file3 is not None
    assert retrieved_file3.name == "file3.log"
    assert retrieved_file3.type == EntryType.FILE
    assert retrieved_file3.data == bytearray(b"Log data")

def test_non_existent_path(drofs_setup_teardown):
    drofs_instance = drofs_setup_teardown
    non_existent = drofs_instance.deserialize("/dir1/non_existent.txt")
    assert non_existent is None

def test_invalid_header_fails():
    invalid_file_path = "invalid_filesystem.bin"
    with open(invalid_file_path, 'wb') as f:
        f.write(b"BADHD") # Write an invalid header
        f.write(b"some data") # Some dummy data

    drofs_instance = Drofs(invalid_file_path)
    with pytest.raises(ValueError, match="Invalid DROFS file header."):
        drofs_instance.deserialize("/")

    if os.path.exists(invalid_file_path):
        os.remove(invalid_file_path)

def test_bad_crc_fails(drofs_setup_teardown):
    # Use the file created by the fixture, then corrupt its CRC
    file_system_path = drofs_setup_teardown.file_path

    # Manually corrupt the CRC32 value in the file
    with open(file_system_path, 'r+b') as f:
        f.seek(5) # After "DROFS" header
        f.write(struct.pack('I', 0xBADBEEF)) # Write a bad CRC value

    # Attempt to deserialize from the corrupted file
    corrupted_drofs_instance = Drofs(file_system_path)
    with pytest.raises(ValueError, match="CRC32 checksum mismatch. File may be corrupted."):
        corrupted_drofs_instance.deserialize("/")

def test_corrupted_entry_data_fails(drofs_setup_teardown):
    file_system_path = drofs_setup_teardown.file_path
    drofs_instance = drofs_setup_teardown

    # Get the entry for file1.txt to find its offset
    file1_entry = drofs_instance.deserialize("/dir1/file1.txt")
    assert file1_entry is not None

    # Calculate the absolute offset to the data_length field of file1.txt
    # Entry structure: type (1), name_length (4), name (var), data_length (4), data_crc32 (4), data (var),
    #   num_children (4), children_offsets (var)
    # We need to seek to the start of the data itself, which is after data_length and data_crc32

    # Base offset for linked list data in the file (after header and CRC)
    # Using FILE_METADATA_SIZE from drofs.py
    from drofs import FILE_METADATA_SIZE
    base_offset_for_linked_list = FILE_METADATA_SIZE

    # Read the entire file content
    with open(file_system_path, 'rb') as f:
        f.read(5) # Skip header
        # original_overall_crc32 = struct.unpack('I', f.read(4))[0]
        linked_list_bytes = bytearray(f.read())

    # Find the relative offset of file1.txt's data_crc32 within linked_list_bytes
    # This requires re-parsing the entry structure within the bytes

    # Simulate reading the entry to find the data_crc32 position within linked_list_bytes
    temp_buffer = io.BytesIO(linked_list_bytes)
    # Seek to entry start relative to linked_list_bytes
    temp_buffer.seek(file1_entry.offset - base_offset_for_linked_list)
    temp_buffer.read(1) # Skip type
    name_length = struct.unpack('I', temp_buffer.read(NAME_LENGTH_BYTES).ljust(4, b'\0'))[0]
    temp_buffer.read(name_length) # Skip name
    temp_buffer.read(4) # Skip data_length

    data_crc32_relative_pos_in_linked_list_bytes = temp_buffer.tell() # Position of data_crc32 within linked_list_bytes

    # Corrupt the data_crc32 value within linked_list_bytes
    corrupted_crc_bytes = struct.pack('I', 0xBADBEEF)
    linked_list_bytes[data_crc32_relative_pos_in_linked_list_bytes : data_crc32_relative_pos_in_linked_list_bytes + 4] = corrupted_crc_bytes

    # Recalculate the overall CRC32 for the modified linked_list_bytes
    new_overall_crc32 = zlib.crc32(linked_list_bytes)

    # Write the file back with the original header, new overall CRC32, and modified linked_list_bytes
    with open(file_system_path, 'wb') as f:
        f.write(b"DROFS")
        f.write(struct.pack('I', new_overall_crc32))
        f.write(linked_list_bytes)

    # Attempt to deserialize the corrupted entry
    corrupted_drofs_instance = Drofs(file_system_path)
    with pytest.raises(ValueError, match="Data CRC32 checksum mismatch for entry 'file1.txt'. Entry data may be corrupted."):
        corrupted_drofs_instance.deserialize("/dir1/file1.txt")

def test_walk_dir2_entries(drofs_setup_teardown):
    drofs_instance = drofs_setup_teardown
    dir2_entry = drofs_instance.deserialize("/dir2")
    assert dir2_entry is not None
    assert dir2_entry.name == "dir2"
    assert dir2_entry.type == EntryType.DIRECTORY

    found_file2 = False
    found_subdir1 = False

    # Base offset for linked list data in the file (after header and CRC)
    # Using FILE_METADATA_SIZE from drofs.py
    from drofs import FILE_METADATA_SIZE
    base_offset_for_linked_list = FILE_METADATA_SIZE

    with open(drofs_instance.file_path, 'rb') as f:
        for child_offset_relative in dir2_entry.children:
            absolute_child_offset = base_offset_for_linked_list + child_offset_relative
            f.seek(absolute_child_offset)
            child_metadata = drofs_instance._read_entry_metadata(f)
            if child_metadata.name == "file2.txt":
                found_file2 = True
            if child_metadata.name == "subdir1":
                found_subdir1 = True

    assert found_file2 is True
    assert found_subdir1 is True

def test_entry_flags(drofs_setup_teardown):
    file_system_path = "test_filesystem_flags.bin"
    drofs_instance = Drofs(file_system_path)

    # Create an Entry with a flag
    from drofs import EntryFlags
    flagged_file = Entry(EntryType.FILE, "flagged_file.bin", data=bytearray(b"Some compressed data"), flags=EntryFlags.COMPRESSED.value)
    root_dir = Entry(EntryType.DIRECTORY, "root_with_flags")
    root_dir.children.append(flagged_file)
    drofs_instance.root = root_dir
    drofs_instance.serialize()

    # Deserialize and check the flag
    retrieved_flagged_file = drofs_instance.deserialize("/flagged_file.bin")
    assert retrieved_flagged_file is not None
    assert retrieved_flagged_file.name == "flagged_file.bin"
    assert retrieved_flagged_file.type == EntryType.FILE
    assert retrieved_flagged_file.data == bytearray(b"Some compressed data")
    assert retrieved_flagged_file.flags == EntryFlags.COMPRESSED.value

    # Clean up the test file
    if os.path.exists(file_system_path):
        os.remove(file_system_path)
