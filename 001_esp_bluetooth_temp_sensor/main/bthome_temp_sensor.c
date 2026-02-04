/*
 * BTHome Bluetooth Temperature Sensor
 *
 * This project implements a Bluetooth LE temperature sensor using ESP-IDF
 * that reports temperature data via the BTHome protocol.
 *
 * Features:
 * - DS18B20 temperature sensor connected to GPIO4
 * - BTHome v2 protocol compliant advertising
 * - Low power design with periodic measurements every 3 minutes
 * - Uses ESP-IDF official components only
 * - Optimized for ESP8684 with 26MHz crystal and 2M Flash
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "onewire_bus.h"
#include "ds18b20.h"

#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"

// Configuration
#define TAG "BTHomeTempSensor"
#ifndef CONFIG_BTHOME_SENSOR_GPIO
#define SENSOR_GPIO 4
#else
#define SENSOR_GPIO CONFIG_BTHOME_SENSOR_GPIO
#endif

#ifndef CONFIG_BTHOME_MEASUREMENT_INTERVAL
#define MEASUREMENT_INTERVAL_MS (3 * 60 * 1000)  // 3 minutes in milliseconds
#else
#define MEASUREMENT_INTERVAL_MS (CONFIG_BTHOME_MEASUREMENT_INTERVAL * 1000)
#endif

#define BATTERY_MEASUREMENT_ADC_UNIT ADC_UNIT_1
#define BATTERY_MEASUREMENT_CHANNEL ADC_CHANNEL_0  // GPIO3 for ESP32 (adjust as needed)

// BTHome constants
#define BTHOME_SERVICE_UUID 0xFCD2
#define BTHOME_VERSION 0x40  // Version 2, no encryption, periodic device
#define BTHOME_TYPE_TEMPERATURE 0x02
#define BTHOME_TYPE_VOLTAGE 0x12

// Global variables
static bool ble_ready = false;
static onewire_bus_handle_t bus = NULL;
static ds18b20_device_handle_t ds18b20_dev = NULL;

// Forward declarations
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static esp_err_t init_ble(void);
static esp_err_t update_advertisement(float temperature, uint16_t battery_mv);

// GAP event handler
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(TAG, "Advertisement data set complete");
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG, "Failed to start advertisement: %d", param->adv_start_cmpl.status);
            } else {
                ESP_LOGI(TAG, "Started BLE advertisement");
                ble_ready = true;
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG, "Failed to stop advertisement: %d", param->adv_stop_cmpl.status);
            } else {
                ESP_LOGI(TAG, "Stopped BLE advertisement");
            }
            break;
        default:
            break;
    }
}

// Initialize BLE
static esp_err_t init_ble(void)
{
    esp_err_t ret;

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize BLE
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

#if CONFIG_IDF_TARGET_ESP8684
    // ESP8684 specific Bluetooth configuration
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    // Adjust buffer sizes for ESP8684 if needed
    bt_cfg.bluetooth.mode = ESP_BT_MODE_BLE;
    bt_cfg.ble_max_conn = 1;  // Reduce max connections to save memory
#else
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
#endif

    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "gap register error, error number %x", ret);
        return ret;
    }

    return ESP_OK;
}

// Initialize OneWire and DS18B20
static esp_err_t init_onewire_sensor(void)
{
    // Configure OneWire bus
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = SENSOR_GPIO,
        .flags = {
            .en_pull_up = true,
        },
    };

    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = 10,
    };

    ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));
    ESP_LOGI(TAG, "OneWire bus initialized on GPIO%d", SENSOR_GPIO);

    // Search for DS18B20 device
    onewire_device_iter_handle_t iter = NULL;
    onewire_device_t next_onewire_device;

    ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
    ESP_LOGI(TAG, "Searching for DS18B20 device...");

    esp_err_t search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
    if (search_result == ESP_OK) {
        // Create DS18B20 device handle from the enumeration result
        ds18b20_config_t ds_cfg = {};
        ESP_ERROR_CHECK(ds18b20_new_device_from_enumeration(&next_onewire_device, &ds_cfg, &ds18b20_dev));
        ESP_LOGI(TAG, "DS18B20 device found, address: %016llX", next_onewire_device.address);

        // Set resolution to 12-bit for highest accuracy
        ESP_ERROR_CHECK(ds18b20_set_resolution(ds18b20_dev, DS18B20_RESOLUTION_12B));
    } else {
        ESP_LOGE(TAG, "No DS18B20 device found");
        ESP_ERROR_CHECK(onewire_del_device_iter(iter));
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(onewire_del_device_iter(iter));
    return ESP_OK;
}

// Read battery voltage (simplified - adjust based on your voltage divider circuit)
static uint16_t read_battery_voltage(void)
{
    // This is a simplified implementation - adjust based on your actual hardware setup
    // For now, returning a fixed value as an example
    // In a real implementation, you would measure the actual battery voltage

    // Example: if using a voltage divider that halves the battery voltage
    // and assuming the ADC reference voltage is 3.3V
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    int raw_reading;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw_reading));

    // Convert raw reading to millivolts (assuming 12-bit ADC and voltage divider)
    // This is a placeholder - adjust according to your actual circuit
    uint16_t battery_mv = 3700; // Placeholder value

    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));

    return battery_mv;
}

// Update advertisement with temperature data
static esp_err_t update_advertisement(float temperature, uint16_t battery_mv)
{
    if (!ble_ready) {
        ESP_LOGW(TAG, "BLE not ready, skipping advertisement update");
        return ESP_FAIL;
    }

    // Prepare BTHome service data
    // Format: [BTHome UUID] [Device Info] [Type ID] [Data]
    uint8_t adv_data[31]; // Max BLE advertisement data length
    uint8_t data_idx = 0;
    
    // Service UUID (0xFCD2) - little endian
    adv_data[data_idx++] = 0xD2;  // LSB
    adv_data[data_idx++] = 0xFC;  // MSB
    
    // Device info byte (BTHome version 2, no encryption, periodic device)
    adv_data[data_idx++] = BTHOME_VERSION;
    
    // Temperature measurement
    // Type ID: 0x02 (Temperature)
    adv_data[data_idx++] = BTHOME_TYPE_TEMPERATURE;
    
    // Temperature value: convert to integer with 0.01 factor
    // For example, 25.06°C becomes 2506
    int16_t temp_raw = (int16_t)(temperature * 100.0f);
    
    // Store as little-endian bytes
    adv_data[data_idx++] = temp_raw & 0xFF;         // LSB
    adv_data[data_idx++] = (temp_raw >> 8) & 0xFF;  // MSB
    
    // Battery voltage measurement (optional)
    // Type ID: 0x12 (Voltage)
    adv_data[data_idx++] = BTHOME_TYPE_VOLTAGE;
    
    // Voltage value in mV
    adv_data[data_idx++] = battery_mv & 0xFF;         // LSB
    adv_data[data_idx++] = (battery_mv >> 8) & 0xFF;  // MSB

    // Prepare advertising data structure
    esp_ble_adv_data_t adv_data_struct = {
        .set_scan_rsp = false,
        .include_txpower = true,
        .min_interval = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
        .max_interval = 0x0010, // slave connection max interval, Time = max_interval * 1.25 msec
        .appearance = 0x00,
        .manufacturer_len = 0,
        .p_manufacturer_data = NULL,
        .service_data_len = data_idx,
        .p_service_data = adv_data,
        .service_uuid_len = 0,
        .p_service_uuid = NULL,
        .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
    };

    // Set the advertising data
    esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data_struct);
    if (ret) {
        ESP_LOGE(TAG, "Config adv data failed, error code = %x", ret);
        return ret;
    }

    // Start advertising
    esp_ble_adv_params_t adv_params = {
        .adv_int_min = 0x100,  // Minimum advertising interval: 250ms (0x100 * 0.625ms)
        .adv_int_max = 0x100,  // Maximum advertising interval: 250ms (0x100 * 0.625ms)
        .adv_type = ADV_TYPE_NONCONN_IND,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };

    ret = esp_ble_gap_start_advertising(&adv_params);
    if (ret) {
        ESP_LOGE(TAG, "Start advertising failed, error code = %x", ret);
        return ret;
    }

    ESP_LOGI(TAG, "Updated advertisement with temperature: %.2f°C, battery: %"PRIu16"mV", 
             temperature, battery_mv);

    return ESP_OK;
}

// Initialize all required modules
static esp_err_t initialize_modules(void)
{
    esp_err_t ret;

    // Initialize OneWire sensor
    ret = init_onewire_sensor();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize DS18B20 sensor: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize BLE
    ret = init_ble();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BLE: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

// Main application task
void temperature_measurement_task(void *pvParameters)
{
    float temperature;
    uint16_t battery_mv;
    esp_err_t ret;

    // Initialize all modules
    if (initialize_modules() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize modules");
        vTaskDelete(NULL); // Delete this task if initialization fails
    }

    // Main loop - only runs once for battery-powered devices
    // After first measurement cycle, the device goes to deep sleep and restarts from app_main

    // Trigger temperature conversion
    ret = ds18b20_trigger_temperature_conversion(ds18b20_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to trigger temperature conversion: %s", esp_err_to_name(ret));
        // Attempt to reinitialize the sensor
        if (init_onewire_sensor() != ESP_OK) {
            ESP_LOGE(TAG, "Failed to reinitialize DS18B20 sensor");
        } else {
            ESP_LOGI(TAG, "DS18B20 sensor reinitialized successfully");
        }
        goto sleep_and_restart;
    }

    // Wait for conversion to complete (max 750ms for 12-bit resolution)
    // Adjusted for ESP8684 with 26MHz crystal if needed
    vTaskDelay(pdMS_TO_TICKS(750));

    // Read temperature
    ret = ds18b20_get_temperature(ds18b20_dev, &temperature);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read temperature: %s", esp_err_to_name(ret));
        // Attempt to reinitialize the sensor
        if (init_onewire_sensor() != ESP_OK) {
            ESP_LOGE(TAG, "Failed to reinitialize DS18B20 sensor");
        } else {
            ESP_LOGI(TAG, "DS18B20 sensor reinitialized successfully");
        }
        goto sleep_and_restart;
    }

    // Read battery voltage
    battery_mv = read_battery_voltage();

    // Update advertisement with new data
    ret = update_advertisement(temperature, battery_mv);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update advertisement: %s", esp_err_to_name(ret));
        // Continue anyway, as we still want to go to sleep
    }

sleep_and_restart:
    // Sleep for the measurement interval
    ESP_LOGI(TAG, "Going to sleep for %d seconds", MEASUREMENT_INTERVAL_MS / 1000);

    // Stop advertising to save power
    esp_ble_gap_stop_advertising();

    // Disable Bluetooth to save power during sleep
    esp_bluedroid_disable();
    esp_bt_controller_disable();

    // Enter deep sleep for the measurement interval
    esp_sleep_enable_timer_wakeup(MEASUREMENT_INTERVAL_MS * 1000);  // Convert ms to μs

    // Actually enter deep sleep
    esp_deep_sleep_start();

    // Code after this will not execute until the chip wakes up from deep sleep
    // After waking up, the program will restart from app_main()
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting BTHome Bluetooth Temperature Sensor");

    // Check if we woke up from deep sleep
    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
    if (wakeup_cause == ESP_SLEEP_WAKEUP_TIMER) {
        ESP_LOGI(TAG, "Woke up from timer");
    } else {
        ESP_LOGI(TAG, "Not a timer wakeup, probably power-on reset");
    }

    // Create the temperature measurement task
    xTaskCreate(temperature_measurement_task, "temp_task", 4096, NULL, 5, NULL);
}