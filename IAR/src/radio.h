#ifndef RADIO_H
#define RADIO_H

#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "nwk_pll.h"
#include "utils.h"
#include "dimmer.h"

uint8_t radio_callback(linkID_t);
void radio_init(void);
void radio_link(void);
void radio_transmit(const char *send_buffer, uint8_t len);
void radio_receive(void);
void radio_transmit_to_addr(linkID_t addr, const char *send_buffer, uint8_t len);
#endif
