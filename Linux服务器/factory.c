#include "factory.h"
#include "comand.h"

/*线程函数*/
void* thfunc(void *p)
{	
	
	pFactory fac=(pFactory)p;
	pQueue queue=&fac->queue;
	pNode pcur;
	//准备用于用户级根目录
	char userWorkPath[MAX_PATH_LEN]={0};
	getcwd(userWorkPath,MAX_PATH_LEN);
	while(1)
	{
		//操作队列时加锁
		pthread_mutex_lock(&queue->mutex);
		if(0==queue->size)
		{
			pthread_cond_wait(&fac->cond,&queue->mutex);
		}
		DeQueue(queue,&pcur);
		pthread_mutex_unlock(&queue->mutex);
		
		//接收客户端命令并处理
		time_t start;
		time_t end;
		start=time(NULL);
		while(1)
		{
			if(Cmd(pcur->newfd,userWorkPath)==1)
				break;
		}
		free(pcur);
		printf("一个客户端已退出\n");
		end=time(NULL);
		//存入数据库
	}
}
/*接收客户端命令*/
int Cmd(int newfd,char *userWorkPath)
{
	CMDTomas cmdTomas;
	bzero(&cmdTomas,sizeof(CMDTomas));
	//接收命令类型和长度
	LoopRecv(newfd,(char*)&cmdTomas,sizeof(CMDTomas));
	//处理命令
	return HandleCmd(cmdTomas.buf,newfd,userWorkPath);
}
/*init factory*/
void InitFac(pFactory pfac,int threadNum,int capacity,pthFunc thfunc)
{
	//factory's member :queue
	pQueue q=&pfac->queue;
	q->head=NULL;
	q->tail=NULL;
	q->size=0;
	q->capacity=capacity;
	pthread_mutex_init(&q->mutex,NULL);
	//other factory's member
	pthread_cond_init(&pfac->cond,NULL);
	pfac->pthid=(pthread_t*)calloc(threadNum,sizeof(pthread_t));
	pfac->isActive=0;
	pfac->thfunc=thfunc;
	pfac->threadNum=threadNum;
}

/*start factory*/
void StartFac(pFactory fac)
{
	if(0==fac->isActive)
	{
		for(int i=0;i<fac->threadNum;i++)
		{
			pthread_create(fac->pthid+i,NULL,fac->thfunc,fac);
			printf("thread %d create sucessful.\n",i);
		}
		fac->isActive=1;
	}
	else
	{
		printf("factory is working!\n");
	}

}

/*enqueue*/
void EnQueue(pQueue queue,Node *newNode)
{
	pthread_mutex_lock(&queue->mutex);
	if(NULL==queue->head)
	{
		queue->head=newNode;
		queue->tail=newNode;	
	}
	else
	{
		queue->tail->next=newNode;
		queue->tail=newNode;
	}
	queue->size++;
	pthread_mutex_unlock(&queue->mutex);
}

/*dequeue*/
void DeQueue(pQueue queue,pNode *theNode)
{
	*theNode=queue->head;
	queue->head=queue->head->next;
	if(NULL==queue->head)
	{
		queue->tail=NULL;
	}
	queue->size--;
}
