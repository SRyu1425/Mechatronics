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

    gpio_init(27); //init gpio 27, 28
    gpio_set_dir(27, GPIO_OUT); //connected to led
    gpio_init(28);
    gpio_set_dir(28, GPIO_IN); //reads hi or lo from switch
    

    //stay in this loop and ask user to enter num of samples to take (1-100)
    while (1) {
        gpio_put(27, 1); //turn led on

        while (gpio_get(28) != 1){} //wait in this loop until switch is pressed
        gpio_put(27,0); //led off

        //get input of num of analog samples to take (1-100)
        char message[100];
        int samp;
        printf("How many analog samples? (enter num 1 to 100): ");
        scanf("%d", &samp);
        printf("\r\n");
        sleep_ms(50);

        for (int i = 1; i<=samp; i++){
            uint16_t result = adc_read(); //read adc count
            double volt = result * (3.3 / 4095); //convert to volts
            printf("adc sample #%d = %.3f volts\r\n",i, volt); //display
            sleep_ms(10); //wait 10ms (100Hz sampling rate)
        }
        
    }
}
