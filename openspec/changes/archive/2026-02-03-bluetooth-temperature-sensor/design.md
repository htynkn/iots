## Context

This project involves creating a Bluetooth temperature sensor using ESP-IDF with ESP32C2 microcontroller. The device will read temperature from a DS18B20 sensor connected via OneWire protocol on GPIO4 and broadcast the data using BTHome v2 protocol over Bluetooth LE. The device needs to operate in low power mode, taking measurements every 3 minutes to conserve battery life.

## Goals / Non-Goals

**Goals:**
- Implement a working Bluetooth temperature sensor using ESP-IDF official components
- Integrate DS18B20 temperature sensor via OneWire protocol
- Broadcast temperature data using BTHome v2 protocol
- Implement low power operation with 3-minute measurement intervals
- Use only ESP-IDF official components, avoiding third-party libraries

**Non-Goals:**
- Supporting other types of sensors besides DS18B20
- Providing a web interface or local display
- Storing historical temperature data on the device
- Supporting over-the-air (OTA) updates

## Decisions

### 1. Use ESP-IDF NimBLE Stack for Bluetooth
We will use the ESP-IDF's built-in NimBLE stack for Bluetooth LE operations as it's officially supported and optimized for ESP32 series chips.

### 2. RMT Peripheral for OneWire Communication
We will use the RMT (Remote Control) peripheral to handle OneWire protocol communication with the DS18B20 sensor. This is the recommended approach in ESP-IDF for OneWire interfaces.

### 3. RTC Timer for Low Power Operation
We will use the RTC timer to schedule wake-ups every 3 minutes, allowing the device to enter deep sleep between measurements to conserve power.

### 4. BTHome v2 Protocol Implementation
We will implement BTHome v2 protocol for broadcasting temperature data, which is a standardized format for home automation sensors over Bluetooth.

### 5. GPIO4 for Sensor Connection
We will connect the DS18B20 sensor to GPIO4 as specified in the requirements, with a 4.7kΩ pull-up resistor.

## Risks / Trade-offs

[Risk: Power consumption higher than expected] → Mitigation: Carefully configure deep sleep modes and disable unnecessary peripherals during sleep periods

[Risk: Bluetooth range limitations] → Mitigation: Optimize transmission power settings and antenna placement

[Risk: Inaccurate temperature readings] → Mitigation: Proper calibration and filtering of sensor readings

[Risk: Timing drift over extended periods] → Mitigation: Use calibrated RTC timer and periodically adjust timing if needed