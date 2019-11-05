#include "include.h"
void handler(int s)
{
	if(s == SIGBUS)
	{
		system("rm /bin/gateway");
		system("killall gateway");
	}
}
int main(int argc,char* argv[])
{
	memset(my_gw_mac,0,6);
	get_hex_mac(my_gw_mac);
	int ret_usart,ret0,ret1,ret2,ret3;          //接受返回值，用于判断
	go_net_flag = 0;
	NET_FLAG = 0;
	fd = usart_open(fd,argv[1]);
	do
	{
		ret_usart = usart_init(fd,115200,0,8,'n',1);
	}while(ret_usart == -1 || fd == -1);
	
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction( SIGPIPE, &sa, 0 );
	
	pthread_mutex_init(&mutex_sign_mac_zt,NULL);
	
	pthread_t id_usart,id_heart_jump,id_gateway_heart;
	ret0 = pthread_create(&id_usart,NULL,(void*)pthread_usart_receive,NULL);
	if(ret0 < 0)
		{	
			printf("id_usart create fault\n");
			return -1;
		}
	ret1 = pthread_create(&id_client,NULL,(void*)pthread_client_receive,NULL);
	if(ret1 < 0)
		{
			printf("id_client create fault\n");
			return -1;
		}
	ret3 = pthread_create(&id_gateway_heart,NULL,(void*)gateway_send_heart_jump,NULL);
	if(ret3 < 0)
		{
			printf("id_gateway_heart create fault\n");
			return -1;
		}
	while(1)
	{
		if(client_init("192.168.77.1")==0)
			break;
		else
		{
			close(cd);
			sleep(1);
		}
	}
	read_channel();
	ret2 = pthread_create(&id_heart_jump,NULL,(void*)heart_jump,NULL);
	if(ret2 < 0)
		{
			printf("id_heart_jump create fault\n");
			return -1;
		}
	pthread_join(id_usart,NULL);
	pthread_join(id_client,NULL);
	pthread_join(id_heart_jump,NULL);
	close(fd);
	close(cd);
	return 0;
}


