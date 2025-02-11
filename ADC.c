#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "hardware/clocks.h"

#define LED_PIN_GREEN 11
#define LED_PIN_BLUE 12
#define LED_PIN_RED 13
#define BOTAO_A 5 //pino saida botao a
#define BOTAO_B 6 //pino saida botao b
#define I2C_PORT i2c1 //porta I2C
#define I2C_SDA 14 //pino SDA
#define I2C_SCL 15 //pino SCL
#define display_address 0x3C //endereço do display

int main()
{
    stdio_init_all();

    while (true) {
       
    }
}
