#include "esp_log.h"
#include "driver/temperature_sensor.h"
#include "common.h"

temperature_sensor_handle_t temp_sensor = NULL;

void set_up_temp_sensor(void)
{
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_sensor));
    ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor));
}

// Return current temp as a string
char* get_temp(void)
{
    float tsens_value;
    ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_sensor, &tsens_value));

    char* temperature_str;
    asprintf(&temperature_str, "%f", tsens_value);
    return temperature_str;
}