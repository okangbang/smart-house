#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <arpa/inet.h>

#include "ipc.h"
#include "cam.h"
#include "web.h"
#include "zgb.h"

#define	DEBUG

#define	MAX_BACKLOG	32
#define	MAX_BUF_LEN	4096

void http_process(int connfd);

//WEB服务器初始化
int task_web(void)
{
    int ret;
    int listenfd;
    int opt = 1;
    int connfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    socklen_t addrlen;

    //	if (argc != 2) {
    //		printf("Usage: %s port\n", argv[1]);
    //		exit(EXIT_FAILURE);
    //	}

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("server->socket");
        exit(EXIT_FAILURE);
    }
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    //	port = atoi(argv[1]);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9999);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
    if (ret == -1) {
        perror("server->bind");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    ret = listen(listenfd, MAX_BACKLOG);
    if (ret == -1) {
        perror("server->listen");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        addrlen = sizeof(struct sockaddr_in);
        memset(&cliaddr, 0, sizeof(struct sockaddr_in));
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &addrlen);
        if (connfd == -1) {
            perror("server->accept");
            continue;
        }
        printf("server->accept: a new client is comming, and the connfd is %d\n", connfd);
        #ifdef DEBUG
        printf("server->accept: a new client is comming, and the connfd is %d\n", connfd);
        #endif
        http_process(connfd);//接受请求，返回请求数据
        close(connfd);
        //sleep(1);
    }

    close(listenfd);
    exit(EXIT_SUCCESS);
    }

