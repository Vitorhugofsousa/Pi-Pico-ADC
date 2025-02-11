#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"

#define LED_PIN_GREEN 11
#define LED_PIN_BLUE 12
#define LED_PIN_RED 13
#define BOTAO_A 5 //pino saida botao a
#define BOTAO_B 6 //pino saida botao b
#define I2C_PORT i2c1 //porta I2C
#define I2C_SDA 14 //pino SDA
#define I2C_SCL 15 //pino SCL
#define display_address 0x3C //endereço do display

const int VRX = 26;
const int VRY = 27;
const int ADC_CHANNEL_0 = 0;
const int ADC_CHANNEL_1 = 1;
const int SW = 22;
const float DIVISOR = 16.0;
const uint16_t PERIODO = 4096;
uint16_t red_led_level, green_led_level, blue_led_level;
uint slice_red, slice_green, slice_blue;


void setup_pwm_led(uint led, uint *slice, uint level){
    gpio_set_function(led, GPIO_FUNC_PWM);
    *slice = pwm_gpio_to_slice_num(led);
    pwm_set_clkdiv(*slice, DIVISOR);
    pwm_set_wrap(*slice, PERIODO);
    pwm_set_gpio_level(led, level);
    pwm_set_enabled(*slice, true);
}

void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value){
    adc_select_input(ADC_CHANNEL_0);
    sleep_us(2);
    *vrx_value = adc_read();
    adc_select_input(ADC_CHANNEL_1);
    sleep_us(2);
    *vry_value = adc_read();
}


int main(){
    stdio_init_all();
    adc_init();
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);
    setup_pwm_led(LED_PIN_RED, &slice_red, 0);
    setup_pwm_led(LED_PIN_GREEN, &slice_green, 0);
    setup_pwm_led(LED_PIN_BLUE, &slice_blue, 0);



    uint16_t vrx_value, vry_value;
    printf("Iniciando...\n");

    while (true) {
    joystick_read_axis(&vrx_value, &vry_value);
    pwm_set_gpio_level(LED_PIN_RED, vrx_value);   
    pwm_set_gpio_level(LED_PIN_GREEN, vry_value);
    pwm_set_gpio_level(LED_PIN_BLUE, gpio_get(SW));
        
    sleep_ms(100);
    }
}
