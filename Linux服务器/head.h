#ifndef __HEAD_H__
#define __HEAD_H__

#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <mysql/mysql.h>
#include <crypt.h>
#include <errno.h>

#define MAX_PATH_LEN 128
typedef struct
{
	int len;
	char buf[1000];
}Tomas;		//用托马斯货运火车传输文件
typedef struct
{
	/*-1连续回复,0只回复一次或回复结束,
	1准备向客户端发送文件,2准备从客户端接收文件
	100注册，200登录*/
	int flagFile;
	int len;
	char buf[512];
}CMDTomas;	//用命令托马斯火车传输命令
#endif

