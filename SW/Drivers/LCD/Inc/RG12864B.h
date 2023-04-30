#ifndef __RG12864B_H
#define __RG12864B_H

#include "main.h"

//extern GPIO_TypeDef* LCD_DB_GPIO_Port[8];
//extern uint16_t LCD_DB_Pin[8];

void LCD_Init();
void LCD_Clear();
void LCD_SetPos(uint8_t x, uint8_t y);
void LCD_SetPixel(uint8_t x, uint8_t y);
void LCD_ResetPixel(uint8_t x, uint8_t y);
void LCD_TogglePixel(uint8_t x, uint8_t y);
void LCD_DrawChar(char c, uint8_t x, uint8_t y);
void LCD_DrawString(char *s, uint8_t x, uint8_t y);

#endif // __RG12864B_H
