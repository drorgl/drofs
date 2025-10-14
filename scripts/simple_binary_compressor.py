import argparse
import os
import zlib


def compress_file_with_zlib(input_filename, output_filename, compression_level):
    """
    Reads the content of the uncompressed file, compresses it using zlib,
    and saves the compressed data to a new file.
    """
    print("Starting compression process...")

    # 1. Check if the input file exists
    if not os.path.exists(input_filename):
        print(f"Error: Input file '{input_filename}' not found.")
        print("Please ensure the input file exists.")
        return

    # 2. Read the binary data from the input file
    try:
        with open(input_filename, 'rb') as f:
            uncompressed_data = f.read()
            input_size = len(uncompressed_data)
            print(f"Read input file: {input_filename}")
            print(f"Uncompressed size: {input_size:,} bytes")
    except OSError as e:
        print(f"Error reading input file: {e}")
        return

    # 3. Compress the data using zlib
    print(f"Compressing data (level={compression_level})...")
    try:
        compressed_data = zlib.compress(uncompressed_data, compression_level)
        output_size = len(compressed_data)
    except zlib.error as e:
        print(f"Error during zlib compression: {e}")
        return

    # 4. Write the compressed data to the output file
    try:
        with open(output_filename, 'wb') as f:
            f.write(compressed_data)
        print(f"Wrote compressed file: {output_filename}")
    except OSError as e:
        print(f"Error writing output file: {e}")
        return

    # 5. Report results
    print("-" * 40)
    print("Compression Results:")
    print(f"Original Size: {input_size:,} bytes")
    print(f"Compressed Size: {output_size:,} bytes")

    # Calculate compression ratio
    ratio = (output_size / input_size) * 100
    print(f"Compression Ratio: {ratio:.2f}% ({(input_size - output_size):,} bytes saved)")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compress a binary file using zlib.")
    parser.add_argument("input_filename", help="Path to the input file to be compressed.")
    parser.add_argument("output_filename", help="Path for the compressed output file.")
    parser.add_argument("-l", "--level", type=int, default=9,
                        help="Compression level (0-9, 9 is highest). Default is 9.")

    args = parser.parse_args()

    compress_file_with_zlib(args.input_filename, args.output_filename, args.level)
