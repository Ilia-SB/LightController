#include "timer.h"

static uint32_t sys_tick_count = 0;

void timer1_init(uint16_t _counter_top, uint8_t _frequency_divider) {
  uint16_t counter_top = _counter_top >> _frequency_divider; //Adjust counter top value according to selected frequency
  
  P1SEL |= BV(0); //Map pin 0 in Port1 to preipheral
  P1SEL |= BV(1); //Map pin 1 in Port1 to preipheral
  P0SEL |= BV(6); //Map pin 6 in Port0 to preipheral
  P0SEL |= BV(7); //Map pin 7 in Port0 to preipheral
  
  PERCFG |= BV(6); //set T1CFG to location alt. 2
  P2SEL &= ~(BV(4)); //set PRI1P1 to timer1 priority
  P2SEL |= BV(3); //set PRI0P1 to timer1 priority
  
  T1CTL |= BV(1); //Enable modulo mode
  
  T1CCTL1 |= BV(2); //Enable compare mode
  T1CCTL1 |= (BV(5) | BV(4)); //Set compare mode 110: high when equal T1CC0, low when equal T1CC1
  T1CCTL2 |= BV(2); //Enable compare mode
  T1CCTL2 |= (BV(5) | BV(4)); //Set compare mode 110: high when equal T1CC0, low when equal T1CC2
  T1CCTL3 |= BV(2); //Enable compare mode
  T1CCTL3 |= (BV(5) | BV(4)); //Set compare mode 110: high when equal T1CC0, low when equal T1CC3  
  T1CCTL4 |= BV(2); //Enable compare mode
  T1CCTL4 |= (BV(5) | BV(4)); //Set compare mode 110: high when equal T1CC0, low when equal T1CC4
  
  T1CC0L = LOWBYTE(counter_top); //Counter top
  T1CC0H = HIGHBYTE(counter_top);
  
  T1CC1L = 0; //Counter compare for channel 1
  T1CC1H = 0;
  T1CC2L = 0; //Counter compare for channel 2
  T1CC2H = 0;
  T1CC3L = 0; //Counter compare for channel 3
  T1CC3H = 0;
  T1CC4L = 0; //Counter compare for channel 4
  T1CC4H = 0;
}

void timer4_init() {
  T4CTL |= (BV(7)|BV(6)|BV(5)); //Prescaler 128
  T4CTL |= BV(2); //Clear
  TIMIF = 0; //Clear interrupt flags
  T4CCTL0 &= ~(BV(6)); //Clear channel interrupt
  T4CCTL1 &= ~(BV(6));
  T4CTL |= BV(3); //Overflow interrupt enable
  T4IE = 1;
  EA = 1;
  T4CTL |= BV(4); //Start timer
}

void configure_clk() {
  CLKCONCMD &= ~(BV(6)); //select 32mHz OSC
  CLKCONCMD &= ~(BV(0) | BV(1) | BV(2)); //set CLKSPD to 32mHz
  CLKCONCMD &= ~(BV(3) | BV(4) | BV(5)); //set TICKSPD to 32mHz
}

uint32_t millis() {
  return sys_tick_count;
}

uint32_t millis_elapsed(uint32_t since) {
  uint32_t ret_val;
  uint32_t now = millis();
  int32_t elapsed = (int32_t)(now - since);
  if (elapsed >= 0) //no overflow happened
    return (uint32_t)elapsed;
  else { //overflow happened
    ret_val = (uint32_t)(LONG_MAX - since + now);
    return ret_val;
  }
}

void enable_watchdog() {
  WDCTL = 0x0;
  WDCTL |= 0x8; //enable watchdog mode
}

void clear_watchdog() {
  WDCTL = 0xA0;
  WDCTL = 0x50;
}

#pragma vector = T4_VECTOR
__interrupt void timer4_isr() {
  sys_tick_count++;
  TIMIF = 0;
}
