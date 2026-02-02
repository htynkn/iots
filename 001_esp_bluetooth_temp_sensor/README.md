# BTHome Bluetooth Temperature Sensor

This project implements a Bluetooth LE temperature sensor using ESP-IDF that reports temperature data via the BTHome protocol.

## Features

- DS18B20 temperature sensor connected to GPIO4
- BTHome v2 protocol compliant advertising
- Low power design with periodic measurements every 3 minutes
- Uses ESP-IDF official components only (no third-party libraries)
- Reports temperature and battery voltage

## Note about ESP8266 vs ESP32

While the original request mentioned ESP8266, please note that ESP8266 does not support Bluetooth LE.
This project uses ESP32 which has native Bluetooth LE capabilities. The OneWire interface works similarly
on both chipsets, but Bluetooth functionality requires ESP32, ESP32-C3, ESP32-S3 or similar.

## Hardware Requirements

- ESP32 development board
- DS18B20 temperature sensor
- 4.7kΩ pull-up resistor for the OneWire bus
- Connection: DS18B20 data pin to GPIO4 of ESP32

## Software Requirements

- ESP-IDF v5.0 or higher
- ESP32 toolchain

## Pin Connections

| DS18B20 Pin | ESP32 Pin |
|-------------|-----------|
| VDD         | 3.3V      |
| GND         | GND       |
| Data        | GPIO4     |

Connect a 4.7kΩ pull-up resistor between the Data pin and VDD pin of the DS18B20.

## Building and Flashing

```bash
cd esp_bluetooth_temp_sensor
idf.py build
idf.py flash
```

## BTHome Protocol Details

The sensor broadcasts temperature data using the BTHome v2 protocol:

- Service UUID: 0xFCD2
- Device Info: 0x40 (Version 2, no encryption, periodic device)
- Temperature Type ID: 0x02
- Temperature Factor: 0.01 (value multiplied by 100 and stored as signed 16-bit integer in little-endian format)
- Battery Voltage Type ID: 0x12 (optional)

Example: A temperature of 25.06°C is transmitted as:
- Type ID: 0x02
- Value: 0xC409 (2506 in decimal, which is 25.06 * 100)

## Power Consumption Notes

The device is designed for low power consumption:
- Measurements taken every 3 minutes
- BLE advertising only during data transmission
- ESP32 deep sleep could be added for even lower power consumption

## Integration

This sensor is compatible with home automation systems that support the BTHome protocol, such as Home Assistant.