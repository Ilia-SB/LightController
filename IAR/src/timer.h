#ifndef TIMER_H
#define TIMER_H

#include "bsp.h"
#include "dimmer.h"
#include <ioCC2530.h>
#include <limits.h>

void configure_clk(void);
void timer1_init(uint16_t, uint8_t);
void timer4_init(void);
void enable_watchdog(void);
void clear_watchdog(void);
uint32_t millis(void);
uint32_t millis_elapsed(uint32_t);
__interrupt void timer4_isr(void);

#endif