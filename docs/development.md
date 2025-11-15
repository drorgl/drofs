# Dependencies
- crc32 library by Christopher Baker
- miniz library by Rich Geldreich


## How to Run CRC32 Hashing

CRC32 hashing is used for data integrity within DROFS. You can calculate CRC32 values using Python's `zlib` or `binascii` modules, or use online calculators for verification

**Python Examples:**
```python
import zlib; print(f"CRC32 value of 0x01: {zlib.crc32(bytes([0x01])):#010x}")
import binascii; print(f"CRC32 value of 0x01: {binascii.crc32(bytes([0x01])):#010x}")
```


**Online Calculator:**
- https://crccalc.com/?crc=Hello%20World&method=CRC-32/ISO-HDLC&datatype=ascii&outtype=hex


# Test Data

- The [test_data](/test_data/) folder contains the directory to be made into drofs image
- The generated image is converted to c byte array using [binheader](../scripts/binheader.md)
- Standalone test data compression using [simple_binary_compressor](../scripts/simple_binary_compressor.md)

# Generating Test Data
- Uncompressed
```bash
python lib/drofs/tool/drofs_cli.py -v test/test_drofs/test_uncompressed.img test_data
python lib/drofs/tool/drofs_cli.py -v -t test/test_drofs/test_uncompressed.img test_data

python scripts/binheader.py test/test_drofs/test_uncompressed.img test/test_drofs -f mock_test_uncompressed_data -c mock_test_uncompressed_data
```

- Compressed
```bash
python lib/drofs/tool/drofs_cli.py -v -l 9 test/test_drofs/test_compressed.img test_data
python lib/drofs/tool/drofs_cli.py -v -t test/test_drofs/test_compressed.img test_data

python scripts/binheader.py test/test_drofs/test_compressed.img test/test_drofs -f mock_test_compressed_data -c mock_test_compressed_data
```

### Preparing Mock Compressed Data for compression helper tests
```bash
python scripts/generate_test_data.py test/test_drofs/uint32_sequence.bin 0 50000
python scripts/simple_binary_compressor.py -l 9 test/test_drofs/uint32_sequence.bin test/test_drofs/uint32_sequence.zlib

python scripts/binheader.py test/test_drofs/uint32_sequence.bin test/test_drofs -f mock_test_compressor_uncompressed_data -c mock_test_compressor_uncompressed_data
python scripts/binheader.py test/test_drofs/uint32_sequence.zlib test/test_drofs -f mock_test_compressor_compressed_data -c mock_test_compressor_compressed_data
```

# Tests
```bash
pytest
```

# Lint
```bash
ruff check .
```

# Coverage
```bash
gcovr --add-tracefile ".pio/tests/*.json" --exclude test --exclude .pio/.* --html-details .reports/coverage.html --root .
```