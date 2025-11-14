import argparse
import os
import struct

# Configuration
# '<I' means:
# < : Little-endian byte order
# I : Unsigned integer (4 bytes, which is uint32_t)
STRUCT_FORMAT = '<I'
BYTES_PER_VALUE = 4

def generate_binary_sequence_file(output_filename, start_value, end_value):
    """
    Generates a binary file containing a sequence of uint32_t values
    from START_VALUE up to and including END_VALUE.
    """

    total_values = end_value - start_value + 1
    target_size = total_values * BYTES_PER_VALUE

    print(f"Generating binary file: {output_filename}")
    print(f"Total values to write: {total_values}")
    print(f"Expected file size: {target_size} bytes (approx. {target_size / 1024 / 1024:.2f} MB)")

    try:
        # Open the file in binary write mode ('wb')
        with open(output_filename, 'wb') as f:
            for value in range(start_value, end_value + 1):
                # Pack the integer into 4 bytes using the specified format
                packed_data = struct.pack(STRUCT_FORMAT, value)
                f.write(packed_data)

                # Optional progress update (every 100,000 values)
                if value > start_value and value % 100000 == 0:
                    print(f"  Processed {value:,} values...")

        # Final check and success message
        final_size = os.path.getsize(output_filename)
        print("-" * 40)
        print(f"Generation complete. Final file size: {final_size:,} bytes")
        if final_size == target_size:
            print("File size matches expected size. Success!")
        else:
            print("Warning: File size mismatch.")

    except OSError as e:
        print(f"Error writing to file: {e}")
    except struct.error as e:
        print(f"Error packing data: {e}. Check if values exceed uint32_t capacity (4,294,967,295).")

def main():
    parser = argparse.ArgumentParser(
        description="Generate a binary file with a sequence of uint32_t values."
    )
    parser.add_argument(
        "output_filename",
        help="The name of the output binary file (e.g., uint32_sequence.bin)"
    )
    parser.add_argument(
        "start_value",
        type=int,
        help="The starting value of the sequence (inclusive)"
    )
    parser.add_argument(
        "end_value",
        type=int,
        help="The ending value of the sequence (inclusive)"
    )

    args = parser.parse_args()

    generate_binary_sequence_file(args.output_filename, args.start_value, args.end_value)

if __name__ == "__main__":
    main()
