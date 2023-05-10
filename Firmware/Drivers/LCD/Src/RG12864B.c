#include "RG12864B.h"
#include "font5x8.h"
#include <string.h>

GPIO_TypeDef *LCD_DB_GPIO_Port[8] = {
    LCD_DB0_GPIO_Port, LCD_DB1_GPIO_Port, LCD_DB2_GPIO_Port, LCD_DB3_GPIO_Port,
    LCD_DB4_GPIO_Port, LCD_DB5_GPIO_Port, LCD_DB6_GPIO_Port, LCD_DB7_GPIO_Port};

uint16_t LCD_DB_Pin[8] = {LCD_DB0_Pin, LCD_DB1_Pin, LCD_DB2_Pin, LCD_DB3_Pin,
                          LCD_DB4_Pin, LCD_DB5_Pin, LCD_DB6_Pin, LCD_DB7_Pin};

uint8_t LCD_bitmap[8][128] = {0};

void LCD_SendByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(LCD_DB_GPIO_Port[i], LCD_DB_Pin[i], data & 1 << i);
    }

    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
    delay_us(5);
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
}

void LCD_Instruction(uint8_t instruction)
{
    HAL_GPIO_WritePin(LCD_D_I_GPIO_Port, LCD_D_I_Pin, GPIO_PIN_RESET);
    LCD_SendByte(instruction);
}

void LCD_Data(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_D_I_GPIO_Port, LCD_D_I_Pin, GPIO_PIN_SET);
    LCD_SendByte(data);
}

void LCD_Clear()
{
    HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_RESET);
    for (int i = 0; i < 8; i++) {
        LCD_Instruction(PAGE_BASE | i);
        LCD_Instruction(X_BASE);
        for (int j = 0; j < LCD_WIDTH / 2; j++) {
            LCD_Data(0);
        }
    }
}

void LCD_ClearBitmap()
{
    memset(LCD_bitmap, 0, LCD_WIDTH * LCD_HEIGHT / 8);
}

void LCD_DrawBitmap()
{
	HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_SET);
	for (int i = 0; i < 8; i++) {
		LCD_Instruction(PAGE_BASE | i);
		LCD_Instruction(X_BASE);
		for (int j = 0; j < 64; j++) {
			LCD_Data(LCD_bitmap[i][j]);
		}
	}

	HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_RESET);
	for (int i = 0; i < 8; i++) {
		LCD_Instruction(PAGE_BASE | i);
		LCD_Instruction(X_BASE);
		for (int j = 64; j < 128; j++) {
			LCD_Data(LCD_bitmap[i][j]);
		}
	}
}

void LCD_Init()
{
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
    HAL_GPIO_WritePin(xLCD_EN_GPIO_Port, xLCD_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_R_W_GPIO_Port, LCD_R_W_Pin, LCD_WRITE);

    LCD_BRIGHTNESS = LCD_BRIGHTNESS_FLASH;

    LCD_Instruction(LCD_ON);
    LCD_Clear();
}

void LCD_SetPos(uint8_t x, uint8_t p)
{
    if (x < LCD_WIDTH / 2) {
        HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_RESET);
        x -= LCD_WIDTH / 2;
    }

    LCD_Instruction(PAGE_BASE | p);
    LCD_Instruction(X_BASE | x);
}

void LCD_SetPixel(uint8_t x, uint8_t y)
{
    LCD_bitmap[y / 8][x] |= 1 << y % 8;
    LCD_SetPos(x, y / 8);
    LCD_Data(LCD_bitmap[y / 8][x]);
}

void LCD_ResetPixel(uint8_t x, uint8_t y)
{
    LCD_bitmap[y / 8][x] &= ~(1 << y % 8);
    LCD_SetPos(x, y / 8);
    LCD_Data(LCD_bitmap[y / 8][x]);
}

void LCD_TogglePixel(uint8_t x, uint8_t y)
{
    if (LCD_bitmap[y / 8][x] & 1 << y % 8) {
        LCD_ResetPixel(x, y);
    } else {
        LCD_SetPixel(x, y);
    }
}

void LCD_DrawChar(char c, uint8_t x, uint8_t p)
{
    if (x > LCD_WIDTH - 1 - FONT_WIDTH) {
        x -= FONT_WIDTH;
    }
    if (x > 0) {
        LCD_SetPos(x - 1, p);
        LCD_Data(0x00);
    }
    for (int i = 0; i < FONT_WIDTH; i++, x++) {
        LCD_SetPos(x, p);
        LCD_Data(font5x8[c - 0x20][i]);
    }
    LCD_Data(0x00);
}

void LCD_DrawString(char *s, uint8_t x, uint8_t p)
{
    for (; *s != '\0'; s++) {
        if (*s == '\n') {
            x = 0;
            p += 1;
            continue;
        }
        if (x + FONT_WIDTH > LCD_WIDTH - 1) {
            x = 0;
            p += 1;
        }
        LCD_DrawChar(*s, x, p);
        x += FONT_WIDTH + 1;
    }
}

void LCD_DrawCharInverse(char c, uint8_t x, uint8_t p)
{
    if (x > LCD_WIDTH - 1 - FONT_WIDTH) {
        x -= FONT_WIDTH;
    }
    if (x > 0) {
        LCD_SetPos(x - 1, p);
        LCD_Data(0xff);
    }
    for (int i = 0; i < FONT_WIDTH; i++, x++) {
        LCD_SetPos(x, p);
        LCD_Data(~(font5x8[c - 0x20][i]));
    }
    LCD_Data(0xff);
}

void LCD_DrawStringInverse(char *s, uint8_t x, uint8_t p)
{
    for (; *s != '\0'; s++) {
        if (*s == '\n') {
            x = 0;
            p += 1;
            continue;
        }
        if (x + FONT_WIDTH > LCD_WIDTH - 1) {
            x = 0;
            p += 1;
        }
        LCD_DrawCharInverse(*s, x, p);
        x += 6;
    }
}

