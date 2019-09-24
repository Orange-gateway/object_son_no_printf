#ifndef _S_FUNC_H_
#define _S_FUNC_H_
#include "include.h"
void delete_len_from_arr(uint8_t *arr,int n,int *len_of_arr);
void delete_len_from_str(char *str,int n,int *len_of_str);
int replace_character(char *string);
pthread_t pth_creat_my(void *func,void *s_data);
void int_to_str(int num,char *str);
char int_to_char(int i);
int char_to_int(char b);
void hex_to_str(uint8_t  *str_unchar,char *str_char,int len_of_hex);
void str_to_hex(uint8_t *str_unchar,char *str_char,int len_of_hex);
int reboot_restart();
void kill_gateway(void);
void get_hex_mac(uint8_t *data);
#endif
