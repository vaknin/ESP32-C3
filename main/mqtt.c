#include "mqtt_client.h"

// Publishes the current temp via MQTT to 'sensors/temperature'
esp_mqtt_client_handle_t set_up_mqtt(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .credentials.client_id = "ESP32C3",
        .broker.address.uri = "mqtt://192.168.1.167",
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_start(client);
    return client;
}