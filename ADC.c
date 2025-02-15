#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"     
#include "lib/font.h"
#include "lib/ssd1306.h"

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
bool led_mode = false;
bool frame_mode = false;
bool display_mode = true;
ssd1306_t ssd;

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
            led_mode = !led_mode;
            printf("Joystick: %s\n", led_mode ? "Led Desabilitado" : " Led Habilitado");
            if (led_mode){
                pwm_set_gpio_level(LED_PIN_RED, 0);
                pwm_set_gpio_level(LED_PIN_BLUE, 0);
                gpio_put(LED_PIN_GREEN, false);
            }
        }
        
        if (gpio == BOTAO_B){
            display_mode = !display_mode;            
            printf("Joystick: %s\n", display_mode ? "Display Habilitado" : "Display Desabilitado");
            
            if (!display_mode){
                ssd1306_fill(&ssd, false); // Limpa o display
                ssd1306_send_data(&ssd);
            }
        }
        
        if (gpio == SW_PIN){
            frame_mode = !frame_mode;
            printf("Joystick: %s\n", frame_mode ? "Frame Mode Habilitado" : "Frame Mode Desabilitado");
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
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, display_address, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    
    uint pwm_wrap = 4096;  
    uint red_pwm_slice = pwm_init_gpio(LED_PIN_RED, pwm_wrap);  
    uint blue_pwm_slice = pwm_init_gpio(LED_PIN_BLUE, pwm_wrap);  
    uint32_t last_print_time = 0; 
    
    uint16_t vrx_value;
    uint16_t vry_value;
    char str_x[5];
    char str_y[5];
    
    bool cor = true;
    
    int quadrado_x = 30; // Posição inicial x do quadrado
    int quadrado_y = 30; // Posição inicial y do quadrado
    int quadrado_largura = 12;
    int quadrado_altura = 7;
    
    gpio_set_irq_enabled_with_callback(SW_PIN, GPIO_IRQ_EDGE_FALL, true, &callback_abtn);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &callback_abtn);  
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &callback_abtn);  

    while (true) {
        adc_select_input(0); 
        vrx_value = adc_read(); 
        
        adc_select_input(1); 
        vry_value = adc_read(); 
        
        bool sw_value = gpio_get(SW_PIN) == 0; 
        if (!led_mode) {
            int blue_level = 0;
            int red_level = 0;
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
        }
            
        if (display_mode){
                uint16_t vrx_value_inverter = 4095 - vrx_value;
                quadrado_x = 30 + (vrx_value_inverter - 2048) / 100; // Ajuste o divisor para controlar a sensibilidade
                quadrado_y = 62 + (vry_value - 2048) / 100; // Ajuste o divisor para controlar a sensibilidade
                
                // Limites para o quadrado não sair da tela
            if (quadrado_x < 3) quadrado_x = 3;
            if (quadrado_x > 125 - quadrado_largura) quadrado_x = 125 - quadrado_largura;
            if (quadrado_y < 3) quadrado_y = 3;
            if (quadrado_y > 90 - quadrado_altura) quadrado_y = 90 - quadrado_altura;
            
            
            ssd1306_fill(&ssd, !cor); // Limpa o display
            if (frame_mode){
                quadrado_largura = 8;
                quadrado_altura = 7;
                ssd1306_rect(&ssd, 5, 5, 118, 56, cor, !cor); // Desenha um retângulo
                ssd1306_rect(&ssd, 7, 7, 114, 52, cor, !cor); // Desenha um retângulo
                ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor); // Desenha um retângulo
                ssd1306_rect(&ssd, quadrado_x, quadrado_y, quadrado_largura, quadrado_altura, cor, 0); // Desenha o quadrado
            }else{
                quadrado_largura = 13;
                quadrado_altura = 3;
                ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor); // Desenha um retângulo
                ssd1306_rect(&ssd, quadrado_x, quadrado_y, quadrado_largura, quadrado_altura, cor, 1); // Desenha o quadrado
            }
            
            ssd1306_send_data(&ssd); // Atualiza o display
        }
            
            
            sleep_ms(100);
        }
        return 0;
    }
    