#include "dimmer.h"

/* external variables */
extern uint8_t lights_process_interval;

/* local variables */
static union t_byte_data *lights[4];
static union t_byte_data light0;
static union t_byte_data light1;
static union t_byte_data light2;
static union t_byte_data light3;
static union t_pwm_config_byte_data *pwm_config;
static union t_pwm_config_byte_data pwm_config_union;
static uint16_t counter_top;
static uint8_t lights_process_interval_old;

static uint16_t pwm_table[]  = { 0,662,1320,1974,2623,3267,3908,4543,5175,5802,6424,7043,7657,8266,8872,9473,10070,10662,11251,11835,12415,12990,13562,14129,14693,15252,15807,16358,16905,17447,17986,18521,19051,19578,20101,20619,21134,21645,22152,22654,23153,23649,24140,24627,25111,25590,26066,26539,27007,27471,27932,28389,28843,29292,29738,30181,30619,31054,31486,31913,32338,32758,33175,33589,33999,34405,34808,35207,35603,35996,36385,36770,37153,37531,37907,38279,38647,39013,39375,39733,40089,40441,40790,41135,41478,41817,42153,42486,42815,43142,43465,43785,44102,44416,44727,45035,45340,45641,45940,46236,46529,46818,47105,47389,47670,47948,48223,48495,48764,49031,49295,49555,49813,50069,50321,50571,50818,51062,51304,51542,51778,52012,52243,52471,52696,52919,53140,53357,53572,53785,53995,54203,54408,54610,54810,55008,55203,55396,55586,55774,55960,56143,56324,56503,56679,56853,57024,57194,57361,57525,57688,57848,58007,58163,58316,58468,58618,58765,58910,59053,59195,59334,59471,59606,59739,59870,59999,60126,60251,60374,60495,60614,60732,60847,60961,61072,61182,61290,61397,61501,61604,61705,61804,61902,61998,62092,62184,62275,62364,62451,62537,62621,62704,62785,62865,62943,63019,63094,63167,63239,63310,63379,63446,63512,63577,63640,63702,63763,63822,63880,63937,63992,64046,64099,64150,64200,64249,64297,64344,64389,64433,64476,64518,64559,64599,64637,64675,64711,64747,64781,64815,64847,64878,64909,64938,64967,64995,65024,65052,65081,65109,65138,65166,65195,65223,65251,65280,65308,65337,65365,65394,65422,65451,65479,65508,65535 };


/* functions */
void dimmer_init(){
  init_lights();
  timer1_init(pwm_table[255], pwm_config->Data.frequency_divider);
  counter_top = pwm_table[255] >> pwm_config->Data.frequency_divider;
  timer4_init();
  //enable_watchdog();
}

void set_brightness(struct t_light *light, uint8_t brightness) {
  uint16_t pwm = calculate_pwm(brightness);
  
  //Timer uses compare mode 110: high when equal T1CC0, low when equal T1CC1
  switch(light->pin) {
  case 1:
    T1CC1L = LOWBYTE(pwm); //Set timer compare value for channel 1
    T1CC1H = HIGHBYTE(pwm);
    break;
  case 2:
    T1CC2L = LOWBYTE(pwm); //Set timer compare value for channel 2
    T1CC2H = HIGHBYTE(pwm);
    break;
  case 3:
    T1CC3L = LOWBYTE(pwm); //Set timer compare value for channel 3
    T1CC3H = HIGHBYTE(pwm);
    break;
  case 4:
    T1CC4L = LOWBYTE(pwm); //Set timer compare value for channel 4
    T1CC4H = HIGHBYTE(pwm);
    break;
  }

  light->brightness = brightness;
}

