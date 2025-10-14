# `binheader.py` - Binary to C Header/Source Converter

`binheader.py` is a Python command-line tool designed to embed binary data directly into C/C++ projects. It takes any binary file as input and generates a corresponding C header (`.h`) and source (`.c`) file. These generated files contain the binary data as a `const unsigned char` array, along with its length, a CRC32 checksum, and relevant timestamp information.

This tool is particularly useful for embedding small assets, firmware, or configuration data directly into an executable, eliminating the need for external file system access at runtime.

## Features

- **Binary to C Array Conversion**: Converts any binary file into a `const unsigned char` array.
- **Data Length**: Provides the exact length of the embedded binary data.
- **CRC32 Checksum**: Calculates and embeds a CRC32 checksum for data integrity verification.
- **Timestamp Information**: Includes the original binary file's modification date, the C file generation date, and the C compilation date (`__DATE__ __TIME__` macros).
- **Human-Readable Comments**: Each line of binary data in the `.c` file is commented with its hexadecimal representation, memory offset, and a printable ASCII interpretation, aiding in debugging and verification.
- **Customizable Names**: Allows specifying the base filename for the generated C files and the name of the C array constant.

## Usage

```bash
python binheader.py <BIN_FILE_PATH> <OUTPUT_DIRECTORY> [OPTIONS]
```

### Arguments

- `<BIN_FILE_PATH>`: The path to the input binary file you want to convert.
- `<OUTPUT_DIRECTORY>`: The directory where the generated `.h` and `.c` files will be saved.

### Options

- `-f`, `--filename <FILENAME>`:
  Specifies the base name for the output `.h` and `.c` files.
  If not provided, the base name of `BIN_FILE_PATH` will be used.
  Example: `-f my_data` will generate `my_data.h` and `my_data.c`.

- `-c`, `--constantname <CONSTANT_NAME>`:
  Specifies the name for the C byte array variable and its associated metadata (length, CRC32, dates).
  If not provided, it defaults to `<BIN_FILE_BASE_NAME>_data`.
  Example: `-c firmware_image` will create `const unsigned char firmware_image[]`, `firmware_image_len`, etc.

## Example

Let's say you have a binary file named `image.bin` and you want to embed it into your C project.

```bash
python binheader.py image.bin ./src/generated -f embedded_image -c my_image_data
```

This command will:
1. Read `image.bin`.
2. Create `src/generated/embedded_image.h` and `src/generated/embedded_image.c`.
3. The C files will contain:
   - `const unsigned char my_image_data[]`
   - `const size_t my_image_data_len`
   - `const uint32_t my_image_data_crc32`
   - `const char my_image_data_binary_modified_date[]`
   - `const char my_image_data_c_generated_date[]`
   - `const char my_image_data_c_compiled_date[]`

### `embedded_image.h` (Example Output)

```c
#ifndef MY_IMAGE_DATA_H
#define MY_IMAGE_DATA_H

#include <stddef.h>
#include <stdint.h>

extern const unsigned char my_image_data[];
extern const size_t my_image_data_len;
extern const uint32_t my_image_data_crc32;

extern const char my_image_data_binary_modified_date[];
extern const char my_image_data_c_generated_date[];
extern const char my_image_data_c_compiled_date[];

#endif // MY_IMAGE_DATA_H
```

### `embedded_image.c` (Example Output Snippet)

```c
#include "embedded_image.h"

const unsigned char my_image_data[] = {
    /* 0x00000000 */ 0x44, 0x52, 0x4f, 0x46, 0x53, 0xea, 0x3f, 0x74, 0x25, 0x02, 0x0c, 0x73, 0x61, 0x6d, 0x70, 0x6c, //* DROFS.?t%..sampl */
    /* 0x00000010 */ 0x65, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //* e_data.......... */
    // ... (rest of the binary data)
};

const size_t my_image_data_len = 9928; // Example length
const uint32_t my_image_data_crc32 = 0xa7622c60; // Example CRC32

const char my_image_data_binary_modified_date[] = "2025-10-06 22:22:49";
const char my_image_data_c_generated_date[] = "2025-10-06 23:07:18";
const char my_image_data_c_compiled_date[] = __DATE__ " " __TIME__;
