#pragma once
#ifndef __GUI_H__
#define __GUI_H__

#include "head.h"
#define WIDTH 800
#define HEIGHT 600
#define SPACE 10
#define MAX_FILE_NUM ((WIDTH/100)*(HEIGHT/100-1))
#define FLOADER		-1
#define DOCUMENT	1
typedef struct
{
	char filename[128];
	char bufsize[128];
	int filetype;	//-1文件夹，1文件
	RECT zone;		//文件在页面上的矩形范围
}FileStat;

extern FileStat FileBeChose;	//被选中的文件
extern FileStat OnePageFile[MAX_FILE_NUM];
extern RECT ButtonZone[4];
extern RECT FileMrbZone[3];
extern RECT FileInfoZone[3];
extern RECT MultiFuncZone[3];


inline wchar_t* ToLPCTSTR(char* buf);
inline char* ToAnsiStr(wchar_t* widebuf);
char* PageEvent();
int MseLClickFb();
int MseRClickFb();
char* CmClClickFb(int cmdNo);
int RegLogClickFb();

inline void DrawRegLog();
inline void DrawError(int cmdNo);
inline void DrawCmCl();
inline void DrawFileInfo();
inline void DrawRClickDialog();
void DrawPage();
void DrawPageNoBatch();
inline void DrawRClickNoBatch();

#endif // !__GUI_H__
