#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <stdlib.h> 
#define LEDPin 25 // the built in LED on the Pico
#define pwmPin1 14 //pin outputing pwm for AIN1
#define pwmPin2 15 //pin outputing pwm for BIN1
#define DIR_PIN1 27 // Direction pin for AIN2
#define DIR_PIN2 28 // Direction pin for BIN2
#define CENTER_LINE 50 // reference location of line relative to camera frame
#define tolerance 10 // center_line +- 10 means line is close enough to center and go at full speed
#define max_speed 1 //max speed of motors (0 to 1)
#define kp 0.01 // Proportional constant for the controller


uint16_t PWM_init(int pin){
    gpio_set_function(pin, GPIO_FUNC_PWM); // Set the inputted Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(pin); // Get PWM slice number
    float div = 1; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // divider
    uint16_t wrap = 6250; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, wrap); //frequency of the pwm will be 20KHz (standard for motor)
    pwm_set_enabled(slice_num, true); // turn on the PWM
    return wrap; //return wrap value to use in main function
}

int main(){
    
    //initialize
    stdio_init_all();
    //while the usb is not connected, wait
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    //when connected (putty) print start
    printf("Start!\n");
 
    gpio_init(DIR_PIN1);
    gpio_set_dir(DIR_PIN1, GPIO_OUT); // Direction pin for AIN2
    gpio_init(DIR_PIN2);
    gpio_set_dir(DIR_PIN2, GPIO_OUT); // Direction pin for BIN2


    //initialize the gpio pin 14, 15 (bottom left) to be pwm, and get wrap value
    //w1 = w2 = 6250
    uint16_t w1 = PWM_init(pwmPin1); 
    uint16_t w2 = PWM_init(pwmPin2); 

    while (1){
        // controller 
        // input a number representing where the line is relative to the camera frame
        //output duty cycles for motors

        //user input over usb
        int line;
        printf("Location of path (enter num 0 to 100): ");
        int ret = scanf("%d", &line);
        
        // Check if the input was successful
        if (ret != 1) {
            printf("Error reading input.\n");
            continue;
        }
        printf("\r\n");

        // Ensure the input is within the expected range
        if (line < 0) line = 0;
        if (line > 100) line = 100;

        int err = line - CENTER_LINE; //get error of line location (negative = to the left of center)
        float duty1; //duty cycle (float from 0 to 1)
        float duty2; //duty1 = left, duty2 = right

        if (abs(err) <= tolerance) {
            //err is within the tolerance, so continue going at max speed
            duty1 = max_speed;
            duty2 = max_speed;            
        } else if (err < 0){
            //line is to the left of the centerline
            //left motor goes slower
            //right motor continues at max speed
            duty1 = max_speed - kp * (abs(err) - tolerance); //(err - tolerance?) or just err
            duty2 = max_speed;
        } else if (err > 0){
            //line is to the right of the centerline
            //left motor continues at max speed
            //right motor goes slower
            duty1 = max_speed;
            duty2 = max_speed - kp * (abs(err) - tolerance); //(err - tolerance?) or just err
        }

        //duty cycle cannot be negative
        if (duty1 < 0){
            duty1 = 0;
        }

        if (duty2 < 0){
            duty2 = 0;
        }

        pwm_set_gpio_level(pwmPin1, (int) (duty1 * w1)); // set the duty cycle to inputted number (between 0 to wrap)
        gpio_put(DIR_PIN1, 0); // direction pin
        pwm_set_gpio_level(pwmPin2, (int) (duty2 * w2)); // set the duty cycle to inputted number (between 0 to wrap)
        gpio_put(DIR_PIN2, 0); // direction pin

        sleep_ms(100); // Add a delay to prevent overwhelming the serial communication
    }
}
