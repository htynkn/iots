idf_component_register(
    SRCS "bthome_temp_sensor.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_timer bt esp_adc onewire_bus ds18b20 nvs_flash esp_driver_gpio)