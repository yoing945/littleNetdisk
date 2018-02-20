#include "comand.h"
#include "factory.h"

/*	各个命令含义
	1 "pathname"	:cd "pathname" 
	2 "filename"	:puts "filename"
	3 "filename"	:gets "filename"
	4 "filename"	:remove "filename"
	5 "direntname"	:mkdir "direntname"
	6 ls
	7 pwd
	100:log in登录
	200:register注册
	0:exit
*/
inline int Reply(int newfd,int flagFile,char *buf)
{
	CMDTomas ct;
	bzero(&ct,sizeof(CMDTomas));
	ct.flagFile=flagFile;
	ct.len=strlen(buf);
	strcpy(ct.buf,buf);
//	printf("flagFile=%d,len=%d,buf=%s\n",flagFile,ct.len,ct.buf);
	int ret=LoopSend(newfd,(char*)&ct,sizeof(CMDTomas));
}

/*命令处理，命令成功返回0，执行exit命令返回1，
命令执行失败返回-1，命令匹配不成功返回-2*/
int HandleCmd(char *input,int newfd,char *userWorkPath)
{
	//分离命令
	char bufcmd[128]={0};
	int cmdNo=0;
	sscanf(input,"%d%s",&cmdNo,bufcmd);	
	//执行对应命令
	switch(cmdNo)
	{
	case 1:
		return Cd(newfd,bufcmd,userWorkPath);
	case 2:
		return Puts(newfd,bufcmd,userWorkPath);	
	case 3:
		return Gets(newfd,bufcmd,userWorkPath);
	case 4:
		return Remove(newfd,bufcmd,userWorkPath);
	case 5:
		return Mkdir(newfd,bufcmd,userWorkPath);
	case 6:
		return Ls(newfd,userWorkPath);
	case 7:
		return Pwd(newfd,userWorkPath);
	case 0:
		return 1;
	case 100:
		return LogIn(newfd);
	case 200:
		return Register(newfd);

	}
	
}
/*除了exit成功返回1，所有命令成功返回0，失败返回-1*/
int Cd(int newfd,char *input,char *userWorkPath)
{
	char pathPwd[MAX_PATH_LEN]={0};
	getcwd(pathPwd,MAX_PATH_LEN);
	int layerPwd=0;
	for(int i=0;pathPwd[i]!=0;i++)
	{
		if(pathPwd[i]=='/')
			layerPwd++;
	}
	int userRootLayer=layerPwd+1;
	//如果userWorkPath与pwd一致，说明才注册或登录，
	//需要更换到用户级根目录
	if(strcmp(userWorkPath,pathPwd)==0)
	{
		sprintf(userWorkPath,"%s/%s",userWorkPath,input);
		Reply(newfd,0," ");
		return 0;
	}
	//返回上一级
	if(strcmp(input,"..")==0)
	{
		int objPathLen=0;
		int layer=0;
		for(int i=0;userWorkPath[i]!=0;i++)
		{
			if(userWorkPath[i]=='/')
			{
				layer++;	
				objPathLen=i;
			}
		}
		//在用户根目录，不做任何改变
		if(layer==userRootLayer)
		{
			Reply(newfd,0," ");
			return 0;
		}
		bzero(userWorkPath+objPathLen,MAX_PATH_LEN-objPathLen);
	}
	//到下一级目录
	else
	{
		char objPathReal[MAX_PATH_LEN]={0};
		sprintf(objPathReal,"%s/%s",userWorkPath,input);
		//检验是否能打开此目录
		DIR *dir=opendir(objPathReal);
		if(dir==NULL)
		{
			Reply(newfd,0,"路径不存在");
			return -1;
		}
		//能打开,取得用户绝对路径
		closedir(dir);
		sprintf(userWorkPath,"%s/%s",userWorkPath,input);
	}
	Reply(newfd,0," ");
	return 0;
}

