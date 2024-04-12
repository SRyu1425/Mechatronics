#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/adc.h"


// The onboard pico LED is connected to GPIO 25
#define LED_PIN 25

    //GPIO PICO_DEFAULT_I2C_SDA_PIN (on Pico this is GP4 (pin 6)) -> SDA on BMP280
    //GPIO PICO_DEFAULT_I2C_SCK_PIN (on Pico this is GP5 (pin 7)) -> SCL on

// ssd device has address of 0b0111100
//7 bit number
#define ADDR 0b0111100

// hardware registers
#define REG_IODIR _u(0x00)
#define REG_GPIO _u(0x09)
#define REG_OLAT _u(0x0A)


//initialize which gp pins are input/outputs
void i2c_chip_init() {
    uint8_t buf[2];

    // send register number followed by its corresponding value
    buf[0] = REG_IODIR;
    buf[1] = 0b01111111; //GP7 output, rest are inputs
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false); //function has start, stop bit included. however will be blocked if we don't get ack bit back
}

void drawChar(int x, int y, char letter){
    //each letter is 5 pixels wide, 1 byte long (8 pixels)
    //iterate through each of the 5 columns of pixels
    int i, j;
    for (i = 0; i<5; i++){
        //c represents the column of pixels 
        //letter -32 b/c the font.h file only starts the ascii tables from the writeable letters
        char c = ASCII[letter-32][i];
        //for each of the 8 pixels, go down x,y plane to turn on/off pixels
        for (j = 0; j < 8; j++){
            //shift the bits in c appropriately and see if it is 1 or 0.  
            char bit = (c>>j) & 0b1;
            if (bit == 0b1){
                ssd1306_drawPixel(x+i, y+j, 1);
            } else {
                ssd1306_drawPixel(x+i, y+j, 0);
            }
        }

    }
}

void drawMessage(int x, int y, char* m){
    int i = 0;
    //while the char is not the null character
    while (m[i]){
        //each char is 5 pixels wide, so shift by 5
        drawChar(x+i*5, y, m[i]);
        i++;
    }
    //send updated character array and update
    ssd1306_update();

}

int main() {
    stdio_init_all();
    //while the usb is not connected, wait
   
   
    adc_init(); // init the adc module
    adc_gpio_init(26); // set ADC0 pin (GP26) to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0

    // Initialize the LED pin
    gpio_init(LED_PIN);
    // Configure the LED pin as an output
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // I2C is "open drain", pull ups to keep signal high when no data is being sent
    i2c_init(i2c_default, 500000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C); //set sda, scl pins
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);

    // i2c initialization
    i2c_chip_init();

    //ssd initialization
    ssd1306_setup();

    sleep_ms(250); // sleep so that data polling and register update don't collide
    while (1) {
        //blink gp25 - if it stops blinking, couldn't get through all of stuff, or init function
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);

        char message[50];

        //test to see if pixel blinks

        /*
        ssd1306_drawPixel(10, 10, 0);
        ssd1306_update();
        sleep_ms(500);

        ssd1306_drawPixel(10, 10, 1);
        ssd1306_update();
        sleep_ms(500);
        */


        //draw the letter h

        /*
        printf(message, "hello %d", i);
        drawChar(64,12,message[0]); //draw h
        ssd1306_update();
        sleep_ms(500);
        */
        


        //timing - returns microsecond time
        uint16_t result = adc_read(); //read adc count
        double volt = result * (3.3 / 4095); //convert to volts
        sprintf(message, "Voltage: %.2f Volts", volt);
        u_int64_t start = to_us_since_boot(get_absolute_time());
        drawMessage(25, 3, message);
        u_int64_t stop = to_us_since_boot(get_absolute_time());
        u_int64_t elapsed_time = stop - start;
        sprintf(message, "FPS = %.2f",  1000000.0/elapsed_time);
        drawMessage(25, 15, message);
    }
}
