# DROFS

![logo](docs/logo/drofs2s.png)
[![Status](https://github.com/drorgl/drofs/actions/workflows/platformio-test.yml/badge.svg)](https://github.com/drorgl/drofs/actions/workflows/platformio-test.yml)


DROFS (Directory Read-Only File System) is a read-only file system designed for embedded systems, allowing efficient storage and retrieval of files and directories from a pre-built image. Key features include:
- **Hierarchical Structure:** Supports directories and nested files.
- **Metadata Support:** Allows associating custom metadata (e.g., original size, original crc32, timestamp) with entries.
- **Data Integrity:** Utilizes CRC32 checksums for overall image verification and individual entry data verification.
- **Compression:** Supports optional compression of entry data.
- **Path-based Access:** Entries can be retrieved using their full path.
- **Iteration:** Provides mechanisms to iterate over child entries within a directory.

# Contents
- [cli](docs/drofs_cli.md) - for making image files from directory structure
- [c library](docs/drofs_c_usage.md) - for reading the image
- [python library](docs/drofs_python_usage.md) - for building and reading the image


# Credits
- Inspired by [crofs](https://github.com/lllucius/esp32_crofs).
- miniz Compression library: https://github.com/joltwallet/esp_full_miniz

# [Development](docs/development.md)

# [Troubleshooting](docs/troubleshooting.md)

# License

Copyright 2025 Dror Gluska

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
