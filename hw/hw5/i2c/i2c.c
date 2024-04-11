#include <stdio.h>

#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"


// The onboard pico LED is connected to GPIO 25
#define LED_PIN 25

    //GPIO PICO_DEFAULT_I2C_SDA_PIN (on Pico this is GP4 (pin 6)) -> SDA on BMP280
    //GPIO PICO_DEFAULT_I2C_SCK_PIN (on Pico this is GP5 (pin 7)) -> SCL on

// i2c device chip has default bus address of 0b0100 (fixed) + 0b000 (grounded a0, a1, a2)
//7 bit number
#define ADDR _u(0b0100000)

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

//sets any pin high or low
void setPin(unsigned char address, unsigned char reg, unsigned char value) {
    uint8_t buf[2];

    // send register number followed by its corresponding value
    buf[0] = reg;
    buf[1] = value; 
    i2c_write_blocking(i2c_default, address, buf, 2, false); //function has start, stop bit included. however will be blocked if we don't get ack bit back

}

unsigned char readPin(unsigned char address, unsigned char reg) {
   
    uint8_t buf[1];
    i2c_write_blocking(i2c_default, ADDR, &reg, 1, true);  // true means after writing, the pico says its not done talking
    i2c_read_blocking(i2c_default, ADDR, buf, 1, false);  // read into the buf

    if (buf[0] & 0b1 == 0b1){
        return 1; //the pin is high
    } else {
        return 0;
    }
}


int main() {
    stdio_init_all();

    // Initialize the LED pin
    gpio_init(LED_PIN);
    // Configure the LED pin as an output
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // I2C is "open drain", pull ups to keep signal high when no data is being sent
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C); //set sda, scl pins
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);


    // chip initialization
    i2c_chip_init();

    sleep_ms(250); // sleep so that data polling and register update don't collide
    while (1) {
        //blink gp25 - if it stops blinking, couldn't get through all of stuff, or init function
        gpio_put(LED_PIN, 1);
        sleep_ms(150);
        gpio_put(LED_PIN, 0);
        sleep_ms(150);

        
        // read from gp0 
        if (readPin(ADDR, REG_GPIO) == 1){
            setPin(ADDR, REG_OLAT, 0b00000000);
        } else if (readPin(ADDR, REG_GPIO) == 0){
            setPin(ADDR, REG_OLAT, 0b10000000);
        }
    }
}
