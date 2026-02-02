## Why

创建一个低功耗蓝牙温度传感器，使用ESP32C2芯片和OneWire温度传感器，通过BTHome协议定期上报温度数据，满足物联网环境监测的需求。

## What Changes

- 创建一个新的ESP-IDF项目，专门用于蓝牙温度传感
- 集成OneWire协议支持DS18B20温度传感器
- 实现BTHome v2协议进行蓝牙数据传输
- 设计低功耗机制，每3分钟上报一次温度
- 使用ESP-IDF官方组件，避免第三方库依赖

## Capabilities

### New Capabilities
- `bluetooth-temp-sensor`: 实现蓝牙温度传感器的核心功能，包括OneWire接口、温度读取和BTHome协议
- `low-power-manager`: 管理设备的低功耗操作，控制测量间隔和休眠模式
- `sensor-interface`: 提供与DS18B20温度传感器的接口，包括初始化、读取和校准

### Modified Capabilities

## Impact

- 需要ESP-IDF框架支持
- 依赖ESP32C2的蓝牙LE功能
- 需要RMT外设用于OneWire通信
- 影响电源管理模块以实现低功耗