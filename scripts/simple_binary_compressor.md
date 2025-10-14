# Simple Binary Compressor

This Python script provides a straightforward way to compress binary files using the `zlib` compression library. It's designed for command-line use, allowing you to specify an input file, an output file, and an optional compression level.

## Features

- **Zlib Compression**: Utilizes Python's built-in `zlib` module for efficient data compression.
- **Command-Line Interface**: Easy to use from the terminal with clear arguments.
- **Compression Level Control**: Supports specifying a compression level from 0 (no compression) to 9 (highest compression).
- **Error Handling**: Gracefully handles cases where the input file does not exist or I/O errors occur during file operations.
- **Detailed Output**: Provides information on original size, compressed size, and compression ratio.

## Usage

To compress a file, run the script from your terminal:

```bash
python scripts/simple_binary_compressor.py <input_filename> <output_filename> [options]
```

### Arguments

- `input_filename`: Path to the binary file you want to compress.
- `output_filename`: Path where the compressed output will be saved.

### Options

- `-l`, `--level`: Compression level (an integer from 0 to 9).
  - `0`: No compression.
  - `1-8`: Various levels of compression, with higher numbers indicating more compression but potentially slower processing.
  - `9`: Highest compression (default).

### Examples

1. **Compress a file with default compression level (9):**
   ```bash
   python scripts/simple_binary_compressor.py my_data.bin my_data.zlib
   ```

2. **Compress a file with a specific compression level (e.g., 6):**
   ```bash
   python scripts/simple_binary_compressor.py large_log.txt compressed_log.zlib --level 6
   ```

3. **Compress an image file:**
   ```bash
   python scripts/simple_binary_compressor.py image.bmp image.zlib
   ```

## How it Works

The script performs the following steps:

1. **Input Validation**: Checks if the specified input file exists.
2. **Read Data**: Reads the entire binary content of the input file into memory.
3. **Compress**: Uses `zlib.compress()` to compress the binary data with the specified compression level.
4. **Write Output**: Writes the resulting compressed binary data to the specified output file.
5. **Report**: Prints a summary of the compression process, including original size, compressed size, and the calculated compression ratio.

## Requirements

- Python 3.x

No external libraries are required beyond Python's standard `zlib` module.
