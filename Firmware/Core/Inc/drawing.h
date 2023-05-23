#ifndef __DRAWING_H
#define __DRAWING_H

#include <stdint.h>
#include "RG12864B.h"

#define RX_BUFF_SIZE 4

extern uint8_t newDrawing, loadDrawing, penup;
extern uint8_t Rx_buff[RX_BUFF_SIZE], Rx_byte;

void drawingLoop();
void sendDrawing();
void receiveDrawing();

#endif // __DRAWING_H

