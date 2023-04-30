#include "RG12864B.h"
#include "font5x8.h"
#include "main.h"

#define WIDTH  128
#define HEIGHT 64

#define LCD_ON    0b00111111
#define LCD_OFF   0b00111110
#define PAGE_BASE 0b10111000
#define X_BASE    0b01000000
#define Z_BASE    0b11000000

#define LCD_READ  GPIO_PIN_SET
#define LCD_WRITE GPIO_PIN_RESET

#define DELAY     1

uint8_t LCD_x = 0;
uint8_t LCD_y = 0;

GPIO_TypeDef* LCD_DB_GPIO_Port[8] = { LCD_DB0_GPIO_Port,
                                      LCD_DB1_GPIO_Port,
                                      LCD_DB2_GPIO_Port,
                                      LCD_DB3_GPIO_Port,
                                      LCD_DB4_GPIO_Port,
                                      LCD_DB5_GPIO_Port,
                                      LCD_DB6_GPIO_Port,
                                      LCD_DB7_GPIO_Port };

uint16_t LCD_DB_Pin[8] = { LCD_DB0_Pin,
                           LCD_DB1_Pin,
                           LCD_DB2_Pin,
                           LCD_DB3_Pin,
                           LCD_DB4_Pin,
                           LCD_DB5_Pin,
                           LCD_DB6_Pin,
                           LCD_DB7_Pin };

uint8_t bitmap[8][128] = {0};

void LCD_Instruction(uint8_t instruction)
{
    HAL_GPIO_WritePin(LCD_D_I_GPIO_Port, LCD_D_I_Pin, GPIO_PIN_RESET);

    for (uint8_t i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(LCD_DB_GPIO_Port[i], LCD_DB_Pin[i], instruction & 1 << i);
    }

    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
    HAL_Delay(DELAY);
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
}

void LCD_Data(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_D_I_GPIO_Port, LCD_D_I_Pin, GPIO_PIN_SET);

    for (uint8_t i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(LCD_DB_GPIO_Port[i], LCD_DB_Pin[i], data & 1 << i);
    }

    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
    HAL_Delay(DELAY);
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
}

void LCD_Clear()
{

    for (int i = 0; i < 8; i++) {
        LCD_Instruction(PAGE_BASE | i);
        for (int j = 0; j < 64; j++) {
            LCD_Instruction(X_BASE | j);
            LCD_Data(0);
        }
    }
}

void LCD_Init()
{
    HAL_GPIO_WritePin(xLCD_EN_GPIO_Port, xLCD_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_R_W_GPIO_Port, LCD_R_W_Pin, LCD_WRITE);

    LCD_Instruction(LCD_ON);
    LCD_Clear();
}

void LCD_SetPos(uint8_t x, uint8_t y)
{
    if (x < WIDTH / 2) {
        HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_RESET);
        x -= WIDTH/2;
    }

    LCD_Instruction(PAGE_BASE | y / 8);
    LCD_Instruction(X_BASE | x);
}

void LCD_SetPixel(uint8_t x, uint8_t y)
{
    bitmap[y/8][x] |= 1 << y % 8;
    LCD_SetPos(x, y);
    LCD_Data(bitmap[y/8][x]);
}

void LCD_ResetPixel(uint8_t x, uint8_t y)
{
    bitmap[y/8][x] &= ~(1 << y % 8);
    LCD_SetPos(x, y);
    LCD_Data(bitmap[y/8][x]);
}

void LCD_TogglePixel(uint8_t x, uint8_t y)
{
    if (bitmap[y/8][x] & 1 << y % 8) {
        LCD_ResetPixel(x, y);
    } else {
        LCD_SetPixel(x, y);
    }
}

void LCD_WriteChar(char c, uint8_t x, uint8_t y)
{
    for (int i = 0; i < 5; i++, x++) {
        LCD_SetPos(x, y);
        LCD_Data(font5x8[c - 0x20][i]);
    }
}

void LCD_WriteString(char *s, uint8_t x, uint8_t y)
{
    for (; *s != '\0'; s++) {
        if (*s == '\n') {
            x = 0;
            y+=8;
            continue;
        }
        if (x + 5 > 127) {
            x = 0;
            y += 8;
        }
        LCD_WriteChar(*s, x, y);
        x += 6;
    }
}

