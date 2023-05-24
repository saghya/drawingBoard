#include "drawing.h"
#include "main.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include "state.h"
#include "menu.h"

#define DEBOUNCE_MILLIS 10

extern UART_HandleTypeDef huart2;

uint32_t previousMillis = 0;
uint32_t currentMillis  = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    currentMillis = HAL_GetTick();
    if (currentMillis - previousMillis < DEBOUNCE_MILLIS) {
        return;
    }
    previousMillis = currentMillis;

    switch (state) {
        case S_MENU:
            if (GPIO_Pin == BTN3_Pin) {
                selectedMenu == M_CHANGE_BRIGHTNESS ? selectedMenu = 0
                                                    : selectedMenu++;
            } else if (GPIO_Pin == BTN1_Pin) {
                selectedMenu == 0 ? selectedMenu = M_CHANGE_BRIGHTNESS
                                  : selectedMenu--;
            } else if (GPIO_Pin == BTN2_Pin) {
                switch (selectedMenu) {
                    case M_NEW_DRAWING:
                        state      = S_DRAWING;
                        newDrawing = 1;
                        break;
                    case M_LOAD_DRAWING:
                        state       = S_DRAWING;
                        loadDrawing = 1;
                        break;
                    case M_SAVE_DRAWING:
                        state = S_SAVE;
                        break;
                    case M_CHANGE_BRIGHTNESS:
                        state = S_CHANGE_BRIGHTNESS;
                        break;
                }
            }
            break;
        case S_DRAWING:
            if (GPIO_Pin == BTN1_Pin) {
                state = S_MENU;
            } else if (GPIO_Pin == BTN2_Pin) {
                penup = penup ? 0 : 1;
            } else if (GPIO_Pin == BTN3_Pin) {
                newDrawing = 1;
            }
            break;
        case S_CHANGE_BRIGHTNESS:
            if (GPIO_Pin == BTN2_Pin) {
                state = S_MENU;
            }
            break;
        case S_SAVE:
            break;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != &huart2) {
        HAL_UART_Receive_IT(huart, &Rx_byte, 1);
        return;
    }

    static uint16_t i       = 0;
    static uint8_t  command = 0;

    if (Rx_byte == '@') {
        i       = 0;
        command = 1;
    } else if (command) {
        if (i < RX_BUFF_SIZE) {
            Rx_buff[i++] = Rx_byte;
        } else {
            i = command = 0;
        }
    }

    if (i == RX_BUFF_SIZE) {
        i = command = 0;
        if (!strncmp((char *)Rx_buff, "SAVE", RX_BUFF_SIZE)) {
            sendDrawing();
        } else if (!strncmp((char *)Rx_buff, "LOAD", RX_BUFF_SIZE)) {
            receiveDrawing();
        }
    }

    HAL_UART_Receive_IT(huart, &Rx_byte, 1);
}

