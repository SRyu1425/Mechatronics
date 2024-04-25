#include <stdio.h>
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/uart.h"
#include "hardware/irq.h"



#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1
// ssd device has address of 0b0111100
//7 bit number
#define ADDR 0b0111100

// hardware registers
#define REG_IODIR _u(0x00)
#define REG_GPIO _u(0x09)
#define REG_OLAT _u(0x0A)

char rxmessage[100];
volatile int i = 0;



// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        //uart_putc(UART_ID, ch);
        if (ch == '\r' || ch == '\n'){
            rxmessage[i] = 0;
            i = 0;
            printf("Message from Zero: %s\n", rxmessage);
        } else {
            rxmessage[i] = ch;
            i++;
        }
        
    }
}



int main() {

    sleep_ms(5000);
    stdio_init_all();
    

   // Set up our UART with a basic baud rate.
    uart_init(UART_ID, 2400);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt  - interrupts when uart chip receives letter b/c it will take time after sending 
    //until the chip actually receives it.
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    // OK, all set up.
    // Lets send a basic string out, and then run a loop and wait for RX interrupts
    // The handler will count them, but also reflect the incoming data back with a slight change!
    uart_puts(UART_ID, "\nHello, uart interrupts\n");


    while (!stdio_usb_connected()){
        sleep_ms(100);
    }

    printf("Start!\n");

    sleep_ms(250); // sleep so that data polling and register update don't collide
    while (1) {
        //blocking scan input number from computer and confirm
        int iFromComputer = 0;
        scanf("%d", &iFromComputer);
        printf("Computer sent: %d\n", iFromComputer);
        
        //put input from computer into a message you will send out
        char txmessage[100];
        sprintf(txmessage, "%d\n", iFromComputer);
        uart_puts(UART_ID, txmessage); //spits out through tx pin

        sleep_ms(250);
    }
}