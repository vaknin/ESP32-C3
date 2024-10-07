#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"

#include "servo.c"
#include "wifi.c"
#include "common.h"
#include "mqtt.c"
#include "temp.c"

#define BUTTON_GPIO 3  // GPIO pin connected to the button
#define DEBOUNCE_TIME_MS 200  // Debounce time in milliseconds

static TimerHandle_t debounce_timer;  // Timer handle for debounce
volatile bool button_pressed = false;  // Flag to signal button press outside ISR

esp_mqtt_client_handle_t setup()
{
	// Event loop
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Connect to WIFI
	connect_to_wifi();
	ESP_LOGI(TAG, "- CONNECTED TO WIFI");

	// Connect to MQTT
	esp_mqtt_client_handle_t client = set_up_mqtt();
	ESP_LOGI(TAG, "- CONNECTED TO MQTT");

	// Temperature
	set_up_temp_sensor();

	// Initialize the servo
	init_servo();

	return client;

}

// ISR handler for the button
void IRAM_ATTR button_isr_handler(void* arg) {
    // Disable further interrupts from the button GPIO
    gpio_isr_handler_remove(BUTTON_GPIO);
    
    // Set the flag or process the button press
    button_pressed = true;
    
    // Start the debounce timer
    xTimerStartFromISR(debounce_timer, NULL);
}

// Timer callback to re-enable the button interrupt after debouncing
void debounce_timer_callback(TimerHandle_t xTimer) {
    // Re-enable the button interrupt after debounce delay
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);
}

void app_main()
{
	init_servo();
	
	gpio_config_t io_conf = {
    .intr_type = GPIO_INTR_NEGEDGE,  // Falling edge triggers the ISR (button press)
    .mode = GPIO_MODE_INPUT,         // Input mode
    .pin_bit_mask = (1ULL << BUTTON_GPIO),  // GPIO 3
    .pull_up_en = GPIO_PULLUP_ENABLE // Enable internal pull-up resistor
	};
	gpio_config(&io_conf);//

	// Create debounce timer (one-shot, not repeating)
    debounce_timer = xTimerCreate("debounce_timer", pdMS_TO_TICKS(DEBOUNCE_TIME_MS), pdFALSE, (void*)0, debounce_timer_callback);
    
    // Install ISR service
    gpio_install_isr_service(0);  // 0 means no flags
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);  // Add the ISR for the button GPIO

    while (1) {
        if (button_pressed) {
            // Reset the flag
            button_pressed = false;

            // Now safely log the button press or handle it
            ESP_LOGI("APP_MAIN", "Button was pressed!");
            open_flap();  // Open the flap
        }

        // Simulating doing other tasks
        vTaskDelay(pdMS_TO_TICKS(100));  // Small delay to simulate other tasks
    }

	// esp_mqtt_client_handle_t client = setup();

	// char *temperature;
	// while (1)
	// {
	// 	// open_flap();
	// 	// vTaskDelay(3000 / portTICK_PERIOD_MS);
	// 	// temperature = get_temp();
	// 	// esp_mqtt_client_publish(client, "sensors/temperature", temperature, 0, 1, 0);
	// }
}