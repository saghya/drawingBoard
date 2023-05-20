#ifndef __CALLBACKS_H
#define __CALLBACKS_H

#include "drawing.h"

extern volatile MenuItem selectedMenu;
extern volatile State    state;
extern uint8_t           newDrawing, loadDrawing;
extern uint8_t           Rx_buff[RX_BUFF_SIZE];
extern uint8_t           Rx_byte;
extern uint8_t           penup;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif // __CALLBACKS_H
