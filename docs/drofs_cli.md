
# DROFS CLI Usage

The `drofs_cli.py` script provides a command-line interface for creating and comparing DROFS archives.

## Usage

```
python drofs_cli.py [-l level] [-t] [-v] imagepath sourcepath
```

## Arguments

*   `imagepath`: Path to the DROFS archive file.
*   `sourcepath`: Path to the source directory or file to be archived or compared against.

## Options

*   `-l`, `--level <level>`: Compression level (0-9). 0 means no compression. This uses `zlib` which is compatible with `miniz`.
    *   Default: `0` (no compression)
*   `-t`, `--test`: Compare the `imagepath` archive with the `sourcepath` folder. It reads file by file, determines if it's in the archive, and compares its contents.
*   `-v`, `--verbose`: Display what the CLI is doing, providing detailed output during archive creation or comparison.

## Examples

### Create an archive without compression

```bash
python lib/drofs/tool/drofs_cli.py my_archive.drofs /path/to/source_folder
```

### Create a compressed archive (level 6)

```bash
python lib/drofs/tool/drofs_cli.py -l 6 my_compressed_archive.drofs /path/to/source_folder
```

### Compare an archive with a source folder (verbose)

```bash
python lib/drofs/tool/drofs_cli.py -t -v my_archive.drofs /path/to/source_folder
```

### Create a compressed archive with verbose output

```bash
python lib/drofs/tool/drofs_cli.py -l 9 -v my_super_compressed_archive.drofs /path/to/another_folder
```