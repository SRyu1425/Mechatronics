/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
#include "pico/stdlib.h"

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
#endif
}
*/
#include "pico/stdlib.h"

// The LED is connected to GPIO 25
#define LED_PIN 25

// Main (runs on core 0)
int main() {
    // Initialize the LED pin
    gpio_init(LED_PIN);
    // Configure the LED pin as an output
    gpio_set_dir(LED_PIN, GPIO_OUT);
    // Loop
    while (true) {
        // Set high
        gpio_put(LED_PIN, 1);
        // Sleep
        sleep_ms(250);
        // Set low
        gpio_put(LED_PIN, 0);
        // Sleep
        sleep_ms(250);
    }
}
