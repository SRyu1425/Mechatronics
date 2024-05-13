/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"   // Include this for stdio_usb_connected





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

#define ADDR 0x68

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);


//initialize which gp pins are input/outputs
void mpu6050_init() {
    uint8_t buf[2];

    // send register number followed by its corresponding value
    //write 0x00 to the PWR_MGMT_1 register to turn the chip on
    buf[0] = PWR_MGMT_1;
    buf[1] = 0x00; //GP7 output, rest are inputs
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false); //function has start, stop bit included. however will be blocked if we don't get ack bit back

    //To enable the accelerometer, write to the ACCEL_CONFIG register. Set the sensitivity to plus minus 2g
    buf[0] = ACCEL_CONFIG;
    buf[1] = 0x00; //GP7 output, rest are inputs
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false);

    //To enable the accelerometer, write to the ACCEL_CONFIG register. Set the sensitivity to plus minus 2g
    buf[0] = GYRO_CONFIG;
    buf[1] = 0x18; //GP7 output, rest are inputs
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false);


}

void whoAmI_check(){
    uint8_t buffer[1];
    buffer[0] = WHO_AM_I;
    //write to who am i register
    i2c_write_blocking(i2c_default, ADDR, buffer, 1, true);

    i2c_read_blocking(i2c_default, ADDR, buffer, 1, false);  // check what you got back

    if (buffer[0] != 0x68){
        printf("Need reset!\n");
        while (1){
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
    i2c_write_blocking(i2c_default, ADDR, &val, 1, true); // true to keep master control of bus
    i2c_read_blocking(i2c_default, ADDR, buffer, 14, false);

    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]); 
    }

    // Now temperature for 2 bytes
    // The register is auto incrementing on each read
    *temp = (buffer[6] << 8 | buffer[7]);

    // Now gyro data for 6 bytes
    // The register is auto incrementing on each read
    for (int i = 0; i < 3; i++) {
        gyro[i] = (buffer[(i + 4)*2] << 8 | buffer[(i + 4)*2 + 1]);
    }

}


/*------------- MAIN -------------*/
int main(void)
{
  stdio_init_all();
  //while the usb is not connected, wait
  // while (!stdio_usb_connected()) {
  //     sleep_ms(100);
  // }
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

  
  board_init();
  tusb_init();

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    hid_task();
    
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;



  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      // gpio_put(16, 1);
      // sleep_ms(150);
      // gpio_put(16, 0);
      // sleep_ms(150);


      int8_t deltax;
      int8_t deltay;
      int16_t acceleration[3], gyro[3], temp;
      mpu6050_read_raw(acceleration, gyro, &temp);
      float x = gyro[0] * 0.007630; 
      float y = gyro[1] * 0.007630;

      if (x > -1 && x < 1){
        deltay = 0;
      } else if (x < -4){
        deltay = -10;
      } else if (x < -3){
        deltay = -8;
      } else if (x < -2){
        deltay = -5;
      } else if (x < -1){
        deltay = -3;
      } else if (x > 4){
        deltay = 10;
      } else if (x > 3){
        deltay = 8;
      } else if (x > 2){
        deltay = 5;
      } else if (x > 1){
        deltay = 3;
      }


      if (y > -1 && y < 1){
        deltax = 0;
      } else if (y < -4){
        deltax = -10;
      } else if (y < -3){
        deltax = -8;
      } else if (y < -2){
        deltax = -5;
      } else if (y < -1){
        deltax = -3;
      } else if (y > 4){
        deltax = 10;
      } else if (y > 3){
        deltax = 8;
      } else if (y > 2){
        deltax = 5;
      } else if (y > 1){
        deltax = 3;
      }


      // no button, right + down, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, deltax, deltay, 0, 0);

      //print to screen for debugginn
      printf("Gyro\n X = %.2f\n Y = %.2f\n", deltax, deltay); // units of degrees per sec

    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report =
      {
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

      if ( btn )
      {
        report.hat = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      }else
      {
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_MOUSE, btn);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
