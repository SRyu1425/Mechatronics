#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define GPIO14 14
#define SERVO_WRAP 62500

void pwm_initialize() {
    printf("Initializing PWM\n");
    
    gpio_set_function(GPIO14, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(GPIO14);
    printf("PWM slice number: %d\n", slice_num);
    
    float div = 40;
    pwm_set_clkdiv(slice_num, div);
    printf("PWM clock divider set to: %.2f\n", div);
    
    pwm_set_wrap(slice_num, SERVO_WRAP);
    printf("PWM wrap set to: %d\n", SERVO_WRAP);
    
    pwm_set_enabled(slice_num, true);
    printf("PWM enabled\n");
    
    pwm_set_gpio_level(GPIO14, 31250); // Set duty cycle to 50%
    printf("PWM duty cycle set to 50%%\n");
}

int main() {
    stdio_init_all();
    sleep_ms(5000); // Give some time for the serial connection to be established

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
    printf("LED initialized\n");

    pwm_initialize();

    while (1) {
        sleep_ms(1000); // Keep the program running
    }
}