int Puts(int newfd,char *input,char *userWorkPath)
{
	char objPathReal[MAX_PATH_LEN]={0};
	sprintf(objPathReal,"%s/%s",userWorkPath,input);
	struct stat fileStat;
	if(stat(objPathReal,&fileStat)==0||*input==0)
	{
		Reply(newfd,0,"文件已存在或没有给定文件名");
		return -1;
	}
	Reply(newfd,2,input);
	RecvFile(newfd,objPathReal);
	return 0;
}
int Gets(int newfd,char *input,char *userWorkPath)
{
	char objPathReal[MAX_PATH_LEN]={0};
	//getcwd(objPathReal,MAX_PATH_LEN);
	sprintf(objPathReal,"%s/%s",userWorkPath,input);
	struct stat fileStat;
	if(stat(objPathReal,&fileStat)==-1||*input==0)
	{
		Reply(newfd,0,"权限不够或文件不存在");
		return -1;
	}
	Reply(newfd,1,"准备发送文件");
	return SendFile(newfd,objPathReal,input,fileStat.st_size);
	
}
int Remove(int newfd,char *input,char *userWorkPath)
{
	char objPathReal[MAX_PATH_LEN]={0};
	//getcwd(objPathReal,MAX_PATH_LEN);
	sprintf(objPathReal,"%s/%s",userWorkPath,input);
	//删除文件
	if(remove(objPathReal)==-1)
	{
		Reply(newfd,0,"删除失败");
		return -1;
	}
	Reply(newfd,0,"删除成功");
	return 0;
}
int Mkdir(int newfd,char* input,char* userWorkPath)
{
	char objPathReal[MAX_PATH_LEN]={0};
	//getcwd(objPathReal,MAX_PATH_LEN);
	sprintf(objPathReal,"%s/%s",userWorkPath,input);
	int ret=mkdir(objPathReal,0777);
	if(ret==-1)
	{
		Reply(newfd,0,"文件夹已存在");
		return -1;
	}

	Reply(newfd,0,"创建成功");
	return 0;
}
inline int Pwd(int newfd,char* userWorkPath)
{	
	puts(userWorkPath);
	Reply(newfd,0,userWorkPath);
	return 0;
}
int Ls(int newfd,char *userWorkPath)
{
	char buf[128]={0};
	//当前目录
	DIR *dir=opendir(userWorkPath);
	if(dir==NULL)
	{
		puts("打开目录失败");
		return -1;
	}
	//读取目录文件
	puts("开始读取目录");
	struct dirent *current;
	struct stat fileStat;
	while((current=(readdir(dir)))!=NULL)
	{
		bzero(buf,sizeof(buf));
		//如果是 . 或者 .. 文件，跳过
		if(strcmp(current->d_name,".")==0||
				strcmp(current->d_name,"..")==0)
		{
			continue;
		}
		//如果是目录
		else if(current->d_type==DT_DIR)
		{
			sprintf(buf,"%s%s/",buf,current->d_name);
			Reply(newfd,-1,buf);
		}
		//其他文件
		else
		{
			stat(current->d_name,&fileStat);
			sprintf(buf,"%s%-20s%ldB",buf,current->d_name,fileStat.st_size);
			Reply(newfd,-1,buf);
		}
	}
	closedir(dir);
	//代表发送结束
	Reply(newfd,0," ");
	return 0;	
}
/*注册*/
int Register(int newfd)
{
	printf("hello\n");
	char username[128]={0};
	char upwd[128]={0};
	char tablename[128]="alluser";
	CMDTomas ct;
	while(1)
	{
		bzero(&ct,sizeof(ct));
		bzero(username,sizeof(username));
		LoopRecv(newfd,(char*)&ct,sizeof(ct));
		//若客户端点击了取消
		if(ct.flagFile==0)
			return 1;
		strcpy(username,ct.buf);
		puts(username);
		bzero(&ct,sizeof(ct));
		//先判断数据库中是否存在此用户
		if(SearchUser(tablename,username)!=NULL)
		{
			//若被使用发送2000表示已被使用
			ct.flagFile=2000;
			LoopSend(newfd,(char*)&ct,sizeof(ct));
		}
		else
		{
			//通知客户端发送密码
			ct.flagFile=200;
			LoopSend(newfd,(char*)&ct,sizeof(ct));
			break;
		}
	}
	//接收密码
	bzero(&ct,sizeof(ct));
	LoopRecv(newfd,(char*)&ct,sizeof(ct));
	strcpy(upwd,ct.buf);
	//存入数据库
	InsertIntoDB(tablename,username,upwd);
	
	return 0;
}
/*登录*/
int LogIn(int newfd)
{
	char username[128]={0};
	char* userpwd;
	char recvpwd[128]={0};
	char tablename[128]="alluser";
	CMDTomas ct;
	while(1)
	{
		bzero(&ct,sizeof(ct));
		bzero(username,sizeof(username));
		LoopRecv(newfd,(char*)&ct,sizeof(ct));
		//若客户端点击了取消
		if(ct.flagFile==0)
			return 1;
		strcpy(username,ct.buf);
		bzero(&ct,sizeof(ct));
		//先判断数据库中是否存在此用户
		if((userpwd=SearchUser(tablename,username))==NULL)
		{
			//若不存在
			ct.flagFile=1000;
			LoopSend(newfd,(char*)&ct,sizeof(ct));
		}
		else
		{
			//通知客户端发送密码
			ct.flagFile=100;
			LoopSend(newfd,(char*)&ct,sizeof(ct));
			break;
		}
	}
	//与真实用户密码比对
	while(1)
	{
		//接收密码
		bzero(&ct,sizeof(ct));
		bzero(recvpwd,sizeof(recvpwd));
		LoopRecv(newfd,(char*)&ct,sizeof(ct));
		strcpy(recvpwd,ct.buf);
		//若客户端点了取消
		if(ct.flagFile==0)
			return 1;
		bzero(&ct,sizeof(ct));
		if(strcmp(userpwd,recvpwd)==0)
		{
			//通知客户端收到
			ct.flagFile=100;
			LoopSend(newfd,(char*)&ct,sizeof(ct));
			break;
		}
		ct.flagFile=1001;
		LoopSend(newfd,(char*)&ct,sizeof(ct));
	}
	return 0;
}

