#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/spi.h"

#define PI 3.14159265358979323846
static volatile float SineWaveform[200]; // sine waveform
static volatile float TriWaveform[200]; // triangle waveform
void makeSinWaveform();
void makeTriWaveform();


void makeSinWaveform() {
//make array of 100 data points that make up a period of a sine wave, and get through 200 data points in 1 sec to get 
//2 cycles per sec which is 2hz 
//get through 2 cycles in 200 data points 
    float amplitude = (3.3 - 0) / 2.0; // Half the range
    float offset = (3.3 + 0)/2; // Middle of the range
    float frequency = 2.0; // One cycle
    
    for (int i = 0; i < 200; i++) {
        // Calculate the sine value for 2 cycles over the size of the array
        float theta = (float)i / 200.0 * (2.0 * PI * 2.0);
        SineWaveform[i] = amplitude * sin(theta) + offset;
    }
}

//make array of 100 data points that make up a period of a sine wave, and get through 100 data points in 1 sec to get 
//1 cycles per sec which is 1hz 
//1 cycle in 200 data points 
void makeTriWaveform() {
    int halfSize = 200 / 2;
    float maxValue = 3.3;
    float rise = maxValue / halfSize; // Correct slope calculation

    // First half (rising edge)
    for (int i = 0; i < halfSize; ++i) {
        TriWaveform[i] = rise * i;
    }

    // Second half (falling edge)
    for (int i = halfSize; i < 200; ++i) {
        TriWaveform[i] = maxValue - rise * (i - halfSize);
    }
}


static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    asm volatile("nop \n nop \n nop");
}


static uint16_t calc_data(float volt){
    uint16_t data;
    int count = (int) (volt * (1023.0/3.3)); //convert voltage to 10bit number
    
    //buffered bit, 1x gain bit, active mode operation bit
    data = data | (0b111 << 12);
    //calculate digital count num for desired voltage
    data = data | (count << 2);

    return data;
}

//call this function in a loop while going through array of all the voltages i want to make
//have sleep in between every call that makes it go at 100hz
static void write_register(bool channel, uint16_t data){
    //channel = 0 is A, 1 is B
    data = data | channel << 15;
    uint8_t buf[2];
    buf[0] = data >> 8;  //first 8 bit num send 
    buf[1] = data;
    cs_select(); //makes pin to low
    spi_write_blocking(spi_default, buf, 2);
    cs_deselect();
}


int main() {
    //initialize
    stdio_init_all();
     
    // This example will use SPI0 at 0.5MHz.
    spi_init(spi_default, 1000000);
    //temp make baud rate slower for debugging ^
    //gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    //this goes to sck pin 
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);

    //make waveforms
    makeSinWaveform();
    makeTriWaveform();

    while (1){
        uint16_t data;
        for (int i = 0; i < 200; i++){
            data = calc_data(SineWaveform[i]);
            write_register(0, data); //channel a

            data = calc_data(TriWaveform[i]);
            write_register(1, data); //channel b

            sleep_ms(5); //200 data points in 1 sec = 1/200 = 5 ms
        }
    } 





}
