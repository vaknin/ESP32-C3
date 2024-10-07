#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define SERVO_GPIO_PIN 5  // GPIO pin connected to your servo
#define LEDC_TIMER       LEDC_TIMER_0
#define LEDC_MODE        LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO   (SERVO_GPIO_PIN)   // Output GPIO
#define LEDC_CHANNEL     LEDC_CHANNEL_0
#define LEDC_DUTY_RES    LEDC_TIMER_14_BIT  // Resolution of PWM duty (14-bit)
#define LEDC_FREQUENCY   (50)               // Frequency in Hertz (50Hz for servos)
#define SERVO_MIN_PULSEWIDTH_US 500         // Minimum pulse width in microseconds (0 degrees)
#define SERVO_MAX_PULSEWIDTH_US 2500        // Maximum pulse width in microseconds (180 degrees)
#define SERVO_MAX_DEGREE        180         // Maximum angle in degrees
#define FLAP_OPEN_ANGLE         0          // Define angle to open flap
#define FLAP_CLOSE_ANGLE        42           // Define angle to close flap
#define FLAP_OPEN_DURATION_MS   500        // Time to wait after opening the flap in milliseconds

// Convert angle in degrees to duty cycle for LEDC
static uint32_t servo_angle_to_duty(uint32_t angle)
{
    // Calculate pulse width based on the angle
    uint32_t pulse_width = SERVO_MIN_PULSEWIDTH_US + 
                           (((SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) * angle) / SERVO_MAX_DEGREE);
    // Convert pulse width to duty cycle (16,384 max duty for 14-bit resolution)
    return (pulse_width * (1 << LEDC_DUTY_RES)) / 20000;  // 20000us = 20ms period (50Hz)
}

// Function to initialize the servo using LEDC
void init_servo(void)
{
    // Configure LEDC timer
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_DUTY_RES, // PWM duty resolution (14-bit)
        .freq_hz = LEDC_FREQUENCY,        // Frequency of PWM signal (50Hz for servos)
        .speed_mode = LEDC_MODE,          // LEDC speed mode (low speed)
        .timer_num = LEDC_TIMER           // Timer number
    };
    ledc_timer_config(&ledc_timer);

    // Configure LEDC channel
    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL,
        .duty       = 0,                  // Initially set duty to 0
        .gpio_num   = LEDC_OUTPUT_IO,      // GPIO pin connected to the servo
        .speed_mode = LEDC_MODE,           // Speed mode (low speed)
        .hpoint     = 0,                  // Start at 0
        .timer_sel  = LEDC_TIMER           // Timer selected
    };
    ledc_channel_config(&ledc_channel);
}

// Function to move servo to a specific angle
void move_servo_to_angle(uint32_t angle)
{
    uint32_t duty = servo_angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);  // Set the PWM duty cycle
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);     // Update duty to apply the change
}

// Function to open and then close the flap
void open_flap(void)
{
    // Move to the open position
    move_servo_to_angle(FLAP_OPEN_ANGLE);
    vTaskDelay(pdMS_TO_TICKS(FLAP_OPEN_DURATION_MS));  // Wait for the specified duration

    // Close the flap after the duration
    move_servo_to_angle(FLAP_CLOSE_ANGLE);
}