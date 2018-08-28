#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "ipc.h"

int sem_create(const char *pathname, int proj_id, int semflg, key_t *key, int *semid)
{
	int ret;
	union semun semun;

	*key = ftok(pathname, proj_id);
	if (*key == -1) {
		perror("semaphore");
		return -1;
	}

	*semid = semget(*key, 1, semflg);
	if (*semid == -1) {
		perror("semaphore");
		return -1;
	}

	semun.val = 1;
	ret = semctl(*semid, 0, SETVAL, semun);
	if (ret == -1) {
		perror("semaphore");
		ret = semctl(*semid, 0, IPC_RMID, semun);
		if (ret) {
			perror("semaphore");
			return -1;
		}
		return -1;
	}

	return 0;
}

int sem_down(int semid)
{
	struct sembuf buf;

	buf.sem_num = 0;
	buf.sem_op = -1;
	buf.sem_flg = SEM_UNDO;

	if(semop(semid, &buf, 1) == -1) {
		perror("semaphore");
		return -1;
	}

	return 0;
}

int sem_up(int semid)
{
	struct sembuf buf;

	buf.sem_num = 0;
	buf.sem_op = 1;
	buf.sem_flg = SEM_UNDO;

	if(semop(semid, &buf, 1) == -1) {
		perror("semaphore");
		return -1;
	}

	return 0;
}

int sem_destroy(int semid)
{
	int ret;
	union semun semun;

	ret = semctl(semid, 0, IPC_RMID, semun);
	if (ret) {
		perror("semaphore");
		return -1;
	}

	return 0;
}

void *shm_create(const char *pathname, int proj_id, int shmflg, size_t size, key_t *key, int *shmid)
{
	int ret;
	void *shm;

	*key = ftok(pathname, proj_id);
	if (*key == -1) {
		perror("shared memory");
		return NULL;
	}

	*shmid = shmget(*key, size, shmflg);
	if (*shmid == -1) {
		perror("shared memory");
		return NULL;
	}

	shm = shmat(*shmid, NULL, 0);
	if (shm == (void *)-1) {
		perror("shared memory");
		ret = shmctl(*shmid, IPC_RMID, NULL);
		if (ret == -1) {
			perror("shared memory");
			return NULL;
		}
		return NULL;
	}

	return shm;
}

int shm_destroy(int shmid, void *shm)
{
	int ret;

	ret = shmdt(shm);
	if (ret == -1) {
		perror("shared memory");
		return -1;
	}

	ret = shmctl(shmid, IPC_RMID, NULL);
	if (ret == -1) {
		perror("shared memory");
		return -1;
	}

	return 0;
}

int msg_create(const char *pathname, int proj_id, int msgflg, key_t *key, int *msgid)
{
	*key = ftok(pathname, proj_id);
	if (*key == -1) {
		perror("message queue");
		return -1;
	}

	*msgid = msgget(*key, msgflg);
	if (*msgid == -1) {
		perror("message queue");
		return -1;
	}

	return 0;
}

int msg_destroy(int msgid)
{
	int ret;

	ret = msgctl(msgid, IPC_RMID, NULL);
	if (ret) {
		perror("message queue");
		return -1;
	}

	return 0;
}
