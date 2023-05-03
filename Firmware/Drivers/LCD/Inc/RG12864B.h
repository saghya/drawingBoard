#ifndef __RG12864B_H
#define __RG12864B_H

#include "main.h"

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
inline void LCD_SetBrightness(uint16_t b);
void LCD_SetPos(uint8_t x, uint8_t y);
void LCD_SetPixel(uint8_t x, uint8_t y);
void LCD_ResetPixel(uint8_t x, uint8_t y);
void LCD_TogglePixel(uint8_t x, uint8_t y);
// x E [0; 127], y E [0; 7]
void LCD_DrawChar(char c, uint8_t x, uint8_t y);
// x E [0; 127], y E [0; 7]
void LCD_DrawString(char *s, uint8_t x, uint8_t y);
// x E [0; 127], y E [0; 7]
void LCD_DrawCharInverse(char c, uint8_t x, uint8_t y);
// x E [0; 127], y E [0; 7]
void LCD_DrawStringInverse(char *s, uint8_t x, uint8_t y);

#endif // __RG12864B_H
