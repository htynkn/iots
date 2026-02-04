## Why

We need to initialize an ESP32C2 project that prints "Hello World" to verify the development environment setup. This serves as a foundational step for developing the 001_esp_bluetooth_temp_sensor project, ensuring the toolchain, compilation, and flashing processes work correctly with the specified hardware constraints (26MHz CPU, 2MB storage).

## What Changes

- Initialize a new ESP-IDF project for ESP32C2 microcontroller
- Configure project for 26MHz CPU frequency and 2MB storage
- Implement basic "Hello World" functionality using ESP_LOGI
- Set up project structure with proper CMakeLists.txt files
- Ensure project compiles without errors
- Enable flashing and monitoring capabilities
- Verify output through serial monitor

## Capabilities

### New Capabilities
- `esp32c2-hello-world`: Basic ESP32C2 project initialization with "Hello World" functionality, proper configuration for 26MHz CPU and 2MB storage, and compilation/flashing verification

### Modified Capabilities

## Impact

- Creates a new ESP-IDF project in the 001_esp_bluetooth_temp_sensor directory
- Requires ESP-IDF v5.4 framework installation
- Sets up build environment for ESP32C2 chip
- Establishes foundation for future Bluetooth temperature sensor functionality