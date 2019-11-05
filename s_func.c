#include "include.h"

int reboot_restart()
{
	sync(); // 同步磁盘数据,将缓存数据回写到硬盘,以防数据丢失
    return reboot(RB_AUTOBOOT);
}
void kill_gateway(void)
{
	close(fd);
	shutdown(cd,SHUT_RDWR);
	close(cd);

	system("killall gateway");

}
void delete_len_from_arr(uint8_t *arr,int n,int *len_of_arr)///操作的数组，删除的长度，本来的长度
{
    int i;
    
    for(i=n; i<(*len_of_arr); i++)
    {
        arr[i-n] =arr[i];
    }
    
    *len_of_arr -=n;
}


void delete_len_from_str(char *str,int n,int *len_of_str)///操作的字符串，删除的长度，本来的长度
{
    int i;
    
    for(i=n; i<(*len_of_str); i++)
    {
        str[i-n] =str[i];
    }
    
    *len_of_str -=n;
}

// 将中文字符替换为'*' 用于json字符串合法性检查
int replace_character(char *string)
{
    if (string == NULL)
    {
        printf("it is NULL\n");
        return -1;
    }
    while(*string++ != '\0')
    {
        if (((*string) < 0x00) || ((*string) > 0x7F))
        {
            *string = '*';
        }
    }

    return 0;
}

void int_to_str(int num,char *str)        
{
    int i,j;
    char string[50];
    memset(string,0,sizeof(string));
    if( num < 0 )
        num=-num;
    i=0;
    do
    {
        string[i++] = num%10+'0';
    }
    while ( (num/=10) >0 );

    for(j=0; j<i; j++) 
    {
        str[j] = string[i-j-1];
    }
    str[i] = '\0';
}
/*******************整形转化为字符*****************/
char int_to_char(int i)
{
	if((-1<i)&&(i<10))
		return i+48;
	else if((i>9)&&(i<16))
		return  i+87;
	else
		return '*';
}
/**************字符转化为整形**************/
int char_to_int(char b)
{
	if((47<b)&&(b<58))
		return b-48;
	else if((64<b)&&(b<71))
		return b-55;
	else if((96<b)&&(b<103))
		return b-87;
	else
		return -1;
}

/*16进制转字符串:参数：16进制，字符串，16进制长度*/
void hex_to_str(uint8_t *str_unchar,char *str_char,int len_of_hex)
{
    int i,H1=0,H2=0;
    for(i=0; i<len_of_hex; i++)
    {
        H1 = str_unchar[i]/16;
        H2 = str_unchar[i]%16;
        if((H1>=0)&&(H1<=9))
        {
            str_char[i+i] = (char)(48 + H1);
        }
        else if((H1>=10)&&(H1<=15))
        {
            str_char[i+i] = (char)(87 + H1);
        }
        if((H2>=0)&&(H2<=9))
        {
            str_char[i+i+1] = (char)(48 + H2);
        }
        else if((H2>=10)&&(H2<=15))
        {
            str_char[i+i+1] = (char)(87 + H2);
        }
    }
    	//str_char[len_of_hex*2]='\0';
}

/*字符串转16进制：参数：16进制数组，字符串，16进制数组长度*/
void str_to_hex(uint8_t *str_unchar,char *str_char,int len_of_hex)
{
    int i,i1=0,i2=0;
    for(i=0; i<len_of_hex; i++)
    {
        if(str_char[2*i]>='a'&&str_char[2*i]<='f')
        {
            i1 = (str_char[2*i]-'a'+10)*16;
        }
        else if(str_char[2*i]>='A'&&str_char[2*i]<='F')
        {
            i1 = (str_char[2*i]-'A'+10)*16;
        }
        else if(str_char[2*i]>='0'&&str_char[2*i]<='9')
        {
            i1 = (str_char[2*i] - '0')*16;
        }

        if(str_char[2*i+1]>='a'&&str_char[2*i+1]<='f')
        {
            i2 = str_char[2*i+1]-'a'+10;
        }
        else if(str_char[2*i+1]>='A'&&str_char[2*i+1]<='F')
        {
            i2 = str_char[2*i+1]-'A'+10;
        }
        else if(str_char[2*i+1]>='0'&&str_char[2*i+1]<='9')
        {
            i2 = str_char[2*i+1] - '0';
        }
        
        str_unchar[i] = i1+i2;
    
    }
  
}

/************线程创建**************/
pthread_t pth_creat_my(void *func,void *s_data)
{

 pthread_t pid;
 pthread_attr_t attr;
///设置线程的分离属性
 int res = pthread_attr_init(&attr);        //初始化
 if(res!=0)
    printf("attr init fault\n");
	
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);//all
	
 res = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
 
 if(res!=0)
    printf("set_detachstate fault\n");

 if(pthread_create(&pid,NULL,func,s_data)!=0)
    {
        printf("create pthread  fault\n");
    }

    return pid;
}
void get_hex_mac(uint8_t *data)
{
	struct ifreq req;
    int sock;
    sock = socket (AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket error:");
    }
    strcpy(req.ifr_name, "ath0");    ///Currently, only get ath0
    if(ioctl(sock, SIOCGIFHWADDR, &req) < 0)
    {
        perror("ioctl error:");
    }
    int i=0;
    for(i=0; i<6; i++)
    {
        data[i] = (unsigned char)req.ifr_hwaddr.sa_data[i];
 
    }
    close(sock);
}
void read_channel(void)
{
	uint8_t read_channel_cmd[16] = {0x5a,0xa5,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x01,0x00};
	usart_send(fd,read_channel_cmd,16);
}