void power_off(struct t_light *light){
switch(light->pin) {
  case 1:
    T1CC1L = 0;
    T1CC1H = 0;
    T1CCTL1 &= ~(BV(5) | BV(4)); //Stop timer 1 on channel 1
    P1SEL &= ~(BV(1)); //Map pin 1 in Port1 to gpio
    P1_1 = 0;
    break;
  case 2:
    T1CC2L = 0;
    T1CC2H = 0;
    T1CCTL2 &= ~(BV(5) | BV(4)); //Stop timer 1 on channel 2
    P1SEL &= ~(BV(0)); //Map pin 0 in Port1 to gpio
    P1_0 = 0;
    break;
  case 3:
    T1CC3L = 0;
    T1CC3H = 0;
    T1CCTL3 &= ~(BV(5) | BV(4)); //Stop timer 1 on channel 3
    P0SEL &= ~(BV(7)); //Map pin 7 in Port0 to gpio
    P0_7 = 0;
    break;
  case 4:
    T1CC4L = 0;
    T1CC4H = 0;
    T1CCTL4 &= ~(BV(5) | BV(4)); //Stop timer 1 on channel 4
    P0SEL &= ~(BV(6)); //Map pin 6 in Port1 to gpio
    P0_6 = 0;
    break;
  }
  
  //current_brightness = 0;
  light->is_switched_on = false;
}

void power_on(struct t_light *light) {
  switch(light->pin) {
  case 1:
    T1CC1L = 0; //Set timer compare value for channel 1
    T1CC1H = 0;
    T1CCTL1 |= (BV(5) | BV(4) | BV(3)); //Initialize pin (to 0)    
    T1CCTL1 |= (BV(5) | BV(4)); //Set compare mode 110: high when equal T1CC0, low when equal T1CC2
    P1SEL |= BV(1); //Map pin 1 in Port1 to preipheral
    break;
  case 2:
    T1CC2L = 0; //Set timer compare value for channel 2
    T1CC2H = 0;
    T1CCTL2 |= (BV(5) | BV(4) | BV(3)); //Initialize pin (to 0)
    T1CCTL2 |= (BV(5) | BV(4)); //Set compare mode 110: high when equal T1CC0, low when equal T1CC2
    P1SEL |= BV(0); //Map pin 0 in Port1 to peripheral
    break;
  case 3:
    T1CC3L = 0; //Set timer compare value for channel 3
    T1CC3H = 0;
    T1CCTL3 |= (BV(5) | BV(4) | BV(3)); //Initialize pin (to 0)
    T1CCTL3 |= (BV(5) | BV(4)); //Set compare mode 110: high when equal T1CC0, low when equal T1CC2
    P0SEL |= BV(7); //Map pin 7 in Port0 to peripheral
    break;
  case 4:
    T1CC4L = 0; //Set timer compare value for channel 4
    T1CC4H = 0;
    T1CCTL4 |= (BV(5) | BV(4) | BV(3)); //Initialize pin (to 0)
    T1CCTL4 |= (BV(5) | BV(4)); //Set compare mode 110: high when equal T1CC0, low when equal T1CC2
    P0SEL |= BV(6); //Map pin 6 in Port0 to peripheral
    break;
  }

  light->is_switched_on = true;
}

void process_at_command(const char *command) {
  char at_command[AT_COMMAND_LEN];
  uint8_t separator_pos; //will hold position of '=' in command string
  if (!string_contains_pos(command, "=", &separator_pos)) { //AT command without '='
    int command_len = strlen(command);
    strncpy(at_command, command, command_len + 1);
    
    if (string_equals(at_command, "AT+LISTPARAMS")) {
      at_list_params();
    } else if (string_equals(at_command, "AT+RESET")) {
      while(1); //watchdog will reset
    } else if(string_equals(at_command, "AT+STOREPWMCONFIG")) {
      at_store_pwm_config();
    } else if(string_equals(at_command, "AT+LISTREMOTES")) {
      at_list_remotes();
    } else if(string_equals(at_command, "AT+SETDEFAULTS")) {
      at_set_defaults();
    } else {
      outputln("Unknown AT command");
    }
  } else { //AT command with '='
    int at_command_len = separator_pos; //AT command length
    int param_len = strlen(command) - separator_pos;
  
    char param_string[AT_PARAMS_LEN];
    memcpy(at_command, command, at_command_len);
    at_command[at_command_len] = '\0';
    memcpy(param_string, command + at_command_len + 1, param_len);
    param_string[param_len] = '\0';
      
    if(string_equals(at_command, "AT+SETPARAMS")) {
      at_set_params(param_string);
    } else if (string_equals(at_command, "AT+SETPWMPARAMS")){
      at_set_pwm_params(param_string);
    } else if(string_equals(at_command, "AT+STORECONFIG")) {
      at_store_config(param_string);
    } else if(string_equals(at_command, "AT+LOADCONFIG")) {
      at_load_config(param_string);
    } else if (string_equals(at_command, "AT+REGISTERREMOTE")) {
      at_register_remote(param_string);
    } else if (string_equals(at_command, "AT+UNREGISTERREMOTE")) {
      at_unregister_remote(param_string);
    } else {
      outputln("Unknown AT command");
    }
  }
}