void http_process(int connfd)
{
    int ret;
    char request[MAX_BUF_LEN];
    char response[MAX_BUF_LEN];

    memset(request, 0, sizeof(request));
    ret = read(connfd, request, MAX_BUF_LEN);
    if (ret <= 0) {
        if (ret == 0)
        printf("server->read: end-of-file\n");
        else
        perror("server->read");
        return;
    }
    #ifdef DEBUG
    printf("server->read: the request is\n%s", request);
    #endif
    //请求主页面文件
    if (strstr(request, "GET /index.html") != NULL) {
        printf("server->http: get html\n");
        int ret;
        FILE *fp;
        struct stat filestat;
        int filesize;
        char status[] = "HTTP/1.0 200 OK\r\n";
        char response_header[] = "Server: fsc100\r\nContent-Type: text/html\r\n\r\n";
        char response_text[4096];

        #ifdef DEBUG
        printf("server->http: get html\n");
        #endif
        fp = fopen("index.html", "rb");
        if (fp == NULL) {
            perror("server->fopen_index.html");
            return;
        }

        ret = stat("index.html", &filestat);
        if (ret == -1) {
            perror("server->stat");
            return;
        }

        filesize = filestat.st_size;
        memset(response_text, 0, sizeof(response_text));
        if (fread(response_text, filesize, 1, fp) != 1) {
            fprintf(stderr, "server->fread: fread failure\n");
            fclose(fp);
            return;
        }

        memset(response, 0, sizeof(response));
        strcat(response, status);
        strcat(response, response_header);
        strcat(response, response_text);
        #ifdef DEBUG
        printf("server->http: the response is\n%s\n", response);
        #endif
        ret = write(connfd, response, strlen(response));
        if (ret == -1) {
            perror("server->write");
            fclose(fp);
            return;
        }
        printf("end\n");
        fclose(fp);
    }
//图片请求
if (strstr(request, "GET /?action=snapshot") != NULL) {
    unsigned int count;
    char status[] = "HTTP/1.0 200 OK\r\n";
    char response_header[] = "Server: fsc100\r\nContent-Type: image/jpeg\r\nContent-Length: ";
    char length[32];

    #ifdef DEBUG
    printf("server->http: get image\n");
    #endif
    sem_down(semid);
    memset(response, 0, sizeof(response));
    strcat(response, status);
    strcat(response, response_header);
    snprintf(length, sizeof(length), "%d", jpg->jpg_size);
    strcat(response, length);
    strcat(response, "\r\n\r\n");
    #ifdef DEBUG
    printf("server->http: the response is\n%s\n", response);
    #endif
    ret = write(connfd, response, strlen(response));
    if (ret == -1) {
        perror("server->write");
        return;           
    }

    count = 0;
    while (count < jpg->jpg_size) {
        ret = write(connfd, jpg->jpg_buf + count, jpg->jpg_size - count);
        if (ret == -1) {
            perror("server->write");
            return;                
        }

        count += ret;

    }
    #ifdef DEBUG
    printf("server->write: write %d bytes success\n", jpg->jpg_size);
    #endif
    sem_up(semid);
}

    //子页面
    else if (strstr(request, "GET /env.html") != NULL) {
        int ret;
        char tmp[128];
        char status[] = "HTTP/1.0 200 OK\r\n";
        char response_header[] = "Server: fsc100\r\nContent-Type: text/html\r\n\r\n";
        char response_text[4096] = {0};

        #ifdef DEBUG
        printf("http->request: %s\n",request);
        #endif
        strcat(response_text, "<html>\r\n<body>\r\n<head>\r\n<meta http-equiv=\"refresh\" content=\"1\">\r\n</head>\r\n");
        snprintf(tmp, sizeof(tmp), "temp:%d hum:%d light:%d",(int)zgb->tem, (int)zgb->hum, (int)zgb->light);
        strcat(response_text, tmp);
        snprintf(tmp, sizeof(tmp), "</html>\r\n</body>\r\n");
        //send
        memset(response, 0, sizeof(response));
        strcat(response, status);
        strcat(response, response_header);
        strcat(response, response_text);
        #ifdef DEBUG
        printf("server->http: the response is\n%s\n", response);
        #endif
        ret = write(connfd, response, strlen(response));
        if (ret == -1) {
            perror("server->write");
            return;
        }
    }

    else if(strstr(request, "POST /index.html?cmd") != NULL) {
        int ret;
        char status[] = "HTTP/1.0 200 OK\r\n";
        char response_header[] = "Server: fsc100\r\nContent-Type: text/html\r\n\r\n";
        char response_text[32] = "cmd is OK ";
        struct com_msg msg;
        if (strstr(request, "led=on") != NULL) {
            /* you can turn on led here */

#ifdef DEBUG
			printf("server->request: send command %s\n", request);
#endif
			msg.type = 1;
			msg.data[0] = 0;
			ret = msgsnd(msgid, &msg, sizeof(struct com_msg) - sizeof(long), 0);
			if (ret == -1) 
            {
				perror("server->msgsnd");
			}
            printf("server->http: led on\n");
            strcat(response_text, "on");
        }

        else if(strstr(request, "led=off") != NULL)
        {
			msg.type = 1;
            msg.data[0] = 1;
			ret = msgsnd(msgid, &msg, sizeof(struct com_msg) - sizeof(long), 0);
			if (ret == -1)
            {
			perror("server->msgsnd");
            }
            printf("server->http: led off\n");
            strcat(response_text, "off");
        }
        
        else if (strstr(request, "fan=on") != NULL)
        {
			msg.type = 1;
            msg.data[0] = 2;
			ret = msgsnd(msgid, &msg, sizeof(struct com_msg) - sizeof(long), 0);
			if (ret == -1)
            {
			perror("server->msgsnd");
            }
            printf("server->http: fan on\n");
            strcat(response_text, "on");
        }

        else if (strstr(request, "fan=off") != NULL)
        {
            /* you can turn off fan here */
			msg.type = 1;
            msg.data[0] = 3;
			ret = msgsnd(msgid, &msg, sizeof(struct com_msg) - sizeof(long), 0);
			if (ret == -1)
            {
			perror("server->msgsnd");
            }
            printf("server->http: fan off\n");
            strcat(response_text, "off");
        }

        else if (strstr(request, "pwm=on") != NULL)
        {
            /* you can turn on led here */
			msg.type = 1;
            msg.data[0] = 4;
			ret = msgsnd(msgid, &msg, sizeof(struct com_msg) - sizeof(long), 0);
			if (ret == -1)
            {
			perror("server->msgsnd");
            }
            printf("server->http: pwm on\n");
            strcat(response_text, "on");
        }

        else if (strstr(request, "pwm=off") != NULL)
        {
            /* you can turn off pwm here */
			msg.type = 1;
            msg.data[0] = 8;
			ret = msgsnd(msgid, &msg, sizeof(struct com_msg) - sizeof(long), 0);
			if (ret == -1)
            {
			perror("server->msgsnd");
            }
            printf("server->http: pwm off\n");
            strcat(response_text, "off");
        }

        memset(response, 0, sizeof(response));
        strcat(response, status);
        strcat(response, response_header);
        strcat(response, response_text);
        #ifdef DEBUG
        printf("server->http: the response is\n%s\n", response);
        #endif
        ret = write(connfd, response, strlen(response));
        if (ret == -1) {
            perror("server->write");
            return;
        }
    }
}
