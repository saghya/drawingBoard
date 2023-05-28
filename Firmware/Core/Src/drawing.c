#include "drawing.h"
#include "flash.h"
#include "state.h"
#include "adc.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart2;

uint32_t x, y;
uint8_t  newDrawing = 0, loadDrawing = 0, penup = 0;

uint8_t Rx_buff[RX_BUFF_SIZE] = {0}, Rx_byte;

void drawingLoop()
{
    if (newDrawing) {
        newDrawing = 0;
        LCD_ClearBitmap();
    } else if (loadDrawing) {
        loadDrawing = 0;
        penup       = 1;
        memcpy(LCD_bitmap, (void *)LCD_BITMAP_FLASH_ADDR, 8 * LCD_WIDTH);
    }

    LCD_DrawBitmap();
    uint8_t prev_x, prev_y;
    while (state == S_DRAWING) {
        if (newDrawing) {
            newDrawing = 0;
            LCD_Clear();
            LCD_ClearBitmap();
        }
        startADCs();

        prev_x = x;
        prev_y = y;
        x = y = 0;
        for (int i = 0; i < ADC_PRECISION; i++) {
            x += adc1_values[i];
            y += adc2_values[i];
        }
        x /= ADC_PRECISION / 2;
        y /= ADC_PRECISION / 2;

        x = ~(x >> 6) & 0x7f;
        y = y >> 7;

        if (penup && !(LCD_bitmap[prev_y / 8][prev_x] & (1 << prev_y % 8))) {
            LCD_SetPixel(0, prev_x, prev_y);
        } else if (!penup) {
            LCD_SavePixel(1, x, y);
        }
        LCD_SetPixel(1, x, y);
    }
}

void sendDrawing()
{
    HAL_UART_Transmit(&huart2, (uint8_t *)"OK", 2, -1);
    HAL_UART_Transmit(&huart2, (uint8_t *)LCD_bitmap, 8 * LCD_WIDTH, -1);
}

void receiveDrawing()
{
    HAL_UART_Transmit(&huart2, (uint8_t *)"OK", 2, -1);
    HAL_UART_Receive(&huart2, (uint8_t *)LCD_bitmap, 8 * LCD_WIDTH, -1);
    LCD_DrawBitmap();
    penup = 1;
    state = S_DRAWING;
}

