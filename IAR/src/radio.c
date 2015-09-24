#include "radio.h"

#define SPIN_ABOUT_A_SECOND   NWK_DELAY(1000)
#define SPIN_ABOUT_A_QUARTER_SECOND   NWK_DELAY(250)


/* How many times to try a Tx and miss an acknowledge before doing a scan */
#define MISSES_IN_A_ROW  3

static linkID_t sLinkID1 = 0;

static char read_buff[40];
static char command[40];
static uint8_t command_pos = 0;

extern volatile uint8_t radio_rx_complete;

uint8_t radio_callback(linkID_t linkID) {
  radio_rx_complete = 1; //Set the flag
  return 0; //Indicate that the actual recieve will be performed on the main thread
}

void radio_init() {
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
   * successful.
   */
  
  outputln("SMPL_Init..");
  while (SMPL_SUCCESS != SMPL_Init(&radio_callback))
  {
    SPIN_ABOUT_A_SECOND; // calls nwk_pllBackgrounder for us
    outputln("  Joining to network failed. Retrying..");
  }
  outputln("Success.");
  
  SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0); // Enable radio RX to be able to listen to AP's messages
}

void radio_link() {
   // Keep trying to link...
  outputln("Linking to AP..");
  while (SMPL_SUCCESS != SMPL_Link(&sLinkID1))
  {
    SPIN_ABOUT_A_SECOND; // calls nwk_pllBackgrounder for us
    outputln("  Retrying..");
  }
  outputln("Success.");
}

void radio_transmit(const char *send_buffer, uint8_t len) {
  radio_transmit_to_addr(sLinkID1, send_buffer, len);
}

void radio_transmit_to_addr(linkID_t addr, const char *send_buffer, uint8_t len) {
  smplStatus_t status;
  uint8_t retry_num = 0;
  //output("Sending: "); outputln(send_buffer);
  while (len > 0) {
    uint8_t bytes_to_send;
    if (len > MAX_APP_PAYLOAD) {
      bytes_to_send = MAX_APP_PAYLOAD; //Prevent radio tx buffer overflow: split msg into frames
    } else {
      bytes_to_send = len; //Send the entire message
    }
    while (1) {
      status = SMPL_SendOpt(addr, (uint8_t*)send_buffer, bytes_to_send, SMPL_TXOPTION_ACKREQ);
      if (status == SMPL_SUCCESS) {
        break; //Done
      }
      if (retry_num == 255) {
        break; // Give up
      }
      retry_num++;

    }
    if (status == SMPL_SUCCESS) {
      //output("Success. ");
    } else {
      //output("Failed. ");
      break;
    }
    //output("Number of attempts: "); outputln_int(retry_num);
    
    send_buffer += bytes_to_send; //Move the pointer
    len -= bytes_to_send; //Reduce remaining bytes to send
  }  
}

void radio_receive() {
  uint8_t len;
  SMPL_Receive(sLinkID1, (uint8_t*)read_buff, &len);
  for (int i=0; i<len; i++) {
    char c = read_buff[i];
    read_buff[i] = '\0';
    if (c == ';') {
      command[command_pos] = '\0';
      command_pos = 0;
      process_received_command(command);
    } else {
      if (command_pos < sizeof(command)) {
        command[command_pos++] = c;
      } else {
        command_pos = 0;
        command[++command_pos] = c;
      }
    }
  }
}