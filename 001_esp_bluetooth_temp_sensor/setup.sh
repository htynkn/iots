#!/bin/bash

# Setup script for BTHome Bluetooth Temperature Sensor

echo "Setting up BTHome Bluetooth Temperature Sensor project..."

# Check if ESP-IDF is sourced
if [ -z "$IDF_PATH" ]; then
    echo "ESP-IDF environment not detected. Please source the ESP-IDF environment:"
    echo "  source /opt/esp/idf/export.sh"
    exit 1
fi

echo "ESP-IDF environment detected at: $IDF_PATH"

# Install required components
echo "Installing required components..."
idf.py add-dependency "espressif/onewire_bus^1.0"
idf.py add-dependency "espressif/ds18b20^0.2"

echo "Building project..."
idf.py build

if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful!"
    echo ""
    echo "To flash the project, run:"
    echo "  idf.py flash"
    echo ""
    echo "To monitor serial output, run:"
    echo "  idf.py monitor"
    echo ""
else
    echo "Build failed!"
    exit 1
fi