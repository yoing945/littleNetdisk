#pragma once
#ifndef __COMMUNICATE_H__
#define __COMMUNICATE_H__

#include "head.h"

inline void ErrorHandling(char* message);
char* UtfToGbk(const char* utf8);
int NetToServer(char* argv[]);
int Communication(SOCKET hServer);
int GetFileList(SOCKET hServer);
inline int HandlngReply(SOCKET hServer);

char* RegOrLog(SOCKET hServer);
char* GUIRegister(SOCKET hServer);
char* GUILogin(SOCKET hServer);

int SendFile(SOCKET hServer, char* filename);
int LoopSend(SOCKET hServer, char* buf, int len);
int RecvFile(SOCKET hServer);
int LoopRecv(SOCKET hServer, char *buf, int len);

#endif // !__COMMUNICATE_H__

