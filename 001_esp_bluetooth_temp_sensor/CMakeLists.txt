cmake_minimum_required(VERSION 3.16)

# Set default target to esp8684 if not already set
if(NOT DEFINED IDF_TARGET)
    set(IDF_TARGET "esp8684")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp_bluetooth_temp_sensor)