void process_command(const char *command) {
  struct t_light *light;
  char params[2][AT_SINGLE_PARAM_LEN];
  
  if (string_equals(command, "OVERFLOW")) {
    outputln("Command buffer overflow");
  }
  int param_num = split_param_string(command, params);
  if (param_num != 2) {
    outputln("Incorrect command");
    return;
  }
  int light_addr = atoi(params[0]);
  for (int i=0; i<4; i++) {
    if (lights[i]->Data.light_address == light_addr) {
      light = &lights[i]->Data;
    }
  }
  
  if (light == NULL) {
    output("Light "); output_int(light->light_address); outputln(" not found");
    return;
  }

  if (string_equals(params[1], "ON")) {
    if (!light->is_switched_on) {//if not already on
      light_turn_on(light);
      //output(params[0]); outputln(params[1]);
    }
  } else if (string_equals(params[1], "OFF")) {
    if (light->is_switched_on) {//if not already off
      light_turn_off(light);
      //power_off(light);
    }
  } else if (string_contains(params[1], "%")) { //if setting brightness in percentages
    int perc = atol(params[1]);
    if (perc < 0 || perc > 100) {
      outputln("Incorrect brightness setting");
      return;
    }
    uint16_t brightness = light->max_brightness * perc / 100;
    change_brightness(light, brightness);
  } else { //if setting brightness in digits
    uint16_t brightness = atol(params[1]);
    if (brightness > light->max_brightness) {
      outputln("Incorrect brightness setting");
      return;
    }
    change_brightness(light, brightness);
  }
}

void init_light_struct(union t_byte_data *light, uint16_t light_address,
                              uint8_t pin, uint8_t max_brightness, uint8_t min_brightness) {
  light->Data.light_address = light_address;
  light->Data.pin = pin;
  light->Data.max_brightness = max_brightness;
  light->Data.min_brightness = min_brightness;
  light->Data.brightness = 0;
  light->Data.on_brightness = max_brightness;
  light->Data.target_brightness = 0;
  light->Data.increment = 0;
  light->Data.is_switched_on = false;
}

void write_config(int num, union t_byte_data *light) {
   flash_page_erase(7, num);
   int len = sizeof(light->Bytes);
   if (len%4 != 0) //number of bytes must be a factor of 4
     len = len / 4 * 4 + 4;
   flash_page_write(7, num, 0, light->Bytes, len);
}

void write_pwm_config(union t_pwm_config_byte_data *pwm_config) {
   flash_page_erase(7, 4);
   int len = sizeof(pwm_config->Bytes);
   if (len%4 != 0) //number of bytes must be a factor of 4
     len = len / 4 * 4 + 4;
   flash_page_write(7, 4, 0, pwm_config->Bytes, len);
}

void read_config(int num, union t_byte_data *light) {
  int len = sizeof(light->Bytes);
  /*
  if (len%4 != 0) //number of bytes must be a factor of 4
     len = len / 4 * 4 + 4;
  */
  flash_page_read(7, num, 0, light->Bytes, len);
}

void read_pwm_config(union t_pwm_config_byte_data *pwm_config) {
  int len = sizeof(pwm_config->Bytes);
  /*
  if (len%4 != 0) //number of bytes must be a factor of 4
     len = len / 4 * 4 + 4;
  */
  flash_page_read(7, 4, 0, pwm_config->Bytes, len);
}

