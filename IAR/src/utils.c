#include "utils.h"

static char hex [] = { '0', '1', '2', '3', '4', '5', '6', '7',
                        '8', '9' ,'A', 'B', 'C', 'D', 'E', 'F' };
static char temp_buffer[64];

extern uint8_t mirror_output_to_radio;

uint8_t split_param_string(const char *string, char dest[][AT_SINGLE_PARAM_LEN]) {
  int len = strlen(string);
  if (len + 1 > AT_PARAMS_LEN) //prevent buffer overflow
  {
    return 0;
  }
  char str[AT_PARAMS_LEN]; //temp buffer for strtok because it destroys source string
  strncpy(str, string, len + 1);
  int result = 0;
  char *entry;
  char sep[2] = ",";
  
  entry = strtok(str, sep);
  while (entry != NULL) {
    if (strlen(entry) + 1 > AT_SINGLE_PARAM_LEN) { //prevent buffer overflow
      return 0;
    }
    //dest[result++] = entry;
    memcpy(dest[result++], entry, strlen(entry) + 1);
    entry = strtok(NULL, sep);
  }
  return result;
}

bool strings_equal(const char *string1, const char *string2) {
  if (strcmp(string1, string2) == 0) {
    return true;
  } else {
    return false;
  }
}

bool string_contains_pos(const char *string1, const char *string2, uint8_t *pos) {
  char *substr = strstr(string1, string2);
  if (substr != NULL) {
    *pos = substr - string1;
    return true;
  } else {
    *pos = 0;
    return false;
  }
}
  
bool string_starts_with(const char *string1, const char *string2) {
  uint8_t pos = 0;
  if (string_contains_pos(string1, string2, &pos)) {
    if (pos == 0) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool string_contains(const char *string1, const char *string2) {
  return string_contains_pos(string1, string2, NULL);
}

bool string_equals(const char* string1, const char* string2) {
  if (strcmp(string1, string2) == 0) {
    return true;
  } else {
    return false;
  }
}

void uitoa_hex(unsigned int num, char* buff) {
    int len=0,k=0;
    do//for every 4 bits
    {
        //get the equivalent hex digit
        buff[len] = hex[num & 0xF];
        len++;
        num>>=4;
    }while(num!=0);
    //since we get the digits in the wrong order reverse the digits in the buffer
    for(;k<len/2;k++)
    {//xor swapping
        buff[k]^=buff[len-k-1];
        buff[len-k-1]^=buff[k];
        buff[k]^=buff[len-k-1];
    }
    //null terminate the buffer and return the length in digits
    buff[len]='\0';
}

void itoa(int n, char s[]) {
     int i, sign;
 
     if ((sign = n) < 0)  /* записываем знак */
         n = -n;          /* делаем n положительным числом */
     i = 0;
     do {       /* генерируем цифры в обратном порядке */
         s[i++] = n % 10 + '0';   /* берем следующую цифру */
     } while ((n /= 10) > 0);     /* удаляем */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}

void uiatoa(char *output_array, const uint8_t *input_array, uint8_t len) {
  for (int i=0; i<len; i++) {
      uitoa_hex(input_array[i], temp_buffer);
      strcat(output_array, temp_buffer);
  }
}

 void reverse(char s[]) {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}

void output(const char *string) {
  tx_send_wait(string, strlen(string));
  if(mirror_output_to_radio) {
    radio_transmit(string, strlen(string));
  }
}

void output_int(int n) {
  itoa(n, temp_buffer);
  output(temp_buffer);
}

void outputln_int(int n) {
  itoa(n, temp_buffer);
  outputln(temp_buffer);  
}

void outputln(const char *string) {
  output(string);
  output("\r\n");
}

void output_uint_hex(unsigned int n) {
  uitoa_hex(n, temp_buffer);
  output(temp_buffer);
}

void outputln_uint_hex(unsigned int n) {
  uitoa_hex(n, temp_buffer);
  outputln(temp_buffer);
}

void report(uint8_t light_num, uint8_t state, uint8_t brightness) {
  output_int(light_num); output(",");
  output_int(state); output(",");
  output_int(brightness); outputln(";");
}

bool arrays_equal(const uint8_t* arr1, const uint8_t* arr2, uint8_t len) {
  for (int i=0; i<len; i++) {
    if (arr1[i] != arr2[i]) {
      return false;
    }
  }
  return true;
}
