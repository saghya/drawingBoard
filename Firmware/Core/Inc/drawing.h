#ifndef __DRAWING_H
#define __DRAWING_H

#include <stdint.h>
#include "RG12864B.h"

#define ADC_PRECISION         4096
#define LCD_BITMAP_FLASH_ADDR 0x0800C100
#define RX_BUFF_SIZE          4

typedef enum {
    M_NEW_DRAWING,
    M_LOAD_DRAWING,
    M_SAVE_DRAWING,
    M_CHANGE_BRIGHTNESS
} MenuItem;

typedef enum {
    S_MENU,
    S_DRAWING,
    S_SAVE,
    S_CHANGE_BRIGHTNESS,
} State;

extern ADC_HandleTypeDef  hadc1;
extern ADC_HandleTypeDef  hadc2;
extern DMA_HandleTypeDef  hdma_adc1;
extern DMA_HandleTypeDef  hdma_adc2;
extern TIM_HandleTypeDef  htim1;
extern TIM_HandleTypeDef  htim8;
extern UART_HandleTypeDef huart2;
extern uint8_t            LCD_bitmap[8][LCD_WIDTH];

void delay_us(uint16_t us);
void updateFlash(uint8_t bitmap[8][LCD_WIDTH]);
void saveDrawing();
void saveBrightness();
void menuLoop();
void drawingLoop();
void brightnessLoop();
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void mainLoopInit();
void mainLoop();

#endif // __DRAWING_H
