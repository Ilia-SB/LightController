#include "rfm12.h"

static uint8_t init_bytes[] = {0x80, 0xe7, 0x82, 0xd8, 0xa6, 0x40, 0xc6, 0x2a, 0x94, 0x80, 0xc2, 0xac, 0xca, 0x80, 0xce, 0xd4, 0xc4, 0x83, 0x98, 0xd0, 0xcc, 0x67, 0xe0, 0x00, 0xc8, 0x00, 0xc0, 0x40, 0x00, 0x00, 0xca, 0x80, 0xca, 0x83};
extern volatile uint8_t   remote_buffer[5];
extern volatile uint8_t   remote_bit_counter,
                          remote_packet_counter;
extern volatile uint32_t  previous_bit_time, current_bit_time,
                          previous_packet_time, current_packet_time;
extern volatile uint8_t          rfm12_packet_read;

void rfm12_init() {
  spi_init();
  for (int i=0; i<34; i=i+2) {
    spi_slave_select();
    spi_write(init_bytes[i]);
    spi_write(init_bytes[i+1]);
    spi_slave_deselect();
  }
}

void rfm12_read_packet() {
  if (millis_elapsed(previous_bit_time) > RFM12_BIT_THRESHOLD) {
    remote_bit_counter = 0;
  }
  previous_bit_time = millis();
  
  spi_slave_select();
  spi_read();
  spi_read();
  remote_buffer[remote_bit_counter++] = spi_read();
  spi_slave_deselect();
  
  if (remote_bit_counter == 5) {
    spi_slave_select();
    spi_write(0xCA);
    spi_write(0x80);
    spi_slave_deselect();
    spi_slave_select();
    spi_write(0xCA);
    spi_write(0x83);
    spi_slave_deselect();
    remote_bit_counter = 0;
    rfm12_packet_read = 1;
  }
}

/*
void rfm12_read_data() {
  spi_slave_select();
  spi_read(); //Skip first 2 bytes - we don't need them
  spi_read();
  received_data_buffer[received_bytes_counter++] = spi_read();
  spi_slave_deselect();
  if (received_bytes_counter == 4) { //Send some service data to rfm12
    received_bytes_counter = 0;
    spi_slave_select();
    spi_write(0xCA);
    spi_write(0x80);
    spi_slave_deselect();
    spi_slave_select();
    spi_write(0xCA);
    spi_write(0x83);
    spi_slave_deselect();
    if (rfm12_previous_packet_time == 0) //if it's a first ever packet
    {
      rfm12_previous_packet_time = millis();
    } else
    {
      rfm12_previous_packet_time = rfm12_this_packet_time;
    }
    rfm12_this_packet_time = millis();
    rfm12_data_ready = 1; //Set flag for main loop
  }
}
*/

/*
uint32_t rfm12_get_data() {
  uint32_t *data;
  data = (uint32_t*) received_data_buffer;
  return *data;
}
*/