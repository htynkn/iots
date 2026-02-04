## Context

We are initializing an ESP32C2 project for a temperature sensor application. The project needs to be configured specifically for 26MHz CPU frequency and 2MB storage capacity. This is a foundational step to ensure the development environment is properly set up before implementing the Bluetooth temperature sensor functionality. The ESP-IDF v5.4 framework is the chosen development framework.

## Goals / Non-Goals

**Goals:**
- Successfully initialize an ESP-IDF project for ESP32C2
- Configure the project for 26MHz CPU frequency and 2MB storage
- Implement basic "Hello World" functionality using ESP logging
- Ensure the project compiles without errors
- Enable successful flashing to ESP32C2 device
- Verify output through serial monitor
- Create proper project structure with CMakeLists.txt files

**Non-Goals:**
- Implement actual temperature sensing functionality
- Implement Bluetooth communication
- Add advanced power management features
- Create complex user interfaces

## Decisions

1. **ESP-IDF Framework Choice**: Using ESP-IDF v5.4 as it's the latest stable version with full ESP32C2 support. This framework provides all necessary drivers and tools for ESP32 development.

2. **Project Structure**: Following ESP-IDF recommended project structure with main directory containing the application code and top-level CMakeLists.txt for the project.

3. **CPU Frequency Configuration**: Setting CPU frequency to 26MHz as specified in requirements. This is lower than the default 80MHz or 160MHz, which will reduce power consumption but also processing speed.

4. **Storage Configuration**: Configuring for 2MB flash storage which is typical for ESP32C2 variants. This affects partition table selection.

5. **Logging Method**: Using ESP_LOGI for "Hello World" output as it's the standard logging mechanism in ESP-IDF and integrates well with the serial monitor.

6. **Build System**: Using CMake as the build system as it's the default and recommended build system for ESP-IDF v5.4.

## Risks / Trade-offs

[Risk: Incorrect CPU frequency configuration] → Mitigation: Carefully configure sdkconfig.defaults with appropriate frequency settings and verify with frequency detection functions

[Risk: Insufficient flash storage for application] → Mitigation: Start with minimal "Hello World" application to verify basic functionality before expanding

[Risk: Toolchain compatibility issues] → Mitigation: Use official ESP-IDF v5.4 toolchain and verify compatibility with ESP32C2

[Risk: Serial communication issues] → Mitigation: Use standard UART pins and verify baud rate settings in configuration