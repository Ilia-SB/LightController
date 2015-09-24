#ifndef RFM12_H
#define RFM12_H

#include "spi.h"
#include "bsp.h"
#include <stdio.h>

/*
If time in ms between individual bits sent is greater than
RFM12_BIT_THRESHOLD then packet is incomplete and will be discarded
*/
#define RFM12_BIT_THRESHOLD     2

/*
If time in ms between individual packets sent is greater than
RFM12_PACKET_THRESHOLD then command transmission is over
*/
#define RFM12_PACKET_THRESHOLD 15

void rfm12_read_packet(void);
uint32_t rfm12_get_data(void);
void rfm12_init(void);
#endif