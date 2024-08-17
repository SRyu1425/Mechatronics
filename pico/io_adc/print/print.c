#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

int main() {
    //initialize
    stdio_init_all();
    //while the usb is not connected, wait
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    //when connected (putty) print start
    printf("Start!\n");
 

    adc_init(); // init the adc module
    adc_gpio_init(26); // set ADC0 pin (GP26) to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0
 
    //stay in this loop and take in a string and then print it out when 'enter' is pressed
    while (1) {
        char message[100];
        scanf("%s", message);
        printf("message: %s\r\n",message);
        sleep_ms(50);
        uint16_t result = adc_read();
        printf("adc: %d\r\n",result);
    }
}
