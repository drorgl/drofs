# Troubleshooting


## Tests Not Running Properly on ESP32
- Run the tests with verbose:
```bash
pio test -e esp32s3 -vvv
```
If you see a message like so:
```
TimeoutError: Could not automatically find serial port for the `Freenove ESP32-S3 WROOM N8R8 (8MB Flash / 8MB PSRAM)` board based on the declared HWIDs=['303A:1001']
```
it means platformio is not finding the ESP32's port

