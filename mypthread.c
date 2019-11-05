#include "include.h"

/*线程串口接收函数*/
void pthread_usart_receive(void)
{
	int i,len,arr_len=0,data_len,flag_break;
	uint8_t r_buff[1024];//串口接收缓冲区
	uint8_t rc_buff[10240];//中间转换缓冲区
	memset(r_buff,0,1024);
	memset(rc_buff,0,10240);
	
	while(1)
	{
		len = read(fd,r_buff,sizeof(r_buff));
		if(len>0)
		{
			for(i=0;i<len;i++)
			{
				rc_buff[arr_len]=r_buff[i];
				arr_len+=1;
			}
			memset(r_buff,0,1024);
			
		}

		while(1)
		{
			if(arr_len > 15)
			{
				flag_break = 0;//break标志位
				for(i=0;i<arr_len;i++)
				{
					if((rc_buff[i]==0x5A)&&(rc_buff[i+1]==0xA5))//找到一帧数据的包头
					{
						if(i>0)
						{
							delete_len_from_arr(rc_buff,i,&arr_len);
						}
						if(arr_len > 15)
						{
							data_len = rc_buff[13]*256+rc_buff[14]+15;

							if(arr_len >= data_len)//判断总数据的长度是否大于等于这一帧数据的长度
							{
					
								uint8_t *my_u_data = NULL;
								my_u_data =(uint8_t*)malloc(data_len);//解析数据接收缓冲区
								memset(my_u_data,0,data_len);

								for(i=0;i<data_len;i++)
									my_u_data[i]=rc_buff[i];

								if(up_sign_mac(my_u_data) == 1)//判断标记的mac的链表中是否和这一帧数据的mac相同，如果相同则发送mac信息给主网关
								{
									uint8_t *notice_mac = NULL;
									notice_mac = (uint8_t *)malloc(21);
									memset(notice_mac,0,21);
									memcpy(notice_mac,my_u_data,15);
									memcpy(notice_mac+15,my_gw_mac,6);
									notice_mac[10] = 0xf2;
									notice_mac[13] = 0x00;
									notice_mac[14] = 0x06;
									pth_creat_my(pthread_v_send,notice_mac);
								}
								pth_creat_my(pthread_v_send,my_u_data);
								
								delete_len_from_arr(rc_buff,data_len,&arr_len);
							}
							else
								flag_break = 1;
						}
						break;
					}
					else if(i==arr_len-1)
						flag_break = 1;
				}
				if(flag_break) break;
			}
			else
				break;
		}
	}
}

/*线程接收服务器函数*/
void pthread_client_receive(void)
{
	int flag=0,ret_recv,str_len=0,i,num=0;
	char c_send[1024],str_from_server[10240];//客户端数据缓冲区
	memset(str_from_server,0,10240);
	memset(c_send,0,1024);
	while(1)
	{	
		ret_recv=recv(cd,c_send,1024,0);

		if(ret_recv>0)
		{	
			for(i=0;i<ret_recv;i++)
			{
				str_from_server[str_len]=c_send[i];
				str_len+=1;
			}
			memset(c_send,0,sizeof(c_send));
			
			
		}
		else if(ret_recv == 0)
		{
			num++;
			if(num>5)
			{
				printf("server is gone\n");
				kill_gateway();
			}
		}
		else
		{
			sleep(1);
			continue;
		}
		
		while(1)
		{
			if(str_len>15)
			{
				
				flag=0;
				
				for(i=14;i<str_len;i++)
				{
					if(str_from_server[i]=='\n')
					{
						flag=i;
						break;
					}	
				}
				
				if(flag>=15)
				{
					char *r_str = NULL;
					r_str=(char *)malloc(flag+1);//解析数据接收缓冲区

					memset(r_str,0,sizeof(r_str));

					memcpy(r_str,str_from_server,flag+1);
					r_str[flag]='\0';
					replace_character(r_str);
					if(json_checker(r_str)==0)
					{
						first = time(NULL);
						pth_creat_my(pthread_u_send,r_str);
					}
					else
					{
						free(r_str);
						r_str=NULL;
					}
					
					delete_len_from_str(str_from_server,flag+1,&str_len);
				}	 
				else if(flag==14)
				{
				 	delete_len_from_str(str_from_server,flag+1,&str_len);
				}
				else
					break;
			}
			else
			{
				break;
			}
					
		}

	}
}

/*线程发送至串口函数*/
void pthread_u_send(char *str)
{
	int len = 0;
	cJSON *root = cJSON_Parse(str);//需要释放（1）
	cJSON *api = cJSON_GetObjectItem(root,"api");
	if( strcmp("devCmd",api->valuestring) == 0 )//common device control
	{
		cJSON *mac_get = cJSON_GetObjectItem(root,"gw_mac");

		cJSON *command = cJSON_GetObjectItem(root,"command");
		len = strlen(command->valuestring);
		len = len/2;
		uint8_t *final_cmd = (uint8_t *)malloc(len);
		memset(final_cmd,0,len);
		str_to_hex(final_cmd,command->valuestring,len);
		if( mac_get->valueint == 0 && final_cmd[10] != 0x20 && final_cmd[10] != 0x21 && final_cmd[10] != 0x55)//主网关找不到控制设备，并且不是允许入网和禁止入网和删除设备
		{
			sign_mac_zt(final_cmd);//标记mac
			if(final_cmd[10] == 0x51)//如果是红外设备的控制，就发送40获取红外设备的mac
			{
				uint8_t get_mac_hw[16] = {0x5a,0xa5,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x01,0x00};
				memcpy(get_mac_hw+2,final_cmd+2,8);
				usart_send(fd,get_mac_hw,16);
			}
		}
		usart_send(fd,final_cmd,len);
		free(final_cmd);
		final_cmd = NULL;
	}
	free(str);
	str = NULL;
	cJSON_Delete(root);
	root = NULL;
	pthread_detach(pthread_self());//自身分离属性线程，系统自动清除资源
	pthread_exit((void *)0);
}


