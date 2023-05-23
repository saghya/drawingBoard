#include "menu.h"
#include "state.h"
#include "RG12864B.h"
#include "adc.h"

volatile MenuItem selectedMenu = M_NEW_DRAWING;
static char      *menus[]      = {"- Start new drawing", "- Load drawing",
                                  "- Save drawing", "- Change brightness"};

void menuLoop()
{
    LCD_Clear();

    LCD_DrawString("Main menu", 64 - 6 * 4, 0);

    LCD_DrawStringInverse("UP", 32 - 6, 7);
    LCD_DrawStringInverse("OK", 64 - 6, 7);
    LCD_DrawStringInverse("DOWN", 96 - 2 * 6, 7);

    while (state == S_MENU) {
        startADCs();
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

