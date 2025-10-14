import argparse
import os
import zlib

from drofs import Drofs, Entry, EntryFlags, EntryMetadata, EntryMetadataType, EntryType


def create_archive(image_path, source_path, compression_level, verbose):
    if verbose:
        print(f"Creating archive at: {image_path}")
        print(f"Source path: {source_path}")
        print(f"Compression level: {compression_level}")

    # Build the Drofs linked list recursively
    root_entry = build_drofs_tree(source_path, compression_level, verbose)

    drofs_instance = Drofs(image_path)
    drofs_instance.root = root_entry
    drofs_instance.serialize()

    if verbose:
        print("Archive created successfully.")

def build_drofs_tree(current_path, compression_level, verbose):
    name = os.path.basename(current_path)
    metadata_list = []

    if os.path.isdir(current_path):
        entry = Entry(EntryType.DIRECTORY, name)
        if verbose:
            print(f"Adding directory: {current_path}")

        # Add timestamp metadata for directory (creation time)
        creation_time = int(os.path.getctime(current_path))
        metadata_list.append(EntryMetadata(EntryMetadataType.TIMESTAMP, creation_time.to_bytes(4, 'little')))
        entry.metadata = metadata_list

        for item in os.listdir(current_path):
            item_path = os.path.join(current_path, item)
            child_entry = build_drofs_tree(item_path, compression_level, verbose)
            if child_entry:
                entry.children.append(child_entry)
        return entry
    elif os.path.isfile(current_path):
        with open(current_path, 'rb') as f:
            data = f.read()

        original_crc32 = zlib.crc32(data)
        flags = 0
        if compression_level > 0:
            original_size = len(data)
            compressed_data = zlib.compress(data, compression_level)
            if (len(compressed_data) < len(data)):
                print(f"{current_path}: compressed {len(compressed_data)} is smaller than original {len(data)}")
                flags |= EntryFlags.COMPRESSED.value
                data = compressed_data
                print(f"adding original crc32 {original_crc32:#010x}")
                metadata_list.append(EntryMetadata(EntryMetadataType.ORIGINAL_CRC32, original_crc32.to_bytes(4, 'little')))
            else:
                print(f"{current_path}: compressed {len(compressed_data)} is larger than original {len(data)}")
        else:
            original_size = len(data)

        # Add original size metadata for file
        print("adding original size ", original_size)
        metadata_list.append(EntryMetadata(EntryMetadataType.ORIGINAL_SIZE, original_size.to_bytes(4, 'little')))

        # Add timestamp metadata for file (modification time)
        modification_time = int(os.path.getmtime(current_path))
        print("adding timestamp ", modification_time)
        metadata_list.append(EntryMetadata(EntryMetadataType.TIMESTAMP, modification_time.to_bytes(4, 'little')))

        entry = Entry(EntryType.FILE, name, bytearray(data), flags=flags, metadata=metadata_list)
        if verbose:
            print(f"Adding file: {current_path}")
        return entry
    else:
        if verbose:
            print(f"Skipping unknown item: {current_path}")
        return None

def compare_archive(image_path, source_path, verbose):
    if verbose:
        print(f"Comparing archive: {image_path} with source path: {source_path}")

    drofs_instance = Drofs(image_path)
    root_archive_entry = drofs_instance.deserialize_root()

    if not root_archive_entry:
        print(f"Error: Could not deserialize archive from {image_path}")
        return

    # Perform comparison recursively
    compare_recursive(root_archive_entry, source_path, verbose)

    if verbose:
        print("Comparison complete.")

def compare_recursive(archive_entry : Entry, current_source_path, verbose):
    # This function needs to be implemented to compare the archive content with the file system
    # This will involve iterating through archive_entry's children and comparing them with
    # files/directories in current_source_path.
    # For now, this is a placeholder.
    if verbose:
        print(f"Comparing archive entry '{archive_entry.name}' with '{current_source_path}'")

    # Example: Check if the source path exists
    if not os.path.exists(current_source_path):
        print(f"Source path '{current_source_path}' does not exist.")
        return

    # If it's a directory in the archive
    if archive_entry.type == EntryType.DIRECTORY:
        if not os.path.isdir(current_source_path):
            print(f"Mismatch: Archive entry '{archive_entry.name}' is a directory, but '{current_source_path}' is not.")
            return

        # Compare children
        print(f"Comparing {archive_entry}")
        archive_children_names = {child.name for child in archive_entry.children if isinstance(child, Entry)}
        print(f"Archive Children Names {archive_children_names} from {archive_entry.children}")
        source_children_names = set(os.listdir(current_source_path))

        # Check for items in archive but not in source
        for name in archive_children_names:
            if name not in source_children_names:
                print(f"Missing in source: '{os.path.join(current_source_path, name)}'")

        # Check for items in source but not in archive
        for name in source_children_names:
            if name not in archive_children_names:
                print(f"Missing in archive: '{os.path.join(current_source_path, name)}'")

        # Recursively compare common children
        for child_archive_entry in archive_entry.children:
            if child_archive_entry.name in source_children_names:
                child_source_path = os.path.join(current_source_path, child_archive_entry.name)
                compare_recursive(child_archive_entry, child_source_path, verbose)

    # If it's a file in the archive
    elif archive_entry.type == EntryType.FILE:
        if not os.path.isfile(current_source_path):
            print(f"Mismatch: Archive entry '{archive_entry.name}' is a file, but '{current_source_path}' is not.")
            return

        with open(current_source_path, 'rb') as f:
            source_data = f.read()

        archive_data = archive_entry.data
        # If compressed, decompress before comparison
        if archive_entry.flags & EntryFlags.COMPRESSED.value:
            archive_data = zlib.decompress(archive_data)

        if archive_data != source_data:
            print(f"Content mismatch: '{current_source_path}'")
        else:
            if verbose:
                print(f"Content match: '{current_source_path}'")

        # Compare original size metadata if available
        original_size_metadata = archive_entry.get_metadata_by_type(EntryMetadataType.ORIGINAL_SIZE)
        if original_size_metadata:
            stored_original_size = int.from_bytes(original_size_metadata.data, 'little')
            if stored_original_size != len(source_data):
                print(f"Original size metadata mismatch for '{current_source_path}': Stored {stored_original_size}, Actual {len(source_data)}")
            else:
                if verbose:
                    print(f"Original size metadata match for '{current_source_path}': {stored_original_size}")
        else:
            if verbose:
                print(f"No ORIGINAL_SIZE metadata found for '{current_source_path}'")


def main():
    parser = argparse.ArgumentParser(description="DROFS CLI tool for creating and comparing archives.")
    parser.add_argument("imagepath", help="Path to the DROFS archive file.")
    parser.add_argument("sourcepath", help="Path to the source directory or file.")
    parser.add_argument("-l", "--level", type=int, default=0, choices=range(0, 10),
                        help="Compression level (0-9). 0 means no compression. Compatible with miniz (zlib).")
    parser.add_argument("-t", "--test", action="store_true",
                        help="Compare the image with the folder, reading file by file and comparing contents.")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="Display what the CLI is doing.")

    args = parser.parse_args()

    if args.test:
        compare_archive(args.imagepath, args.sourcepath, args.verbose)
    else:
        create_archive(args.imagepath, args.sourcepath, args.level, args.verbose)

if __name__ == "__main__":
    main()
