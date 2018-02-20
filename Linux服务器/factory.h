#ifndef __FACTORY_H__
#define __FACTORY_H__
#include "head.h"
//node 
typedef struct node
{
	int newfd;
	struct node* next;
}Node,*pNode;
//queue
typedef struct
{
	Node *head;
	Node *tail;
	int capacity;
	int size;
	pthread_mutex_t mutex;
}Queue,*pQueue;
//factory
typedef void* (*pthFunc)(void *);
typedef struct
{
	Queue queue;
	pthread_cond_t cond;
	pthread_t *pthid;
	int isActive;
	pthFunc thfunc;
	int threadNum;
}Factory,*pFactory;
/*工厂框架*/
void InitFac(pFactory, int, int, pthFunc);
void StartFac(pFactory);
void* thfunc(void *);
void EnQueue(pQueue,Node *);
void DeQueue(pQueue,pNode *);
/*文件发送与接收*/
int SendFile(int,char*,char*,long int);
int LoopSend(int,char *,int);
int RecvFile(int,char*);
int LoopRecv(int,char *,int);
int SendCmd(int,char *);
/*数据库操作*/
char* SearchUser(char* tablename,char* username);
int InsertIntoDB(char* tablename,char* username,char* ciphertext);

#endif
