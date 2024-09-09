#include <stdio.h>
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

#define GPIO14 14
#define GPIO15 15
#define SERVO_WRAP 62500

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1
#define UART_TX_PIN 0
#define UART_RX_PIN 1
// ssd device has address of 0b0111100
// 7 bit number
#define ADDR 0b0111100

// hardware registers
#define REG_IODIR _u(0x00)
#define REG_GPIO _u(0x09)
#define REG_OLAT _u(0x0A)

char rxmessage[100];
volatile int i = 0;

int Kp = 2;
int left_duty = 100;
int right_duty = 100;
int pid_control(int current, int desired);

// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        if (ch == '\r' || ch == '\n'){
            rxmessage[i] = 0;
            i = 0;
            int cen;
            sscanf(rxmessage, "%d", &cen);
            if (cen == -1) {
                left_duty = 0;
                right_duty = 0;
            } else {
                left_duty = calc_left_duty_cycle(cen);
                right_duty = calc_right_duty_cycle(cen);
            }
            printf("Input %d. Setting left to %d percent and right to %d percent.\r\n", cen, left_duty, right_duty);
            pwm_set_duty_cycle(14, left_duty);
            pwm_set_duty_cycle(15, right_duty);
        } else {
            rxmessage[i] = ch;
            i++;
        }
        
    }
}

int calc_left_duty_cycle(int cen) {
    if (cen >= 50) {
        return 100;
    }
    return 100 - Kp*(50-cen);
}

int calc_right_duty_cycle(int cen) {
    if (cen <= 50) {
        return 100;
    }
    return 100 - Kp*(cen-50);
}

int pid_control(int current, int desired) {
    int error = current - desired;
    int new = Kp*error + current;
    if (new < 0) {
        new = 0;
    } else if (new > 100) {
        new = 100;
    }
    return new;
}

void pwm_initialize() {
    gpio_set_function(GPIO14, GPIO_FUNC_PWM); // Set the GPIO14 to be PWM
    uint slice_num = pwm_gpio_to_slice_num(GPIO14); // Get PWM slice number
    float div = 40; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // divider
    uint16_t wrap = SERVO_WRAP; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM
    pwm_set_gpio_level(GPIO14, 0); // set the duty cycle to 100%

    gpio_set_function(GPIO15, GPIO_FUNC_PWM); // Set the GPIO15 to be PWM
    slice_num = pwm_gpio_to_slice_num(GPIO15); // Get PWM slice number
    pwm_set_clkdiv(slice_num, div); // divider
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM
    pwm_set_gpio_level(GPIO15, 0); // set the duty cycle to 100%
}

void pwm_set_duty_cycle(int pin, int cycle) {
    pwm_set_gpio_level(pin, (cycle*SERVO_WRAP)/100);
}

int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
    pwm_initialize();
    
    stdio_init_all();

    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, 2400);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, false);
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);
    uart_puts(UART_ID, "\nHello, uart interrupts\n");

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");
    pwm_set_duty_cycle(14, 100);
    pwm_set_duty_cycle(15, 100);
 
    while (1) {
        ;
    }
}