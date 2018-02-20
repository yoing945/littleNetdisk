#pragma once
#ifndef  __HEAD_H__
#define  __HEAD_H__
#include <stdlib.h>
#include <winsock2.h>
#include <stdio.h>
#include <io.h>
#include <string>
#include <graphics.h>
#include <tchar.h>

#pragma comment(lib,"WS2_32")
using namespace std;

typedef struct
{
	int len;
	char buf[1000];
}Tomas;     
 typedef struct
 {
	 int flagFile; 
	 int len;
	 char buf[512];
 }CMDTomas;  


#endif // ! __HEAD_H__

