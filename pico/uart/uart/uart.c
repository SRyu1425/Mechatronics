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

volatile int y = 0;
volatile int x = 0;


//initialize which gp pins are input/outputs
void i2c_chip_init() {
    uint8_t buf[2];

    // send register number followed by its corresponding value
    buf[0] = REG_IODIR;
    buf[1] = 0b01111111; //GP7 output, rest are inputs
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false); //function has start, stop bit included. however will be blocked if we don't get ack bit back
}


void drawChar(int x, int y, uint8_t letter){
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
    ssd1306_update();
}

// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        //uart_putc(UART_ID, ch);
        if (ch == '\r' || ch == '\n'){
            x=0;
            y+=9;
        } else {
            drawChar(x,y,ch);
            x+=5;
        }
        
    }
}



int main() {

    sleep_ms(5000);
    stdio_init_all();
    //while the usb is not connected, wait
    // I2C is "open drain", pull ups to keep signal high when no data is being sent
    i2c_init(i2c_default, 500000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C); //set sda, scl pins
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    // i2c initialization
    i2c_chip_init();

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

    //ssd initialization
    ssd1306_setup();

    sleep_ms(250); // sleep so that data polling and register update don't collide
    while (1) {
        tight_loop_contents();
    }
}