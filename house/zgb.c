#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#include "zgb.h"

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

//控制命令ID
#define on_led1			0
#define off_led1		1
#define on_speaker		2
#define off_speaker		3
#define on_fan			4
#define on_fan_low		5
#define on_fan_mid		6
#define on_fan_high 	7
#define off_fan			8
#define on_seven_led    9
#define off_seven_led	10


typedef struct _command_t{
	uint8_t operate_id;	//define the object operated
	uint8_t operation;	//define the action object should do 
}command_t;

#define  start_machine_t 0xAA	 //开机
#define  data_flow_t 	 0xBB	 //数据采集
#define  rfid_msg_t      0xCC	 //rfid信息
#define  command_tag_t	 0xDD	 //命令
#define  key_msg_t 	 0xEE    //按键
#define  other_type_t	 0x00	 //其他（未定义）
typedef enum _message_tag_t{
	start_machine	= start_machine_t,
	data_flow	= data_flow_t,
	rfid_msg	= rfid_msg_t,
	command		= command_tag_t,
	key		= key_msg_t,
	other_type	= other_type_t
}message_tag_t;			//标记消息类型

typedef struct _tem_t{		//温度信息(2bytes)
	uint8_t ltem;
	uint8_t htem;
}tem_t;					

typedef struct _hum_t{		//湿度信息(2bytes)
	uint8_t lhum;
	uint8_t hhum;
}hum_t;

typedef struct _light_t{	//光照信息(4bytes)
	uint32_t light;
}light_t;

typedef struct _state_t	{ 	//设备状态(4bytes)
	uint8_t led_state;
	uint8_t fan_state;
	uint8_t buzz_state;
	uint8_t seven_led_state;
}state_t;

typedef struct _env_msg_t{	//环境信息
	tem_t tem;		//温度（2bytes）
	hum_t hum;		//湿度（2bytes）
	light_t light;		//光照值(4bytes)
	state_t state;		//设备状态(4bytes)
}env_msg_t;

typedef union _data_t {		//可选择发送的信息类型
	command_t command;
	env_msg_t env_msg;
}data_t;
extern struct zgb_buf_t *zgb; 
#if 1
typedef struct	{		//发送消息格式
	uint8_t tag;		//数据类型(1byte)
	uint8_t slave_address;	//从设备地址(1byte)
	uint8_t data_length;	//数据长度  (1byte)
 	data_t data;		//数据正文(23bytes)
	uint16_t crc;		//2bytes
}message_t;


#endif
int serial_Open(int comport)
{  
	/*分别为com1，com2， com3对应 ttyUSB0 ttyUSB1 ttyUSB2 */
	char *dev[] = {"/dev/ttyUSB0"};
	
	int fd;
    fd = open( dev[comport], O_RDWR|O_NOCTTY|O_NDELAY);  
    if (FALSE == fd)  
    {  
        perror("Can't Open Serial Port");  
        return(FALSE);
     }  
     //恢复串口为阻塞状态                                 
     if(fcntl(fd, F_SETFL, 0) < 0)  
     {  
        printf("fcntl failed!\n");  
        return(FALSE);  
     }       
     else  
     {  
        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
     }  
      //测试是否为终端设备      
    if(0 == isatty(fd))  
    {  
        printf("standard input is not a terminal device\n");  
        return(FALSE);
	}  
	else
    {  
        printf("isatty success!\n");  
    }                
    printf("fd->open=%d\n",fd);  
    return fd;  
} 

void serial_Close(int fd)
{
    close(fd);
}

int serial_init(int comport)
{  
    int fd_comport = FALSE;
	//打开串口
	if((fd_comport=serial_Open(comport))<0)
	{
		perror("serial_Open");
		return FALSE;
	}
    //设置串口数据帧格式  
    if (serial_Set(fd_comport,115200,0,8,1,'N')<0)  
    {   
		perror("serial_Set");
        return FALSE;  
    } 
    fprintf(stdout,"zgb init success\n");
    return fd_comport; 
} 

extern  int serial_Recv(int fd, void *p,int data_len, int timeout)
{  
	static struct pollfd fds;
	fds.fd    = fd;
	fds.events= POLLIN;
	fds.revents=0;

	/*60 second timeout*/
	int ret = poll(&fds, 1, timeout);
	if(0 >= ret)
	{
		perror("[recv_serial]poll");
		return -1;	/*error or timout*/
	}

	if(fds.revents & POLLIN)
		return read(fd, p, data_len);

	printf("[TRACE][recv_serial]poll unkonw err.\r\n");
	return -1;		/*unknow error*/
	return read(fd, p, data_len);  
} 