void change_brightness(struct t_light *light, uint8_t new_brightness) {
  if (new_brightness > light->brightness) {
    light->increment = 1;
    light->target_brightness = new_brightness;
  }
  else if (new_brightness < light->brightness) {
    light->increment = -1;
    light->target_brightness = new_brightness;
  }  
}

void process_lights() {
  for (int i=0; i<4; i++) { //cycle through all lights on this controller
    struct t_light *light = &lights[i]->Data;
    if (light->increment != 0) { //if brightness needs to be changed
      if (!light->is_switched_on) { //if light is off
        power_on(light); //turn it on
      }
      set_brightness(light, light->brightness + light->increment); //change brightness 1 step
      if (light->brightness == light->target_brightness) {//if target brightness is reached
        light->increment = 0; //set increment to 0
        if (light->brightness == 0) {
          power_off(light);
        }
        //write_config(i, lights[i]);
        report(light->light_address, light->is_switched_on, light->brightness);
      }
    }
  }
}

//void init_lights(union t_byte_data *(*lights)[4], union t_pwm_config_byte_data *pwm_config_union) {
void init_lights() {
  pwm_config = &pwm_config_union;
  pwm_config->Data.frequency_divider = 0;
  read_pwm_config(pwm_config);
  lights[0] = &light0;
  lights[1] = &light1;
  lights[2] = &light2;
  lights[3] = &light3;
  for (int i=0; i<4; i++) {
    read_config(i, lights[i]);
    struct t_light *light = &lights[i]->Data;
    light->is_switched_on = false;
    light->target_brightness = 0;
    //light->on_brightness = light->max_brightness;
    light->brightness = 0;
    light->increment = 0;
  }
}

uint16_t calculate_pwm(const uint8_t brightness) {
  /*
  uint16_t brightness = light->brightness;
  double p = pwm_config->Data.power_coeff * brightness;
  double pw = pow(2.7182818, p);
  double result = pwm_config->Data.k * pw;
  uint16_t uresult = (uint16_t)result;
  return uresult + light->pwm_adjust;
  */
  uint16_t ret_val;
  //ret_val = (pwm_table[brightness]) / pwm_config->Data.frequency_divider;
  ret_val = pwm_table[brightness] >> pwm_config->Data.frequency_divider;
  if (ret_val == counter_top) { //if we return ret_val equal to counter_top the light goes off so return (counter_top-1)
    return counter_top - 1;
  }
  return ret_val;
}

static void at_list_params() {
      outputln("\r\nPWM config:");
      output("frequency divider: \t"); output_int(pwm_config->Data.frequency_divider); outputln("");
      for (int i=0; i<4; i++) {
        struct t_light *light = &lights[i]->Data;
        output("\r\nLight "); output_int(i+1); output(":\r\n");
        output("address: \t"); output_int(light->light_address); output("\r\n");
        output("pin: \t"); output_int(light->pin); output("\r\n");
        output("is switched: \t"); output_int(light->is_switched_on); output("\r\n");
        output("brightness when on: \t"); output_int(light->on_brightness); output("\r\n");
        output("current brightness: \t"); output_int(light->brightness); output("\r\n");
        output("target brightness: \t"); output_int(light->target_brightness); output("\r\n");
        output("maximum brightness: \t"); output_int(light->max_brightness); output("\r\n");
        output("minimum brightness: \t"); output_int(light->min_brightness); output("\r\n");
        output("increment: \t"); output_int(light->increment); output("\r\n");
      }
}

static void at_store_pwm_config() {
      write_pwm_config(pwm_config);
      outputln("OK");
}

