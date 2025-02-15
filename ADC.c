#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"     


#define BOTAO_A 5 //pino saida botao a
#define BOTAO_B 6 //pino saida botao b
#define LED_PIN_GREEN 11 //pino saida led verde
#define LED_PIN_BLUE 12 //pino saida led azul
#define LED_PIN_RED 13  //pino saida led vermelho
#define I2C_SDA 14 //pino SDA
#define I2C_SCL 15 //pino SCL
#define SW_PIN 22 //pino do botão SW JOYSTICK
#define VRX_PIN 26   //pino do eixo X do joystick
#define VRY_PIN 27   //pino do eixo Y do joystick
#define I2C_PORT i2c1 //porta I2C
#define display_address 0x3C //endereço do display

uint actual_time = 0;
bool joystick_mode = false;

uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);

    pwm_set_enabled(slice_num, true);
    return slice_num;
}

void callback_abtn(uint gpio, uint32_t events) {
    uint time = to_ms_since_boot(get_absolute_time());
    if (time - actual_time > 300) {
        actual_time = time;
        if (gpio == BOTAO_A){
            joystick_mode = !joystick_mode;
            printf("Joystick: %s\n", joystick_mode ? "Led Desabilitado" : " Led Habilitado");
       
            if (joystick_mode){
                pwm_set_gpio_level(LED_PIN_RED, 0);
                pwm_set_gpio_level(LED_PIN_BLUE, 0);
                gpio_put(LED_PIN_GREEN, false);
            }
            
            // printf("Botão A\n");
        }
        

    }
    
}

int main(){
    stdio_init_all();
    adc_init();
    adc_gpio_init(VRX_PIN); 
    adc_gpio_init(VRY_PIN); 
    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN); 
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_put(LED_PIN_GREEN, false); 
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_put(LED_PIN_RED, false); 
    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
    gpio_put(LED_PIN_BLUE, false);
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    
    uint pwm_wrap = 4096;  
    uint red_pwm_slice = pwm_init_gpio(LED_PIN_RED, pwm_wrap);  
    uint blue_pwm_slice = pwm_init_gpio(LED_PIN_BLUE, pwm_wrap);  
    uint32_t last_print_time = 0; 
    
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &callback_abtn);  

    while (true) {
        
        if (!joystick_mode) {
             adc_select_input(0); 
             uint16_t vrx_value = adc_read(); 
     
             adc_select_input(1); 
             uint16_t vry_value = adc_read(); 
             
             int blue_level = 0;
             int red_level = 0;
             bool sw_value = gpio_get(SW_PIN) == 0; 
             if (vrx_value > 2400) {
                 red_level = vrx_value - 2400; 
             } else if (vrx_value < 1700){
                 red_level = 1700 - vrx_value;
             }
             
             if (vry_value > 2400) {
                 blue_level = vry_value - 2400; 
             } else if (vry_value < 1700){
                 blue_level = 1700 - vry_value;
             }
             
             if (sw_value) { 
                 gpio_put(LED_PIN_GREEN, true); 
             } else {
                 gpio_put(LED_PIN_GREEN, false);
             }
     
             pwm_set_gpio_level(LED_PIN_RED, red_level);
             pwm_set_gpio_level(LED_PIN_BLUE, blue_level);
     
     
             uint32_t current_time = to_ms_since_boot(get_absolute_time());
             if (current_time - last_print_time > 1000) {
             //    printf("VRX: %u, VRY: %u, SW: %d\n", vrx_value, vry_value, sw_value);
                 last_print_time = current_time;
             }
             

         }
         
        
        
        sleep_ms(100);
    }
    return 0;
}
