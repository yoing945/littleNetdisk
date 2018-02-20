#include "comand.h"
#include "factory.h"
#define JudgeArgc(argc,input) {if(argc!=input){printf("format:threadNum capacity\n");return -1;};}


int main(int argc,char *argv[])
{
		//读取配置文件信息
		char** figInfo;
		figInfo=(char**)calloc(2,sizeof(char*));
		figInfo[0]=(char*)calloc(128,sizeof(char));
		figInfo[1]=(char*)calloc(128,sizeof(char));
		FILE* fileD=fopen("figure","r");
		fgets(figInfo[0],128,fileD);
		fgets(figInfo[1],128,fileD);
		if (figInfo[0][strlen(figInfo[0]) - 1] == '\n')
				figInfo[0][strlen(figInfo[0]) - 1] = 0;
		if (figInfo[1][strlen(figInfo[1]) - 1] == '\n')
				figInfo[1][strlen(figInfo[1]) - 1] = 0;
		fclose(fileD);
		//初始化线程池工厂
		int threadNum=10;
		int capacity=10;
		Factory fac;
		InitFac(&fac,threadNum,capacity,thfunc);
		//启动工厂
		StartFac(&fac);
		//socket
		int sfd=socket(AF_INET,SOCK_STREAM,0);
		if(sfd==-1)
		{
				perror("socket");
				return -1;
		}
		//设置IP和接口重用
		int reuse=1;
		setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
		//bind
		struct sockaddr_in ser;
		ser.sin_family=AF_INET;
		ser.sin_port=htons(atoi(figInfo[1]));
		ser.sin_addr.s_addr=inet_addr(figInfo[0]);
		int ret=bind(sfd,(struct sockaddr*)&ser,sizeof(struct sockaddr));
		if(ret==-1)
		{
				perror("bind");
				close(sfd);
				return -1;
		}
		//listen
		listen(sfd,capacity);

		int newfd;
		int addrlen=sizeof(struct sockaddr);
		struct sockaddr_in client;
		pQueue queue=&fac.queue;
		pNode newNode;
		while(1)
		{
				newfd=accept(sfd,(struct sockaddr*)&client,&addrlen);
				printf("%s %d success connect\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
				newNode=(Node*)calloc(1,sizeof(Node));
				newNode->newfd=newfd;
				EnQueue(queue,newNode);
				//printf("Enqueue successful!\n");
				pthread_cond_signal(&fac.cond);
		}
		free(figInfo);
		return 0;
}
