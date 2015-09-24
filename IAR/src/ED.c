#include <string.h>
#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"
#include "nwk_pll.h"
#include "uart_intfc.h"
#include "dimmer.h"
#include "utils.h"
#include "radio.h"
#include "rfm12.h"
#include "spi.h"

#ifndef APP_AUTO_ACK
#error ERROR: Must define the macro APP_AUTO_ACK for this application.
#endif

void toggleLED(uint8_t);

static void linkTo(void);
static void serial_read(void);

static uint8_t  sTid = 0;
static linkID_t sLinkID1 = 0;

uint8_t mirror_output_to_radio = 0;
uint8_t volatile radio_rx_complete = 0;

#define SPIN_ABOUT_A_SECOND   NWK_DELAY(1000)
#define SPIN_ABOUT_A_QUARTER_SECOND   NWK_DELAY(250)

/* How many times to try a Tx and miss an acknowledge before doing a scan */
#define MISSES_IN_A_ROW  2

#define COMMAND_BUFFER_SIZE 64
#define READ_BUFFER_SIZE 128

char serial_buffer[READ_BUFFER_SIZE];
char command_buffer[COMMAND_BUFFER_SIZE];
uint8_t command_buffer_pos = 0;

volatile uint8_t  rfm12_packet_read = 0, rfm12_command_pending = 0;
volatile uint8_t  remote_buffer[5];
volatile uint8_t  remote_bit_counter=0,
                  remote_packet_counter = 0;
volatile uint32_t previous_bit_time = 0, current_bit_time =0,
                  previous_packet_time = 0, current_packet_time = 0;

void main (void) {
  BSP_Init();
  //radio_init();
  uart_intfc_init();
  SMPL_Init(0);
  dimmer_init();
  rfm12_init();
  outputln("Starting...");
  
  while(1) {
    /*Check serial*/
    if (rx_peek()) {
      serial_read();
    }
    
    /*Check radio*/
    if (radio_rx_complete) {
      radio_rx_complete = 0;
      radio_receive();
    }
    
    if (rfm12_command_pending && (millis_elapsed(previous_packet_time) > RFM12_PACKET_THRESHOLD)) {
      if (remote_packet_counter == 5) {
        outputln("On/Off");
      } else if (remote_packet_counter > 5) {
        outputln("Stop dimming");
      }
      
      /*Cleanup*/
      rfm12_command_pending = 0; //Clear flag
      remote_packet_counter = 0;
    }
    
    if (rfm12_packet_read) {
      rfm12_packet_read = 0; //Clear flag
      previous_packet_time = millis();

      rfm12_command_pending = 1; //Set flag
      remote_packet_counter++;
      if (remote_packet_counter == 6) {
        outputln("Start dimming");
      }
    }
  }

  /* If an on-the-fly device address is generated it must be done before the
   * call to SMPL_Init(). If the address is set here the ROM value will not
   * be used. If SMPL_Init() runs before this IOCTL is used the IOCTL call
   * will not take effect. One shot only. The IOCTL call below is conformal.
   */
#ifdef I_WANT_TO_CHANGE_DEFAULT_ROM_DEVICE_ADDRESS_PSEUDO_CODE
  {
    addr_t lAddr;

    createRandomAddress(&lAddr);
    SMPL_Ioctl(IOCTL_OBJ_ADDR, IOCTL_ACT_SET, &lAddr);
  }
#endif /* I_WANT_TO_CHANGE_DEFAULT_ROM_DEVICE_ADDRESS_PSEUDO_CODE */

  /* Keep trying to join (a side effect of successful initialization) until
   * successful. Toggle LEDS to indicate that joining has not occurred.
   */
  /*
  while (SMPL_SUCCESS != SMPL_Init(0))
  {
    toggleLED(1);
    toggleLED(2);
    SPIN_ABOUT_A_SECOND;
  }
*/
  
/*
  if (!BSP_LED2_IS_ON())
  {
    toggleLED(2);
  }
  if (!BSP_LED1_IS_ON())
  {
    toggleLED(1);
  }
*/
  /* Unconditional link to AP which is listening due to successful join. */
  //linkTo();
}