extern  int serial_Send(int fd, void *p,int data_len, int timeout)
{ 
	static struct pollfd fds;
	fds.fd    = fd;
	fds.events= POLLOUT;	
	fds.revents=0;	

	/*60 second timeout*/
	int ret = poll(&fds, 1, timeout);
	if(0 >= ret)
	{
		perror("[write_serial]poll");
		return -1;	/*error or timout*/
	}

	if(fds.revents & POLLOUT)
		return write(fd, p, data_len);

	return -1;		/*unknow error*/
	return write(fd, p, data_len);
} 
int serial_Set(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{  
     
     int   i;  
     int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};  
     int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};  
           
    struct termios options;  
     
    /*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1. 
    */  
    if  ( tcgetattr( fd,&options)  !=  0)  
    {  
         perror("SetupSerial 1");      
         return(FALSE);   
    }  
    
    //设置串口输入波特率和输出波特率  
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)  
    {  
         if  (speed == name_arr[i])  
         {               
             cfsetispeed(&options, speed_arr[i]);   
             cfsetospeed(&options, speed_arr[i]);    
         }  
   }       
     
    //修改控制模式，保证程序不会占用串口  
    options.c_cflag |= CLOCAL;  
    //修改控制模式，使得能够从串口中读取输入数据  
    options.c_cflag |= CREAD;  
    options.c_oflag &= ~(ONLCR | OCRNL);
	options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	options.c_iflag &= ~(ICRNL | INLCR);
	options.c_iflag &= ~(IXON | IXOFF | IXANY);
	
    //设置数据流控制  
    switch(flow_ctrl)  
    {  
        
       case 0 ://不使用流控制  
              options.c_cflag &= ~CRTSCTS;  
              break;     
        
       case 1 ://使用硬件流控制  
              options.c_cflag |= CRTSCTS;  
              break;  
       case 2 ://使用软件流控制  
              options.c_cflag |= IXON | IXOFF | IXANY;  
              break;  
    }  
    //设置数据位  
    //屏蔽其他标志位  
    options.c_cflag &= ~CSIZE;  
    switch (databits)  
    {    
       case 5    :  
                     options.c_cflag |= CS5;  
                     break;  
       case 6    :  
                     options.c_cflag |= CS6;  
                     break;  
       case 7    :      
                 options.c_cflag |= CS7;  
                 break;  
       case 8:      
                 options.c_cflag |= CS8;  
                 break;    
       default:     
                 fprintf(stderr,"Unsupported data size\n");  
                 return (FALSE);   
    }  
    //设置校验位  
    switch (parity)  
    {    
       case 'n':  
       case 'N': //无奇偶校验位。  
                 options.c_cflag &= ~PARENB;   
                 options.c_iflag &= ~INPCK;      
                 break;   
       case 'o':    
       case 'O'://设置为奇校验      
                 options.c_cflag |= (PARODD | PARENB);   
                 options.c_iflag |= INPCK;               
                 break;   
       case 'e':   
       case 'E'://设置为偶校验    
                 options.c_cflag |= PARENB;         
                 options.c_cflag &= ~PARODD;         
                 options.c_iflag |= INPCK;        
                 break;  
       case 's':  
       case 'S': //设置为空格   
                 options.c_cflag &= ~PARENB;  
                 options.c_cflag &= ~CSTOPB;  
                 break;   
        default:    
                 fprintf(stderr,"Unsupported parity\n");      
                 return (FALSE);   
    }   
    // 设置停止位   
    switch (stopbits)  
    {    
       case 1:     
                 options.c_cflag &= ~CSTOPB; 
				 break;   
       case 2:     
                 options.c_cflag |= CSTOPB;
				 break;  
       default:     
                       fprintf(stderr,"Unsupported stop bits\n");   
                       return (FALSE);  
    }  
     
  //修改输出模式，原始数据输出  
    options.c_oflag &= ~OPOST;  
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//我加的  
//options.c_lflag &= ~(ISIG | ICANON);  
     
    //设置等待时间和最小接收字符  
    options.c_cc[VTIME] = 1; /* 读取一个字符等待0*(0/10)s */    
    options.c_cc[VMIN] = 1; /* 读取字符的最少个数为0 */  
     
    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读  
    tcflush(fd,TCIFLUSH);  
     
    //激活配置 (将修改后的termios数据设置到串口中）  
    if (tcsetattr(fd,TCSANOW,&options) != 0)    
    {  
       perror("com set error!\n");    
       return (FALSE);   
    }  
	printf("serial set success\n");
    return (TRUE);   
} 

