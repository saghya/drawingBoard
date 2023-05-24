#include "flash.h"
#include "state.h"
#include "RG12864B.h"
#include <string.h>

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
    LCD_DrawString("Saving...", LCD_WIDTH / 2 - 5 * 5, 3);
    updateFlash(LCD_bitmap);
}

void saveBrightness()
{
    uint8_t LCD_bitmapFlash[8][LCD_WIDTH];
    memcpy(LCD_bitmapFlash, (void *)LCD_BITMAP_FLASH_ADDR, 8 * LCD_WIDTH);
    updateFlash(LCD_bitmapFlash);
}