static void at_set_params(const char *param_string) {
      char params[5][AT_SINGLE_PARAM_LEN];
      int param_num = split_param_string(param_string, params);
      if (param_num != 5) {
        outputln("Incorrect number of parameters");
        return;
      }
  
      for (int i=0; i<param_num; i++) {
        output(params[i]);
        output(", ");
      }
      outputln("");
      
      uint8_t  num =            (uint8_t)strtoul(params[0], NULL, 10);
      uint16_t light_address =  strtoul(params[1], NULL, 10);
      uint8_t  pin =            (uint8_t)strtoul(params[2], NULL, 10);
      uint8_t  max_brightness = (uint8_t)strtoul(params[4], NULL, 10);
      uint8_t  min_brightness = (uint8_t)strtoul(params[3], NULL, 10);
  
      init_light_struct(lights[num-1], light_address, pin, max_brightness, min_brightness);
      outputln("OK");
}

static void at_set_defaults() {
  for (int i=0; i<4; i++) {
    struct t_light *light = &lights[i]->Data;
    light->brightness = 0;
    light->increment = 0;
    light->is_switched_on = false;
    light->known_remotes_num = 0;
    light->light_address = 0;
    light->max_brightness = 255;
    light->min_brightness = 0;
    light->on_brightness = 0;
    light->pin = i+1;
    light->target_brightness = 0;
    light->dimming_direction = UP;
  }
  outputln("OK");
}

static void at_set_pwm_params(const char *param_string) {
        char params[1][AT_SINGLE_PARAM_LEN];
        int param_num = split_param_string(param_string, params);
        if (param_num != 1) {
        outputln("Incorrect number of parameters");
        return;
      }
      uint8_t divider = (uint8_t)strtoul(params[0], NULL, 10);
      pwm_config->Data.frequency_divider = divider;
      timer1_init(pwm_table[255], pwm_config->Data.frequency_divider);
      outputln("OK");
}

static void at_store_config(const char *param_string) {
      char params[1][AT_SINGLE_PARAM_LEN];
      int param_num = split_param_string(param_string, params);
      if (param_num != 1) {
        outputln("Incorrect number of parameters");
        return;
      }
      if (string_equals(params[0], "ALL")) {
        for (int i=0; i<4; i++) {
          write_config(i, lights[i]);
        }
        write_pwm_config(pwm_config);
      } else {
        int num = atoi(params[0]);
        write_config(num, lights[num]);
      }
      outputln("OK");
}

static void at_load_config(const char *param_string) {
      char params[1][AT_SINGLE_PARAM_LEN];
      int param_num = split_param_string(param_string, params);
      if (param_num != 1) {
        outputln("Incorrect number of parameters");
        return;
      }
      
      int num = atoi(params[0]);
      read_config(num, lights[num]);
      outputln("OK");
}

static void at_list_remotes() {
  outputln("Registered remotes:");
  for (uint8_t i=0; i<4; i++) {
    output("Light "); output_int(i+1); outputln("");
    struct t_light *light = &lights[i]->Data;
    for (uint8_t j=0; j<light->known_remotes_num; j++) {
       output_int(j); output(": ");
       output_uint_hex(light->known_remotes[j*5]);
       output_uint_hex(light->known_remotes[j*5+1]);
       output_uint_hex(light->known_remotes[j*5+2]);
       output_uint_hex(light->known_remotes[j*5+3]);
       outputln_uint_hex(light->known_remotes[j*5+4]);
    }
  }
}

static void at_register_remote(const char *param_string) {
  char params[6][AT_SINGLE_PARAM_LEN];
  uint8_t light_num;
  uint8_t remote[5];
  int param_num = split_param_string(param_string, params);
  if (param_num != 6) {
    outputln("Incorrect number of parameters");
    return;
  }
  
  light_num = (uint8_t)strtoul(params[0], NULL, 10) - 1; //lights are 1-based hence -1
  remote[0] = (uint8_t)strtoul(params[1], NULL, 16);
  remote[1] = (uint8_t)strtoul(params[2], NULL, 16);
  remote[2] = (uint8_t)strtoul(params[3], NULL, 16);
  remote[3] = (uint8_t)strtoul(params[4], NULL, 16);
  remote[4] = (uint8_t)strtoul(params[5], NULL, 16);

  //TODO: Add check if remote is already known
  struct t_light *light = &lights[light_num]->Data;
  if (light->known_remotes_num >= REMOTES_NUM) {
    outputln("Too many remotes registered for the light. Can not register more.");
    return;
  } else {
    /*Check if remote is already known*/
    for (int i=0; i<light->known_remotes_num; i++) {
      if (arrays_equal(remote, light->known_remotes + i*5, 5)) {
        outputln("Remote already known.");
        return;
      }
    }
    memcpy(light->known_remotes + light->known_remotes_num * 5, remote, 5);
    light->known_remotes_num++;
    outputln("OK");
  }
}