int task_zgb()
{
	int fd = serial_init(0);
	fprintf(stdout,"zgb init success!\n");
	if(0 > fd){
		return -1;
	}

	printf("-------init_serial done.-------\n");
	printf("-------msg = %d-------\n", sizeof(message_t));

	struct pollfd pfd[2] = {
		[0]= {
			.fd     = fd,
			.events = POLLIN, 
			.revents= 0,
		},
		[1]= {
			.fd     = 0,
			.events = POLLIN, 
			.revents= 0,
		},
	};

#define MAX 36
	uint8_t msgbuf[MAX];
	message_t *msg_r, *msg_s;
//	env *envsend=malloc(sizeof(* envsend));
	uint8_t devid = 0;
	

	while(1){
		int ret = poll(pfd, 2, -1);
		if(0 > ret){
			perror("poll");
			return -1;
		} else if(0 == ret) {
			printf("Timeout.\n");
			continue;
		} 

		if(POLLIN & pfd[0].revents){	
			memset(msgbuf, 0, MAX);
			int len, num;
			uint8_t *p = msgbuf;
			num = serial_Recv(fd, p, 1, 1000);
			if(0 > num){
				printf("recv fail.\n");
				continue;
			}
			if(data_flow_t != p[0]){
				printf("msg_r->tag = %.2x\n", p[0]);
				continue;
			}
			p++;
			len = MAX-1;
			while(len){
				num = serial_Recv(fd, p, len, -1);
				if(0 > num){
					printf("recv fail.\n");
					return -1;
				}
				//printf("recv num = %d, len = %d\n", num, len);

				len -= num;
				p   += num;
			}

	//		printf("recv:");
			//int i;
	//		for(i = 0; i < MAX; i++){
	//			printf("%.2X ", msgbuf[i]);
	//		}
	//		printf("\n");
			msg_r = (message_t *)msgbuf;
			if(data_flow_t != msg_r->tag){
				printf("msg_r->tag(%d) = %.2x\n", sizeof(msg_r->tag), msg_r->tag);
				continue;
			}
			
	//		printf("env data sample:\n");
			devid = msg_r->slave_address;
	/*		printf("\tdev id: %x\n", msg_r->slave_address);
			printf("\ttotal len: %ubytes\n", msg_r->data_length);
			printf("\ttem: low=%u, hig=%u\n", \
				msg_r->data.env_msg.tem.ltem, msg_r->data.env_msg.tem.htem);
			printf("\thum: low=%u, hig=%u\n", \
				msg_r->data.env_msg.hum.lhum, msg_r->data.env_msg.hum.hhum);
			
				
	
		printf("\tled_state=%x, fan_state=%x, buzz_state=%x, seven_led_state=%x\n", \
				msg_r->data.env_msg.state.led_state, \
				msg_r->data.env_msg.state.fan_state, \
				msg_r->data.env_msg.state.buzz_state, \
				msg_r->data.env_msg.state.seven_led_state);
		//	printf("\n\n");
	*/	}
			zgb->light=msg_r->data.env_msg.light.light;
			zgb->tem=msg_r->data.env_msg.tem.htem;
			zgb->hum=msg_r->data.env_msg.hum.hhum;

		if(POLLIN & pfd[1].revents){	
			memset(msgbuf, 0, MAX);
			msg_s = (message_t *)msgbuf;
			msg_s->tag = command_tag_t;
			msg_s->slave_address = devid;
			msg_s->data_length = MAX;

			printf("0: led on.\n");
			printf("1: led off.\n");
			printf("2: bepp on.\n");
			printf("3: bepp off.\n");
			printf("4: fan on.\n");
			printf("5: fan off.\n");
			printf("6: seven_led on.\n");
			printf("7: seven_led off.\n");
			
			char ch  = getchar();
			int i;
			ch -= '0';
			if(0 <= ch && ch <= 4)
			{
				printf("send:");
				msg_s->data.command.operate_id = ch;
				for(i = 0; i < MAX; i++){
					printf("%.2X ", msgbuf[i]);
				}
				printf("\n");
				int num = serial_Send(fd, msgbuf, MAX, -1);
				if(MAX != num){
					printf("send fail.\n");
					return -1;
				}
			}
			if(5 <= ch && ch <= 7)
			{
				printf("send:");
				msg_s->data.command.operate_id = (ch + 3);
				for(i = 0; i < MAX; i++){
					printf("%.2X ", msgbuf[i]);
				}
				printf("\n");
				int num = serial_Send(fd, msgbuf, MAX, -1);
				if(MAX != num){
					printf("send fail.\n");
					return -1;
				}
			}
	
			printf("\n");
		}
	}

	return 0;
}
