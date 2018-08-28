#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>


#include "web.h"
#include "ipc.h"
#include "cam.h"
#include "srv.h"

#define COM_SHM_SZ	((128 + 4) * 1024)
#define	ZGB_DAT_OFF	(128 * 1024)
#define	SYNC_FLAG_OFF	((128 + 4) * 1024 - sizeof(int))

key_t ksem = 0;
key_t kshm = 0;
key_t kmsg = 0;

int semid = -1;
int shmid = -1;
int msgid = -1;

void *shm = NULL;
struct jpg_buf_t *jpg = NULL;
struct zgb_buf_t *zgb = NULL;
int *sync_flag = NULL;

//定义摄像头、Zigbee、WEB服务器、总服务器模块pid
pid_t cam_pid;
pid_t zgb_pid;
pid_t web_pid;
pid_t srv_pid;

//僵尸进程处理模块
void sigchld_handler(int signo)
{
	pid_t pid;
	int status;

	pid = wait(&status);
	printf("process %d exited\n", pid);
}

//主函数模块
int main(int argc, char *argv[])
{
	int ret;

    //创建信号量
	ret = sem_create(DAT_SEM_KEY, DAT_SEM_FLG, IPC_CREAT | 0666, &ksem, &semid);
	if (ret) {
		printf("create semphore failed\n");
		exit(EXIT_FAILURE);
	}
	printf("create semphore success\n");
    
    //创建共享内存
	shm = shm_create(DAT_SHM_KEY, DAT_SHM_FLG, IPC_CREAT | 0666, COM_SHM_SZ, &kshm, &shmid);
	if (!shm) {
		printf("create shared memory failed\n");
		sem_destroy(semid);
		exit(EXIT_FAILURE);
	}
	printf("create shared memory success\n");

    //创建消息队列
	ret = msg_create(COM_MSG_KEY, COM_MSG_FLG, IPC_CREAT | 0666, &kmsg, &msgid);
	if (ret) {
		printf("create message queue failed\n");
		sem_destroy(semid);
		shm_destroy(shmid, shm);
		exit(EXIT_FAILURE);
	}
	printf("create message queue success\n");

	jpg = shm;
	zgb = shm + ZGB_DAT_OFF;
	sync_flag = shm + SYNC_FLAG_OFF;

    //创建摄像头模块
	cam_pid = fork();
	if (cam_pid == -1) {
		perror("fork camera process");
		goto out;
	} else if (cam_pid == 0) {
		task_cam();
	} else {
		printf("fork camera process sucess, and the pid = %d\n", cam_pid);
	}

    //创建Zigbee模块
	zgb_pid = fork();
	if (zgb_pid == -1) {
		perror("fork zigbee process");
		goto out;
	} else if (zgb_pid == 0) {
		task_zgb();
	} else {
		printf("fork zigbee process sucess, and the pid = %d\n", zgb_pid);
	}
//#if 0
    //WEB服务器模块
	web_pid = fork();
	if (web_pid == -1) {
		perror("fork web server process");
		goto out;
	} else if (web_pid == 0) {
		task_web();
	} else {
		printf("fork web server process sucess, and the pid = %d\n", web_pid);
	}
//#endif
    
    //Android、QT服务器模块
	srv_pid = fork();
	if (srv_pid == -1) {
		perror("fork android server process");
		goto out;
	} else if (srv_pid == 0) {
		task_srv();
	} else {
		printf("fork Android & QT server process sucess, and the pid = %d\n", srv_pid);
	}

    //创建信号SIGCHLD
	signal(SIGCHLD, sigchld_handler);

	while (1) {
		printf("main process\n");
		sleep(10);
	}

//错误返回函数，不同阶段报错返回值不同
out:
	if (semid != -1)
		sem_destroy(semid);
	if (shmid != -1)
		shm_destroy(shmid, shm);
	if (msgid != -1)
		msg_destroy(msgid);

	exit(EXIT_SUCCESS);
}
