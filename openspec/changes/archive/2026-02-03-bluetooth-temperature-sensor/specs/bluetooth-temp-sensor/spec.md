## ADDED Requirements

### Requirement: Bluetooth temperature sensor implements BTHome protocol
The device SHALL broadcast temperature readings using the BTHome v2 protocol specification.

#### Scenario: Successful temperature broadcast
- **WHEN** the device reads a temperature value from the sensor
- **THEN** it SHALL broadcast the data using BTHome v2 format with service UUID 0xFCD2

### Requirement: Device uses ESP32C2 hardware
The implementation SHALL be compatible with ESP32C2 microcontroller and its peripherals.

#### Scenario: Hardware initialization
- **WHEN** the device boots up
- **THEN** it SHALL initialize ESP32C2-specific peripherals including RMT for OneWire communication

### Requirement: Temperature reporting interval
The device SHALL report temperature readings every 3 minutes.

#### Scenario: Regular temperature reporting
- **WHEN** 3 minutes have elapsed since the last report
- **THEN** the device SHALL wake up, read the temperature, and broadcast the data