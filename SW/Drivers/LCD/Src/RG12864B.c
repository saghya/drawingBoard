#include "RG12864B.h"
#include "main.h"
#include "stm32f4xx_hal.h"

#define WIDTH  128
#define HEIGHT 64

#define LCD_ON    0b00111111
#define LCD_OFF   0b00111110
#define PAGE_BASE 0b10111000
#define X_BASE    0b01000000
#define Z_BASE    0b11000000

#define DELAY     1

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

void LCD_Init()
{
    HAL_GPIO_WritePin(xLCD_EN_GPIO_Port, xLCD_EN_Pin, GPIO_PIN_RESET);
    LCD_Reset();
    HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_R_W_GPIO_Port, LCD_R_W_Pin, GPIO_PIN_RESET);

    LCD_Instruction(LCD_ON);
    LCD_Clear();
}

void LCD_SetPixel(uint8_t x, uint8_t y)
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
    LCD_Data(1 << y % 8);
}

void LCD_Test()
{
    //LCD_Instruction(PAGE_BASE | 3);
    //LCD_Instruction(X_BASE | 15);
    ////LCD_Instruction(Z_BASE);

    //uint8_t data[6] = {0b00000000,
    //                   0b01111111,
    //                   0b00001001,
    //                   0b00011001,
    //                   0b00101001,
    //                   0b01000110};

    //for (uint8_t i = 0; i < sizeof(data); i++) {
    //    LCD_Data(data[i]);
    //}

    //LCD_SetPixel(64,32);
}

void LCD_Reset()
{
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(DELAY);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
}

void LCD_Clear()
{

    for (int i = 0; i < 8; i++) {
        LCD_Instruction(PAGE_BASE | i);
        for (int j = 0; j < 64; j++) {
            LCD_Instruction(X_BASE    | j);
            LCD_Data(0);
        }
    }
}

