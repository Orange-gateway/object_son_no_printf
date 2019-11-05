#include "include.h"

/*  tcp初始化函数   */
int client_init(char *connect_ip)
{
	int ret = 1;
	struct sockaddr_in ser_addr;
	
	memset((void*)&ser_addr,0,sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(8888);
	
	//ser_addr.sin_addr.s_addr  = INADDR_ANY;
	inet_aton(connect_ip,&ser_addr.sin_addr);//将点分式ip地址 转换为2进制 
	cd = socket(AF_INET,SOCK_STREAM,0);
	if(cd == -1)
		return -1;
	ret = connect(cd,(struct sockaddr*)&ser_addr,sizeof(ser_addr));
	if( ret == 0)
	{
		printf("connect success  cd = %d\n",cd);
		NET_FLAG = 1;
		char send_father_mac[21] = {0x5a,0xa5,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf4,0x00,0x00,0x00,0x06};
		memcpy(send_father_mac+15,my_gw_mac,6);
		send(cd,send_father_mac,21,0);
		struct timeval timeout = {0,100000}; 
		setsockopt(cd,SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(struct timeval));
		return 0;
	}
	else
	{
		printf("connet fault  cd = %d\n",cd);
		NET_FLAG = 0;
		return -1;
	}
}