static void at_unregister_remote(const char *param_string) {
  char params[2][AT_SINGLE_PARAM_LEN];
  int param_num = split_param_string(param_string, params);
  if (param_num != 2) {
    outputln("Incorrect number of parameters");
    return;
  }
  
  uint8_t light_num = (uint8_t)strtoul(params[0], NULL, 10) - 1; //lights are 1-based hence -1
  struct t_light *light = &lights[light_num]->Data;  
  
  if (string_equals(params[1], "ALL")) {
    light->known_remotes_num = 0;
  } else {
    uint8_t remote_num = (uint8_t)strtoul(params[1], NULL, 10);
    memcpy(light->known_remotes + remote_num * 5,
           light->known_remotes + (remote_num + 1) * 5,
           (light->known_remotes_num - remote_num - 1) * 5);
    light->known_remotes_num--;
  }
  outputln("OK");
}

void process_received_command(const char *command) {
  outputln("");
  if (string_starts_with(command, "AT+")) {
    process_at_command(command);
  } else {
    process_command(command);
  }
}

void process_remote(const uint8_t *remote) {
  for (int i=0; i<4; i++) {
    struct t_light *light = &lights[i]->Data;
    for (int j=0; j<light->known_remotes_num; j++) {
      if (arrays_equal(light->known_remotes + j*5, remote, 5)) {
        //outputln("Remote match");
        if (light->is_switched_on) {
          light_turn_off(light);
        } else {
          light_turn_on(light);
        }
        break;
      }
    }
  }
}

void start_dimming(const uint8_t *remote) {
  outputln("Start dimming");
  for (int i=0; i<4; i++) {
    struct t_light *light = &lights[i]->Data;
    for (int j=0; j<light->known_remotes_num; j++) {
      if (arrays_equal(light->known_remotes + j*5, remote, 5)) {
        lights_process_interval_old = lights_process_interval; //Store the normal interval
        lights_process_interval = 50; //Set a new interval for slow dimming
        if(!light->is_switched_on) { //If light is off, turn it on and dim up
          power_on(light);
          light->dimming_direction = UP;
        }
        if(light->dimming_direction == UP) {
          light->target_brightness = light->max_brightness;
          light->increment = 1;
        } else {
          light->target_brightness = light->min_brightness;
          light->increment = -1;
        }
        break;
      }
    }
  }  
}

void stop_dimming(const uint8_t *remote) {
  outputln("Stop dimming");
  lights_process_interval = lights_process_interval_old;
  for (int i=0; i<4; i++) {
    struct t_light *light = &lights[i]->Data;
    for (int j=0; j<light->known_remotes_num; j++) {
      if (arrays_equal(light->known_remotes + j*5, remote, 5)) {
        light->increment = 0; //Stop dimming
        if (light->brightness != 0) {
          light->target_brightness = light->brightness; //Store current brightness value
        }
        if (light->dimming_direction == UP) { //Switch dimming direction for future dimming
          light->dimming_direction = DOWN;
        } else {
          light->dimming_direction = UP;
        }
        write_config(i, lights[i]); //Store config
        report(light->light_address, light->is_switched_on, light->brightness);
        break;
      }
    }
  }
}

static void light_turn_on(struct t_light *light) {
  //power_on(light);
  uint8_t target_brightness = light->on_brightness;
  //light->brightness = 0;
  change_brightness(light, target_brightness);
}

static void light_turn_off(struct t_light *light) {
  light->on_brightness = light->brightness; //store current brightness
  uint16_t target_brightness = 0;
  change_brightness(light, target_brightness);
}

