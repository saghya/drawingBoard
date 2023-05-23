#include "brightness.h"
#include "main.h"
#include "flash.h"
#include "menu.h"
#include "state.h"
#include "RG12864B.h"
#include <stdio.h>

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

