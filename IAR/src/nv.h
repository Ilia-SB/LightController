#ifndef NV_H
#define NV_H

#include "bsp.h"

void flash_page_read(uint8_t bank, 
    uint8_t page,
    uint16_t offset,
    void *buf, 
    uint16_t len);

void flash_page_write(uint8_t bank, 
    uint8_t page, 
    uint16_t offset,
    void *buf, 
    uint16_t len);

void flash_page_erase(uint8_t bank, 
    uint8_t page);
#endif