static void linkTo()
{
  uint8_t     msg[2];
  uint8_t     button, misses, done;

  /* Keep trying to link... */
  while (SMPL_SUCCESS != SMPL_Link(&sLinkID1))
  {
    toggleLED(1);
    toggleLED(2);
    SPIN_ABOUT_A_SECOND; /* calls nwk_pllBackgrounder for us */
  }

  /* Turn off LEDs. */
  if (BSP_LED2_IS_ON())
  {
    toggleLED(2);
  }
  if (BSP_LED1_IS_ON())
  {
    toggleLED(1);
  }

#ifndef FREQUENCY_HOPPING
  /* sleep until button press... */
  SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
#endif

  while (1)
  {
    /* keep the FHSS scheduler happy */
    FHSS_ACTIVE( nwk_pllBackgrounder( false ) );
    
    button = 0;
    /* Send a message when either button pressed */
    if (BSP_BUTTON1())
    {  /* calls nwk_pllBackgrounder for us */
      SPIN_ABOUT_A_QUARTER_SECOND;  /* debounce... */
      /* Message to toggle LED 1. */
      button = 1;
    }
    else if (BSP_BUTTON2())
    {  /* calls nwk_pllBackgrounder for us */
      SPIN_ABOUT_A_QUARTER_SECOND;  /* debounce... */
      /* Message to toggle LED 2. */
      button = 2;
    }
    if (button)
    {
      uint8_t      noAck;
      smplStatus_t rc;

#ifndef FREQUENCY_HOPPING
      /* get radio ready...awakens in idle state */
      SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
#endif

      /* Set TID and designate which LED to toggle */
      msg[1] = ++sTid;
      msg[0] = (button == 1) ? 1 : 2;
      done = 0;
      while (!done)
      {
        noAck = 0;

        /* Try sending message MISSES_IN_A_ROW times looking for ack */
        for (misses=0; misses < MISSES_IN_A_ROW; ++misses)
        {
          if (SMPL_SUCCESS == (rc=SMPL_SendOpt(sLinkID1, msg, sizeof(msg), SMPL_TXOPTION_ACKREQ)))
          {
            /* Message acked. We're done. Toggle LED 1 to indicate ack received. */
            toggleLED(1);
            break;
          }
          if (SMPL_NO_ACK == rc)
          {
            /* Count ack failures. Could also fail becuase of CCA and
             * we don't want to scan in this case.
             */
            noAck++;
          }
        }
        if (MISSES_IN_A_ROW == noAck)
        {
          /* Message not acked. Toggle LED 2. */
          toggleLED(2);
#ifdef FREQUENCY_AGILITY
          /* Assume we're on the wrong channel so look for channel by
           * using the Ping to initiate a scan when it gets no reply. With
           * a successful ping try sending the message again. Otherwise,
           * for any error we get we will wait until the next button
           * press to try again.
           */
          if (SMPL_SUCCESS != SMPL_Ping(sLinkID1))
          {
            done = 1;
          }
#else
          done = 1;
#endif  /* FREQUENCY_AGILITY */
        }
        else
        {
          /* Got the ack or we don't care. We're done. */
          done = 1;
        }
      }

#ifndef FREQUENCY_HOPPING
      /* radio back to sleep */
      SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
#endif
    }
  }
}


void toggleLED(uint8_t which)
{
  if (1 == which)
  {
    BSP_TOGGLE_LED1();
  }
  else if (2 == which)
  {
    BSP_TOGGLE_LED2();
  }
  return;
}

static void serial_read() {
  char c;
  uint8_t len;
  
  len = rx_receive(serial_buffer, READ_BUFFER_SIZE);
  
  for (int i=0;i<len;i++) {
    c = serial_buffer[i];
    if (c == ';') { /*command separator detected*/
      command_buffer[command_buffer_pos] = '\0'; /*terminate string*/
      process_received_command(command_buffer); /*and process it*/
      command_buffer_pos = 0; /*reset command buffer*/
    } else if (command_buffer_pos < COMMAND_BUFFER_SIZE) { /*prevent buffer overflow*/
      command_buffer[command_buffer_pos++] = c; /*add char to command buffer and shift position*/
    } else {
      strncpy(command_buffer, "OVERFLOW", COMMAND_BUFFER_SIZE); /*report error*/
      process_received_command(command_buffer);
      command_buffer_pos = 0; /*and reset command buffer*/
    }
  }
}