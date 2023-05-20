#include "drawing.h"
#include <stdio.h>
#include <string.h>

uint32_t          x_values[ADC_PRECISION] = {0};
uint32_t          y_values[ADC_PRECISION] = {0};
uint32_t          x, y;
volatile MenuItem selectedMenu = M_NEW_DRAWING;
volatile State    state        = S_MENU;
uint8_t           newDrawing = 0, loadDrawing = 0;
uint8_t           Rx_buff[RX_BUFF_SIZE] = {0};
uint8_t           Rx_byte;
uint8_t           penup = 0;

void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim1, 0); // set the counter value a 0
    while (__HAL_TIM_GET_COUNTER(&htim1) < us)
        ; // wait for the counter to reach the us input in the parameter
}

void updateFlash(uint8_t bitmap[8][LCD_WIDTH])
{
    HAL_FLASH_Unlock();
    FLASH_Erase_Sector(3, FLASH_VOLTAGE_RANGE_3);
    // brightness
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, LCD_BRIGHTNESS_FLASH_ADDR,
                      (uint64_t)LCD_BRIGHTNESS);
    // LCD_bitmap
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < LCD_WIDTH; j++) {
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
                              LCD_BITMAP_FLASH_ADDR + i * LCD_WIDTH + j,
                              (uint64_t)bitmap[i][j]);
        }
    }
    HAL_FLASH_Lock();
}

void saveDrawing()
{
    state = S_MENU;
    LCD_Clear();
    LCD_DrawString("Saving...", 64 - 5 * 5, 3);
    updateFlash(LCD_bitmap);
}

void saveBrightness()
{
    uint8_t LCD_bitmapFlash[8][LCD_WIDTH];
    memcpy(LCD_bitmapFlash, (void *)LCD_BITMAP_FLASH_ADDR, 8 * LCD_WIDTH);
    updateFlash(LCD_bitmapFlash);
}

void menuLoop()
{
    char *menus[] = {"- Start new drawing", "- Load drawing", "- Save drawing",
                     "- Change brightness"};
    LCD_Clear();

    LCD_DrawString("Main menu", 64 - 6 * 4, 0);

    LCD_DrawStringInverse("UP", 32 - 6, 7);
    LCD_DrawStringInverse("OK", 64 - 6, 7);
    LCD_DrawStringInverse("DOWN", 96 - 2 * 6, 7);

    while (state == S_MENU) {
        HAL_ADC_Start_DMA(&hadc1, x_values, ADC_PRECISION);
        HAL_ADC_Start_DMA(&hadc2, y_values, ADC_PRECISION);
        for (int i = 0; i < 4; i++) {
            switch (selectedMenu) {
                case M_NEW_DRAWING:
                    if (i == 0)
                        i++;
                    LCD_DrawStringInverse(menus[0], 1, 2);
                    break;
                case M_LOAD_DRAWING:
                    if (i == 1)
                        i++;
                    LCD_DrawStringInverse(menus[1], 1, 3);
                    break;
                case M_SAVE_DRAWING:
                    if (i == 2)
                        i++;
                    LCD_DrawStringInverse(menus[2], 1, 4);
                    break;
                case M_CHANGE_BRIGHTNESS:
                    if (i == 3)
                        continue;
                    LCD_DrawStringInverse(menus[3], 1, 5);
                    break;
            }
            LCD_DrawString(menus[i], 1, i + 2);
        }
    }
}

void drawingLoop()
{
    if (newDrawing) {
        newDrawing = 0;
        LCD_ClearBitmap();
    } else if (loadDrawing) {
        loadDrawing = 0;
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
        HAL_ADC_Start_DMA(&hadc1, x_values, ADC_PRECISION);
        HAL_ADC_Start_DMA(&hadc2, y_values, ADC_PRECISION);

        prev_x = x;
        prev_y = y;
        x = y = 0;
        for (int i = 0; i < ADC_PRECISION; i++) {
            x += x_values[i];
            y += y_values[i];
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

void brightnessLoop()
{
    LCD_Clear();

    LCD_DrawString("Brightness", 64 - 6 * 5, 0);

    LCD_DrawStringInverse("+", 32, 7);
    LCD_DrawStringInverse("OK", 64 - 6, 7);
    LCD_DrawStringInverse("-", 96 - 6, 7);

    char buff[5] = {0};
    while (state == S_CHANGE_BRIGHTNESS) {
        snprintf(buff, 5, "%3d%%", (LCD_BRIGHTNESS * 100) / 0xFFFF);
        LCD_DrawString(buff, 64 - 6 * 2, 3);

        if (!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin)) {
            if (99 * 0xFFFF / 100 < LCD_BRIGHTNESS) {
                LCD_BRIGHTNESS = 0xFFFF;
            } else {
                LCD_BRIGHTNESS += 0xFFFF / 100;
            }
        }

        if (!HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin)) {
            if (1 * 0xFFFF / 100 >= LCD_BRIGHTNESS) {
                LCD_BRIGHTNESS = 0xFFFF / 100;
            } else {
                LCD_BRIGHTNESS -= 0xFFFF / 100;
            }
        }
        HAL_Delay(25);
    }

    if (LCD_BRIGHTNESS != LCD_BRIGHTNESS_FLASH) {
        saveBrightness();
    }
}

void mainLoopInit()
{
    HAL_TIM_Base_Start(&htim1);
    HAL_UART_Receive_IT(&huart2, &Rx_byte, 1);
    LCD_Init();
}

void mainLoop()
{
    switch (state) {
        case S_MENU:
            menuLoop();
            break;
        case S_DRAWING:
            drawingLoop();
            break;
        case S_SAVE:
            saveDrawing();
            break;
        case S_CHANGE_BRIGHTNESS:
            brightnessLoop();
            break;
    }
}

