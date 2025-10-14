import argparse
import binascii  # Import zlib for CRC32 calculation
import datetime
import os


def bin_to_c_files(bin_file_path, base_name, output_folder, array_name="binary_data"):
    """
    Converts a binary file to C header and implementation files, with all variables
    prefixed by the given array name and adds a C-style comment with ASCII
    representation for each line of binary data.

    Args:
        bin_file_path (str): Path to the input binary file.
        base_name (str): The base name for the output files (e.g., 'data' will
                         create 'data.h' and 'data.c').
        output_folder (str): The path to the folder where the files will be saved.
        array_name (str): The name for the C byte array variable.
    """
    try:
        # 1. Ensure the output folder exists
        os.makedirs(output_folder, exist_ok=True)

        # 2. Get file modification and generation times
        bin_mod_timestamp = os.path.getmtime(bin_file_path)
        binary_modified_date = datetime.datetime.fromtimestamp(bin_mod_timestamp).strftime("%Y-%m-%d %H:%M:%S")
        c_generated_date = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        # 3. Define full output file paths
        h_file_path = os.path.join(output_folder, f"{base_name}.h")
        c_file_path = os.path.join(output_folder, f"{base_name}.c")

        # 4. Read the binary file
        with open(bin_file_path, "rb") as f_bin:
            binary_data = f_bin.read()

        # 5. Generate the header file (.h)
        with open(h_file_path, "w") as f_h:
            f_h.write(f"#ifndef {array_name.upper()}_H\n")
            f_h.write(f"#define {array_name.upper()}_H\n\n")
            f_h.write("#include <stddef.h>\n")
            f_h.write("#include <stdint.h>\n\n")

            # Use 'extern' to declare the variables, all with the prefix
            f_h.write(f"extern const unsigned char {array_name}[];\n")
            f_h.write(f"extern const size_t {array_name}_len;\n")
            f_h.write(f"extern const uint32_t {array_name}_crc32;\n\n") # Add CRC32 declaration

            f_h.write(f'extern const char {array_name}_binary_modified_date[];\n')
            f_h.write(f'extern const char {array_name}_c_generated_date[];\n')
            f_h.write(f'extern const char {array_name}_c_compiled_date[];\n\n')

            f_h.write(f"#endif // {array_name.upper()}_H\n")

        print(f"Generated header file: '{h_file_path}'")

        # Calculate CRC32
        # crc32_value = binascii.crc32(binary_data) & 0xFFFFFFFF # Ensure it's a 32-bit unsigned int

        # 6. Generate the implementation file (.c)
        with open(c_file_path, "w") as f_c:
            f_c.write(f'#include "{base_name}.h"\n\n')
            # f_c.write("#include <stdint.h>\n\n") # Include stdint.h for uint32_t

            # The actual binary data definition
            f_c.write(f"const unsigned char {array_name}[] = {{\n")

            # ASCII representation for each line
            for i in range(0, len(binary_data), 16):
                line_data = binary_data[i:i+16]
                hex_string = ", ".join([f"0x{byte:02x}" for byte in line_data])

                # Create the ASCII representation for the comment
                ascii_string = ""
                for byte in line_data:
                    if 32 <= byte <= 126: # Check for printable ASCII
                        ascii_string += chr(byte)
                    else:
                        ascii_string += "."

                # Write the line with hex values and the comment
                f_c.write(f"    /* 0x{i:08x} */ {hex_string}, //* {ascii_string} */ \n")

            f_c.write("};\n\n")
            f_c.write(f"const size_t {array_name}_len = {len(binary_data)};\n")
            # Add CRC32 definition
            f_c.write(f"const uint32_t {array_name}_crc32 = {binascii.crc32(binary_data):#010x};\n\n")

            # Date/time string definitions, all with the prefix
            f_c.write(f'const char {array_name}_binary_modified_date[] = "{binary_modified_date}";\n')
            f_c.write(f'const char {array_name}_c_generated_date[] = "{c_generated_date}";\n')
            f_c.write(f'const char {array_name}_c_compiled_date[] = __DATE__ " " __TIME__;\n')

        print(f"Generated implementation file: '{c_file_path}'")

    except FileNotFoundError:
        print(f"Error: The file '{bin_file_path}' was not found.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")


def main():
    parser = argparse.ArgumentParser(description="binheader CLI tool for generating headers")
    parser.add_argument("binpath", help="Path to the binary")
    parser.add_argument("sourcepath", help="Path to the source directory")
    parser.add_argument("-f", "--filename", type=str, default=None,
                        help="file name")
    parser.add_argument("-c", "--constantname", type=str, default=None,
                        help="constant name")

    args = parser.parse_args()

    if args.filename is None:
        args.filename = os.path.basename(args.binpath) # Changed from imagepath to binpath

    if args.constantname is None:
        args.constantname = os.path.basename(args.binpath) + "_data" # Changed from imagepath to binpath

    bin_to_c_files(args.binpath,args.filename, args.sourcepath, args.constantname)

if __name__ == "__main__":
    main()
