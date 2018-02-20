#include "factory.h"

int SendCmd(int sfd,char *input)
{
	CMDTomas ct;
	bzero(&ct,sizeof(CMDTomas));
	ct.flagFile=0;
	ct.len=strlen(input);
	strcpy(ct.buf,input);
	return LoopSend(sfd,(char*)&ct,sizeof(CMDTomas));
}
/*发送文件*/
int SendFile(int newfd,char *objPathReal,char *filename,long int size)
{
	//用货运托马斯发货！
	Tomas tomas;
	bzero(&tomas,sizeof(Tomas));
	//发送文件名和文件大小
	strcpy(tomas.buf,filename);
	tomas.len=strlen(tomas.buf)+sizeof(long int);
	strcpy(tomas.buf+strlen(tomas.buf),(char*)&size);
	int ret=0;
	ret=LoopSend(newfd,(char*)&tomas,sizeof(int)+tomas.len);
	if(ret==-1)
	{
		return -1;
	}
	int fd;
	fd=open(objPathReal,O_RDONLY);
	if(fd==-1)
	{
		perror("open");
		return -1;
	}
	while(bzero(&tomas,sizeof(Tomas)),(tomas.len=read(fd,tomas.buf,sizeof(tomas.buf)))>0)
	{
		ret=LoopSend(newfd,(char*)&tomas,tomas.len+4);
		if(ret==-1)
			goto end;
	}
	//send len '0' represent send over
	tomas.len=0;
	LoopSend(newfd,(char*)&tomas,4);
end:
	close(fd);
}
/*loop send*/
int LoopSend(int newfd,char *buf,int len)
{
	int total=0;
	int ret=0;
	while(total<len)
	{
		ret=send(newfd,buf,len-total,0);
		if(ret==-1)
		{
			perror("send");
			printf("errno:%d\n",errno);
			return -1;
		}
		total+=ret;
	}
}
/*receive file*/
int RecvFile(int sfd,char *objPathReal)
{
	long int size,sum;
	Tomas tomas;
	bzero(&tomas,sizeof(Tomas));
	//拿到文件名和大小
	LoopRecv(sfd,(char*)&tomas,sizeof(int));
	if(tomas.len==0)
	{
		return -1;
	}
	LoopRecv(sfd,tomas.buf,tomas.len);
	char filename[128]={0};
	for(int i=0;i<tomas.len-sizeof(long int);i++)
		filename[i]=tomas.buf[i];
	strcpy((char*)&size,tomas.buf+strlen(filename));
	printf("file name:%s\n",filename);
	int fd=open(objPathReal,O_CREAT|O_RDWR,0666);
	if(fd==-1)
	{
		perror("open");
		close(sfd);
		return -1;
	}
	//receive file
	while(1)
	{
		bzero(&tomas,sizeof(Tomas));
		LoopRecv(sfd,(char*)&tomas,sizeof(int));
		if(tomas.len>0)
		{
			LoopRecv(sfd,tomas.buf,tomas.len);
			write(fd,tomas.buf,tomas.len);
		}
		else
			break;
	}
	printf("Recieve over\n");
	close(fd);
}
/*loop receive*/
int LoopRecv(int sfd,char *buf,int len)
{
	int total=0;
	int ret=0;
	while(total<len)
	{
	     ret=recv(sfd,buf+total,len-total,0);
	     total+=ret;
	}
}


