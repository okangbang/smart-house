#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "zgb.h"
#include "ipc.h"
#include "cam.h"
#include "srv.h"
/*
#define DEBUG
*/
#define	MAX_BACKLOG	32
#define	REQ_BUF_LEN	32
#define	HDR_BUF_LEN	20

extern struct jpg_buf_t *jpg;
extern struct zgb_buf_t *zgb;

//僵尸进程处理函数
static void sigchld_handler(int signo)
{
	pid_t pid;
	int status;

	pid=wait(&status);
	printf("process %d exited\n", pid);
}

int go_login(int connfd) 
{
	int ret;
	char response[HDR_BUF_LEN];

	strcpy(response, "OK");
	ret = write(connfd, response, sizeof(response));
	if (ret == -1) {
		perror("server");
		return -1;
	}

	return 0;
}


int send_pic(int connfd) 
{
	int ret;
	int imgsize;
	int count;
	char *imgbuf;
	char response[HDR_BUF_LEN];

	memset(response, 0, sizeof(response));
	sem_down(semid);
	imgsize = jpg->jpg_size;
	imgbuf = malloc(imgsize);
	if (NULL == imgbuf) {
		perror("server->sendpic->malloc");
		return -1;
	}
	memcpy(imgbuf, jpg->jpg_buf, imgsize);
	sem_up(semid);

	sprintf(response, "%dlen", imgsize);
	printf("response=%s\n",response);
	ret = write(connfd, response, sizeof(response));
	if (ret == -1) {
		perror("server");
		return -1;
	}

	count = 0;
	while (count < imgsize) {
		ret = write(connfd, imgbuf + count, imgsize - count);
		if (ret == -1) {
			perror("server->write");
			close(connfd);
			exit(EXIT_FAILURE);
		}

		count += ret;
	}



	return 0;
}



int send_env(int connfd) 
{
	int ret,light,hum,temp;			
	char response[20];
	memset(response, 0, sizeof(response));
	sem_down(semid);
	light =zgb -> light;
	hum = zgb ->hum;
	temp = zgb->tem;
	sem_up(semid);
	sprintf(response, "env%dhum", temp);	
	ret = write(connfd, response, sizeof(response));
	if (ret == -1) {
		perror("server");
		return -1;
	}

	return 0;
}
//服务器端连接上后运行的函数
void client_process(int connfd)
{
	int ret;
	char request[REQ_BUF_LEN];

	while (1) {
        //将收指令的数组清0
		memset(request, 0, sizeof(request));
        //准备接受指令，并检查位数，如果小于等于 0，则 报错或者未收到数据
		ret = read(connfd, request, REQ_BUF_LEN);
		if (ret <= 0) {
			if (ret == 0) {
				printf("server->read: end-of-file\n");
				close(connfd);
				exit(EXIT_SUCCESS);
			} else {
				perror("server->read");
				close(connfd);
				exit(EXIT_FAILURE);
			}
		}


		printf("%s\n", request);
		if (strstr(request, "d#")) {
			ret = go_login(connfd);
			if (ret == -1) {
				printf("login fail\n");
			}
		} else if (strstr(request, "get_img")) {
			ret = send_pic(connfd);
			if (ret == -1) {
				printf("send pic fail\n");
			}
		}else if(strstr(request, "get_env") != NULL) 
		{
			ret=send_env(connfd);   
			if (ret == -1) {
				printf("send env fail\n");
			}

		}
		else {
			printf("%s\n", request);
		}
	}
	close(connfd);
	exit(EXIT_SUCCESS);
}

//服务器与Android、QT客户端连接的函数
void task_srv(void)
{
	int ret;
	int listenfd;
	int opt = 1;
	int connfd;
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	socklen_t addrlen;
	unsigned short port;

    //创建socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1) {
		perror("server->socket");
		exit(EXIT_FAILURE);
	}
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //设置socket结构体，为后面绑定做准备
	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	port = 8888;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

   //bind 绑定socket
	ret = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
	if (ret == -1) {
		perror("server->bind");
		close(listenfd);
		exit(EXIT_FAILURE);
	}

    //进入listen监听队列
	ret = listen(listenfd, MAX_BACKLOG);
	if (ret == -1) {
		perror("server->listen");
		close(listenfd);
		exit(EXIT_FAILURE);
	}

    //创建执行僵尸信号处理函数
	signal(SIGCHLD, sigchld_handler);

    //服务器初始化
	/* srv_init(); */
	while(1) {
		pid_t pid;

        //accept等待客户端连接
		addrlen = sizeof(struct sockaddr_in);
		memset(&cliaddr, 0, sizeof(struct sockaddr_in));
		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &addrlen);
		if (connfd == -1) {
			perror("server->accept");
			continue;
		}
#ifdef DEBUG
		printf("server->accept: a new client is comming, and the connfd is %d\n", connfd);
#endif
        //创建子进程
		pid = fork();
		if (pid == -1) {
			perror("server->fork");
			close(connfd);
			close(listenfd);
			exit(EXIT_FAILURE);
		} else if (pid == 0) {
			close(listenfd);
			client_process(connfd);
		} else {
#ifdef DEBUG
			printf("server->fork: fork a new client process success, and the pid = %d\n", pid);
#endif
			close(connfd);
		}

	};

    
    //进程退出
	printf("android server process exited\n");
	exit(EXIT_SUCCESS);
}
