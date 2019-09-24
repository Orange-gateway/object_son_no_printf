#ifndef _MAIN_H_
#define _MAIN_H_
#include "stdint.h"
#include <time.h>
#include <pthread.h>
 int cd;//socket 描述符
 int fd;//串口描述符
 int NET_FLAG; 
 int go_net_flag;//允许入网标志
 time_t first;//接收更新时间
 time_t second;//心跳更新时间
 time_t net_record_time;//入网记录时间
 pthread_t id_client;
 uint8_t my_gw_mac[6];
 
 typedef struct sign_mac
 {
 	uint8_t the_mac[8];
 	int flag_have;
 	struct sign_mac *next;
 }SIG_MAC;
 SIG_MAC *sign_mac_head,*sign_mac_z,*sign_mac_d;
 
#endif
