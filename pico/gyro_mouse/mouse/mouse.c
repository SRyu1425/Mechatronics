#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#define LED_PIN 25

// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75

//i2c address
#define ADDR 0x68


// static void mpu6050_reset() {
//     // Two byte reset. First byte register, second byte data
//     // There are a load more options to set up the device in different ways that could be added here
//     uint8_t buf[] = {0x6B, 0x80};
//     i2c_write_blocking(i2c_default, ADDR, buf, 2, false);
// }

//initialize which gp pins are input/outputs
void mpu6050_init() {
    uint8_t buf[2];

    // send register number followed by its corresponding value
    buf[0] = PWR_MGMT_1;
    buf[1] = 0x00;     //write 0x00 to the PWR_MGMT_1 register to turn the chip on
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false); //function has start, stop bit included. however will be blocked if we don't get ack bit back

    //To enable the accelerometer, write to the ACCEL_CONFIG register. 
    buf[0] = ACCEL_CONFIG;
    buf[1] = 0x00; //Set the sensitivity to plus minus 2g
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false);

    //To enable the accelerometer, write to the ACCEL_CONFIG register.
    buf[0] = GYRO_CONFIG;
    buf[1] = 0x18;  // Set the sensitivity to plus minus 2000 dps.
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false);


}

void whoAmI_check(){
    uint8_t buffer[1];
    buffer[0] = WHO_AM_I;
    //write to who am i register
    i2c_write_blocking(i2c_default, ADDR, buffer, 1, true);

    i2c_read_blocking(i2c_default, ADDR, buffer, 1, false);  // check what you got back

    //the whoami register should return 0x68, and if it doesn't flash lights to indicate reset
    if (buffer[0] != 0x68){
        printf("Need reset!\n");
        while (1){
            //flash gpio 16 led
            gpio_put(16, 1);
            sleep_ms(150);
            gpio_put(16, 0);
            sleep_ms(150);
        }
    }
}


static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp) {
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.
    //burst read by reading all 14 bytes in a row. everything we want is sequentially right next to each other

    uint8_t buffer[14];

    // Start reading acceleration registers from register 0x3B for 6 bytes
    uint8_t val = ACCEL_XOUT_H;
    i2c_write_blocking(i2c_default, ADDR, &val, 1, true); //write to first register, true to keep control
    i2c_read_blocking(i2c_default, ADDR, buffer, 14, false); //read in 14 bytes to buffer array

    //first get 6 bytes of accel values
    //save into accel array (unsigned int)
    //must take first byte and shift left 8 and or to get the proper 16bit accel value (x,y,z)
    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]); 
    }

    // Now temperature for 2 bytes
    *temp = (buffer[6] << 8 | buffer[7]);

    // Now gyro data for 6 bytes
    // The register is auto incrementing on each read
    //same thing with accel, manipulate bits for proper data for gyro
    for (int i = 0; i < 3; i++) {
        gyro[i] = (buffer[(i + 4)*2] << 8 | buffer[(i + 4)*2 + 1]);
    }

}


int main() {
    stdio_init_all();
    //while the usb is not connected, wait
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    //when connected (putty) print start
    printf("Start!\n");

    // mpu6050_reset();

    // Initialize the LED pin
    gpio_init(LED_PIN);
    // Configure the LED pin as an output
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Initialize random gpio16, led pin
    gpio_init(16);
    // Configure the LED pin as an output
    gpio_set_dir(16, GPIO_OUT);

    // I2C is "open drain", pull ups to keep signal high when no data is being sent
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C); //set sda, scl pins
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    

    
    // chip initialization
    mpu6050_init();

    //read from WHO_AM_I and check that you get the right value back 
    //will check that your I2C bus is working and the chip is plugged in and ready to communicate
    //If you don't get the right value back
    //go into an infinite loop and turn on your LEDs so that you know you need a power reset
    whoAmI_check();


    sleep_ms(250); // sleep so that data polling and register update don't collide

    int16_t acceleration[3], gyro[3], temp; //16 bit int data type


    while (1) {
        //blink gp25 - if it stops blinking, couldn't get through all of stuff, or init function
        // gpio_put(LED_PIN, 1);
        // sleep_ms(150);
        // gpio_put(LED_PIN, 0);
        // sleep_ms(150);

        
        mpu6050_read_raw(acceleration, gyro, &temp);

        printf("Acceleration\n X = %.2f\n Y = %.2f\n Z = %.2f\n", acceleration[0] * 0.000061, acceleration[1] * 0.000061, acceleration[2] * 0.000061); //units of g
        printf("Gyro\n X = %.2f\n Y = %.2f\n Z = %.2f\n", gyro[0] * 0.007630, gyro[1] * 0.007630, gyro[2] * 0.007630); // units of degrees per sec
        // Temperature is simple so use the datasheet calculation to get deg C.
        // Note this is chip temperature.
        printf("Temp. = %f\n", (temp / 340.0) + 36.53);
 
        sleep_ms(10); //100 hz
    }
}