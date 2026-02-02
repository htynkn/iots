## ADDED Requirements

### Requirement: Low power management controls sleep intervals
The device SHALL enter deep sleep mode between temperature measurements to conserve power.

#### Scenario: Entering sleep mode
- **WHEN** the temperature reading and broadcasting is complete
- **THEN** the device SHALL enter deep sleep mode for 3 minutes

### Requirement: Wake up mechanism for periodic readings
The device SHALL wake up from deep sleep after 3 minutes to take new measurements.

#### Scenario: Scheduled wake up
- **WHEN** the 3-minute sleep timer expires
- **THEN** the device SHALL wake up and initiate the temperature reading process

### Requirement: Power consumption optimization
The device SHALL minimize power consumption by disabling unnecessary peripherals during sleep.

#### Scenario: Peripheral management during sleep
- **WHEN** entering sleep mode
- **THEN** the device SHALL disable all non-essential peripherals except RTC