/*线程发送至服务器函数*/
void pthread_v_send(uint8_t *u_data)
{
	int u_data_len = u_data[13]*256+u_data[14]+15;
	if(u_data[10]==0x20 || u_data[10]==0x21)
		dev_into_net(u_data);
	else if(u_data[10]==0x10 && go_net_flag)
	{
		time_t net_now_time = time(NULL);
		if((unsigned int)difftime(net_now_time,net_record_time) >= 300)//超时
		{
			go_net_flag = 0;
			u_data[10] = 0x55;
			usart_send(fd,u_data,16);
		}
		else
		{
			u_data[10] = 0x40;
			u_data[15] = 0x00;
			usart_send(fd,u_data,16);
		}
	}
	else if(u_data[10] == 0x40 && NET_FLAG && go_net_flag)//判断是否正常连接着主网关,并且是允许入网的情况.
	{
		uint8_t *notice_mac = NULL;
		notice_mac = (uint8_t *)malloc(21);
		memset(notice_mac,0,21);
		memcpy(notice_mac,u_data,15);
		memcpy(notice_mac+15,my_gw_mac,6);
		notice_mac[10] = 0xf2;
		notice_mac[13] = 0x00;
		notice_mac[14] = 0x06;
		send(cd,notice_mac,21,0);
		u_data[10] = 0x41;
		send(cd,u_data,u_data_len,0);
		free(notice_mac);
		notice_mac = NULL;
	}
	else if(u_data[10] == 0x55)
	{
		;
	}
	else if(u_data[10] == 0x30)
	{
		memcpy(u_data+2,my_gw_mac,6);
		u_data[10] = 0xf5;
		send(cd,u_data,u_data_len,0);
	}
	else if(NET_FLAG)
	{
		send(cd,u_data,u_data_len,0);
	}
	free(u_data);
	u_data = NULL;
	pthread_detach(pthread_self());//自身分离属性线程，系统自动清除资源
	pthread_exit((void *)0);
}

/*心跳机制*/
void heart_jump(void)
{
	int time_diff=0;
	first = second = time(NULL);
	while(1)
	{
		second=time(NULL);
		if( (abs((unsigned int)difftime(second,first))-time_diff) > 2)
		{
			first = time(NULL);
		}
		time_diff = abs((unsigned int)difftime(second,first));
		if(time_diff > 30)
		{
			printf("first but have no heart-jump!!!\n");
			kill_gateway();
		}
		sleep(1);
	}
}
void gateway_send_heart_jump(void)
{
	uint8_t buff[16] = {0x5a,0xa5,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf1,0x00,0x00,0x00,0x01,0x00};
	while(1)
	{
		if(NET_FLAG)
			send(cd,buff,16,0);
		sleep(10);
	}
}

void dev_into_net(uint8_t *u_data)
{
	if(u_data[10] ==0x20)
	{
		go_net_flag=1;
		net_record_time = time(NULL);
	}
	else
	{
		go_net_flag=0;
	}
}
/*
@mac标记，登记进入链表
*/
void sign_mac_zt(uint8_t *mac)
{
	SIG_MAC *p = NULL;
	p = sign_mac_head;
	pthread_mutex_lock(&mutex_sign_mac_zt);
	if(p==NULL)
	{
		sign_mac_d = (SIG_MAC*)malloc(sizeof(SIG_MAC));
		memset(sign_mac_d,0,sizeof(SIG_MAC));
		memcpy(sign_mac_d->the_mac,mac+2,8);
		sign_mac_d->flag_have = 1;//1代表标记mac
		sign_mac_head = sign_mac_z = sign_mac_d;
		sign_mac_d->next = NULL;
	}
	else
	{
		while( p )
		{
			if(!mac_and_mac_judge(p,mac+2))
			{
				p->flag_have = 1;
				break;
			}
			else if(p->next == NULL)
			{
				sign_mac_d = (SIG_MAC*)malloc(sizeof(SIG_MAC));
				memset(sign_mac_d,0,sizeof(SIG_MAC));
				memcpy(sign_mac_d->the_mac,mac+2,8);
				sign_mac_d->flag_have = 1;
				p->next = sign_mac_d;
				sign_mac_d->next = NULL;
				break;
			}
			p = p->next;
		}
	}
	pthread_mutex_unlock(&mutex_sign_mac_zt);
}
/*
@取消mac链表中的标记
*/
int up_sign_mac(uint8_t *data)
{
	int ret = 0;
	if(data[10] == 0x10) return ret;
	SIG_MAC *p1 = NULL;
	p1 = sign_mac_head;
	pthread_mutex_lock(&mutex_sign_mac_zt);
	while(p1)
	{
		if(!mac_and_mac_judge(p1,data+2))
		{
			if( p1->flag_have )
			{
				p1->flag_have = 0;//0代表擦除标记
				ret = 1;
			}
			break;
		}
		p1 = p1->next;
	}
	pthread_mutex_unlock(&mutex_sign_mac_zt);
	return ret;
}
int mac_and_mac_judge(SIG_MAC *p,uint8_t *mac)
{
	int i=0,flag_success=0;
	for(i=0;i<8;i++)
	{
		if(p->the_mac[i]==mac[i])
		{
			flag_success+=1;
		}
	}
	if(flag_success==8)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

