#include "spi.h"

void spi_init() {
//GPIO config
  PERCFG |= 0x02;       //PERCFG.U1CFG = 1 - enable USART1. set alt location for it's IO
  
  P1SEL |= 0xE0;        //Set bits 5-7 of P1SEL (11100000) - select alt2 location for SPI
                        //5-CLK, 6-MOSI, 7-MISO
  P1SEL &= ~((1<<3)|(1<<4)); //Clear bits 3-4 of P1SEL - map P1_3 and P1_4 for GPIO
  
  P1DIR |= (1<<4); //Set bit 4 of P1DIR - P1_4 pin set as output. SS.
  P1DIR &= ~(1<<3); //Clear bit 3 of P1DIR - P1_3 pin set as input. IRQ.
  
  P1INP &= ~(1<<7); //Clear bit 7 of P1INP - P1_7 pin set to pullup/pulldown mode
  //P1INP &= ~(1<<5); //Clear bit 5 of P1INP - P1_5 pin set to pullup/pulldown mode
  //P1INP &= ~(1<<3); //Clear bit 3 of P1INP - P1_3 pin set to pullup/pulldown mode
  P2INP &= ~(1<<6); //Clear P2INP.PDUP1 - set pullup mode for port 1 pins
  
//SPI config
  //Set baud rate to max
  U1BAUD = 0x00; //BAUD_M=0
  U1GCR |= 0x11; //BAUD_E=1

  U1CSR &= ~((1<<7)|(1<<5)); //bit 7: SPI mode, bit 5: SPI Master mode
  U1CSR |= (1<<6); //bit 6: recieve enable
  
  U1GCR &= ~(1<<7); //Polarity: positive
  U1GCR |= (1<<5)|(1<<6); //Phase: 1; Order: MSB first
  
  P1_4 = 1; //SS high
  
//Interrupt config
  P1IEN |= (1<<3); //Enable interrupt for pin 3
  PICTL |= (1<<1); //Falling edge
  IEN2 |= (1<<4); //Port1 interrupts enable
  P1IFG = 0; //Clear interrupt flag
  P1IF = 0;
  EA=1;
}

#pragma vector = P1INT_VECTOR
__interrupt void P1_ISR(void) {
  if (P1IFG & (1<<3)) {
    //Read 1 packet
    rfm12_read_packet();
  }
  P1IFG = 0;
  P1IF = 0;
}

void spi_write(uint8_t tx_byte) {
  U1DBUF = tx_byte; //Put byte to transmit in buffer. Transmission starts automatically
  while((U1CSR & (1<<1)) == 0); //Wait till transmission completes (CSR byte 1 turns to 1)
  U1CSR &= ~((1<<1)|(1<<2)); //Clear receive and transmit done bits
}

uint8_t spi_read(void) {
  uint8_t rx_byte;
  U1DBUF = 0x00; //Transmit 0
  while((U1CSR & (1<<1)) == 0); //Wait till transmission completes (CSR byte 1 turns to 1)
  rx_byte = U1DBUF;
  U1CSR &= ~((1<<1)|(1<<2)); //Clear receive and transmit done bits
  return rx_byte;
}

void spi_slave_select() {
  P1_4 = 0;
}

void spi_slave_deselect() {
  P1_4 = 1;
}
  
