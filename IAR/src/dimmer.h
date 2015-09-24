#ifndef DIMMER_H
#define DIMMER_H

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "utils.h"
#include "timer.h"
#include "queue.h"
#include "bsp.h"
#include "nv.h"
#include "atcommands.h"

/* defines */
#define LOWBYTE(v)   ((unsigned char) (v))
#define HIGHBYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8))

#define REMOTES_NUM     16

typedef enum {
  UP = 0,
  DOWN = 1
} t_dimming_direction;

/* Structures */
struct t_light {
  uint16_t              light_address;
  uint8_t               pin;
  bool                  is_switched_on;
  uint8_t               on_brightness;
  uint8_t               brightness;
  uint8_t               target_brightness;
  uint8_t               max_brightness;
  uint8_t               min_brightness;
  uint8_t               known_remotes[REMOTES_NUM * 5]; //5 bytes per remote
  uint8_t               known_remotes_num;
  int                   increment;
  t_dimming_direction   dimming_direction;
};

struct t_pwm_config {
  uint8_t      frequency_divider; //100 => ~50khz (?)
};

union t_byte_data {
  struct t_light Data;
  char Bytes[sizeof(struct t_light)];
};

union t_pwm_config_byte_data {
  struct t_pwm_config Data;
  char Bytes[sizeof(struct t_pwm_config)];
};

/* dimmer logic */
void dimmer_init(void);
void set_brightness(struct t_light *, uint8_t);
void power_off(struct t_light *);
void power_on(struct t_light *);
void process_received_command(const char*);
void process_at_command(const char *command);
void process_command(const char *);
void init_light_struct(union t_byte_data *, uint16_t, uint8_t, uint8_t, uint8_t);
void write_config(int, union t_byte_data *);
void write_pwm_config(union t_pwm_config_byte_data *);
void read_config(int, union t_byte_data *);
void read_pwm_config(union t_pwm_config_byte_data *);
void process_lights(void);
void change_brightness(struct t_light *, uint8_t);
void init_lights(void);
uint16_t calculate_pwm(const uint8_t);
static void at_list_params(void);
static void at_store_pwm_config(void);
static void at_set_params(const char*);
static void at_set_pwm_params(const char*);
static void at_store_config(const char*);
static void at_load_config(const char*);
static void at_list_remotes(void);
static void at_register_remote(const char*);
static void at_unregister_remote(const char*);
static void at_set_defaults(void);
void process_remote(uint32_t);
void start_dimming(uint32_t);
void stop_dimming(uint32_t);
static void light_turn_on(struct t_light *);
static void light_turn_off(struct t_light *);

#endif