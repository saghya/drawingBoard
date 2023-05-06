#ifndef __RG12864B_H
#define __RG12864B_H

#include "main.h"

#define LCD_DB0_Pin       GPIO_PIN_5
#define LCD_DB0_GPIO_Port GPIOB
#define LCD_DB1_Pin       GPIO_PIN_4
#define LCD_DB1_GPIO_Port GPIOB
#define LCD_DB2_Pin       GPIO_PIN_10
#define LCD_DB2_GPIO_Port GPIOB
#define LCD_DB3_Pin       GPIO_PIN_8
#define LCD_DB3_GPIO_Port GPIOA
#define LCD_DB4_Pin       GPIO_PIN_9
#define LCD_DB4_GPIO_Port GPIOA
#define LCD_DB5_Pin       GPIO_PIN_7
#define LCD_DB5_GPIO_Port GPIOC
#define LCD_DB6_Pin       GPIO_PIN_6
#define LCD_DB6_GPIO_Port GPIOB
#define LCD_DB7_Pin       GPIO_PIN_7
#define LCD_DB7_GPIO_Port GPIOA

#define LCD_CS1_Pin       GPIO_PIN_8
#define LCD_CS1_GPIO_Port GPIOB
#define LCD_CS2_Pin       GPIO_PIN_8
#define LCD_CS2_GPIO_Port GPIOC

#define LCD_D_I_Pin          GPIO_PIN_9
#define LCD_D_I_GPIO_Port    GPIOB
#define LCD_R_W_Pin          GPIO_PIN_5
#define LCD_R_W_GPIO_Port    GPIOA
#define LCD_RST_Pin          GPIO_PIN_6
#define LCD_RST_GPIO_Port    GPIOA
#define xLCD_EN_Pin          GPIO_PIN_12
#define xLCD_EN_GPIO_Port    GPIOB
#define LCD_E_Pin            GPIO_PIN_6
#define LCD_E_GPIO_Port      GPIOC
#define LCD_BL_PWM_Pin       GPIO_PIN_9
#define LCD_BL_PWM_GPIO_Port GPIOC

#define LCD_WIDTH  128
#define LCD_HEIGHT 64

#define LCD_ON    0b00111111
#define LCD_OFF   0b00111110
#define PAGE_BASE 0b10111000
#define X_BASE    0b01000000
#define Z_BASE    0b11000000

#define LCD_READ       GPIO_PIN_SET
#define LCD_WRITE      GPIO_PIN_RESET
#define LCD_BRIGHTNESS TIM8->CCR4

extern TIM_HandleTypeDef htim8; // timer used for brightness

void LCD_Init();
void LCD_Clear();
void LCD_SetBrightness(uint16_t b);
void LCD_SetPos(uint8_t x, uint8_t y);
void LCD_SetPixel(uint8_t x, uint8_t y);
void LCD_ResetPixel(uint8_t x, uint8_t y);
void LCD_TogglePixel(uint8_t x, uint8_t y);
void LCD_DrawChar(char c, uint8_t x, uint8_t p);
void LCD_DrawString(char *s, uint8_t x, uint8_t p);
void LCD_DrawCharInverse(char c, uint8_t x, uint8_t p);
void LCD_DrawStringInverse(char *s, uint8_t x, uint8_t y);

#endif // __RG12864B_H
