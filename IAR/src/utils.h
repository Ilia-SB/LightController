#ifndef UTILS_H
#define UTILS_H
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "uart_intfc.h"
#include "atcommands.h"
#include "radio.h"

uint8_t split_param_string(const char*, char [][AT_SINGLE_PARAM_LEN]);
bool strings_equal(const char*, const char*);
bool string_contains_pos(const char*, const char*, uint8_t*);
bool string_starts_with(const char*, const char*);
bool string_contains(const char*, const char*);
bool string_equals(const char*, const char*);
void output(const char *);
void outputln(const char *);
void output_int(int n);
void outputln_int(int n);
void output_uint_hex(unsigned int n);
void outputln_uint_hex(unsigned int n);
void itoa(int n, char s[]);
void uitoa_hex(unsigned int num, char* buff);
void reverse(char s[]);
void report(uint8_t light_num, uint8_t state, uint8_t brightness);
#endif