## Why

当前的 `001_esp_bluetooth_temp_sensor` 项目存在编译问题，且配置不兼容ESP8684芯片（主频26MHz，容量2M）。需要修正这些问题以确保项目能在目标硬件上正常编译和运行。

## What Changes

- 修正现有的编译错误和警告
- 调整SDK配置以适配ESP8684的硬件特性（主频26MHz，容量2M）
- 更新分区表和内存配置以适应2M Flash容量
- 修改时钟频率相关设置以匹配26MHz主频
- 确保蓝牙温度传感器功能在ESP8684上正常工作

## Capabilities

### New Capabilities
- `esp8684-support`: 实现对ESP8684芯片的支持，包括正确的时钟配置和内存管理
- `bluetooth-temp-sensor-esp8684`: 在ESP8684平台上运行蓝牙温度传感器功能

### Modified Capabilities
- `bluetooth-temp-sensor`: 原有的蓝牙温度传感器功能，需要调整以适应ESP8684平台的限制

## Impact

- 项目配置文件（sdkconfig.defaults, partitions.csv等）将被修改
- CMakeLists.txt可能需要更新以支持ESP8684
- 源代码中与时钟频率相关的部分可能需要调整
- 编译脚本和构建配置将受到影响