#ifndef _IPC_H
#define _IPC_H

#define DAT_SEM_KEY "."
#define DAT_SEM_FLG 's'

#define DAT_SHM_KEY "."
#define DAT_SHM_FLG 'm'

#define COM_MSG_KEY "."
#define COM_MSG_FLG 'c'

extern key_t ksem;
extern key_t kshm;
extern key_t kmsg;

extern int semid;
extern int shmid;
extern int msgid;

extern void *shm;
extern struct jpg_buf_t *jpg;
extern struct zgb_buf_t *zgb;
extern int *sync_flag;

union semun{
	int val;
	struct semid_ds *buf;
	unsigned short *arry;
	struct seminfo *_buf;

};
struct com_msg{
	long type;
	unsigned char data[252];
};

int sem_create(const char *pathname,int proj_id,int semflg,key_t *key,int *semid);
int sem_down(int semid);
int sem_up(int semid);
int sem_destroy(int semid);
void *shm_create(const char *pathname,int proj_id,int shmflag,size_t size,key_t *key,int *shmid);
int shm_destroy(int shmid,void *shm);
int msg_create(const char *pathname,int proj_id,int msgflag,key_t *key,int *shmid);
int destroy(int msgid);

#endif
