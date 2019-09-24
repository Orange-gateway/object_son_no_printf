#ifndef _MYPTHREAD_H_
#define _MYPTHREAD_H_
#include "main.h"
#include <stdint.h>
/*线程串口接收函数*/
void pthread_usart_receive(void);
/*线程接收服务器函数*/
void pthread_client_receive(void);

void heart_jump(void);

void gateway_send_heart_jump(void);

/*线程发送至串口函数*/
void pthread_u_send(char *str);

/*线程发送至服务器函数*/
void pthread_v_send(uint8_t *u_data);

void dev_into_net(uint8_t *u_data);

void sign_mac_zt(uint8_t *mac);

int up_sign_mac(uint8_t *data);

int mac_and_mac_judge(SIG_MAC *p,uint8_t *mac);


#endif
