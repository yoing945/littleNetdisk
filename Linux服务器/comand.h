#ifndef __COMAND_H__
#define __COMAND_H__

#include "head.h"

/*函数声明*/
int Mkdir(int,char*,char*);
int HandleCmd(char*,int,char*);
int Cd(int,char*,char*);
int Ls(int,char*);
int Puts(int,char*,char*);
int Gets(int,char*,char*);
int Remove(int,char*,char*);
int Pwd(int,char*);
int Cmd(int,char*);
int Reply(int,int,char *);

int Register(int newfd);
int LogIn(int newfd);
#endif
