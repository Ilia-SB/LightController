#ifndef SPI_H
#define SPI_H

#include <iocc2530.h>
#include "bsp.h"
#include "rfm12.h"
#include "timer.h"

void spi_init(void);
void spi_write(uint8_t);
uint8_t spi_read(void);
void spi_slave_select(void);
void spi_slave_deselect(void);
__interrupt void P1_ISR(void);

#endif
