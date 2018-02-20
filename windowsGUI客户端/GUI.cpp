#include "gui.h"

FileStat FileBeChose;	//被选中的文件
FileStat OnePageFile[MAX_FILE_NUM];
//窗口底部的四个按钮区域
RECT ButtonZone[4] = {  { 0,HEIGHT - 50,50,HEIGHT },
						{ 50,HEIGHT - 50,200,HEIGHT },
						{ 200,HEIGHT - 50 ,WIDTH - 50,HEIGHT },
						{ WIDTH - 50,HEIGHT - 50,WIDTH,HEIGHT } };
//选中文件后右击的窗口区域
RECT FileMrbZone[3] = { { WIDTH / 2 - 150,HEIGHT / 2 - 100,WIDTH / 2 ,HEIGHT/2 - 50},
						{ WIDTH / 2 - 150,HEIGHT / 2 - 50,WIDTH / 2 ,HEIGHT/2 },
						{ WIDTH / 2 - 150,HEIGHT / 2,WIDTH / 2 ,HEIGHT/2 + 50 } };
//显示文件详细信息的区域
RECT FileInfoZone[3]= { { WIDTH / 2 ,HEIGHT / 2 - 100,WIDTH / 2 + 150 ,HEIGHT/2 - 50 },
						{ WIDTH / 2 ,HEIGHT / 2 - 50,WIDTH / 2 + 150 ,HEIGHT/2 },
						{ WIDTH / 2 ,HEIGHT / 2,WIDTH / 2 + 150 ,HEIGHT/2 + 50 }};
//多功能窗口区域
RECT MultiFuncZone[3] = {{ WIDTH / 2 - 100,HEIGHT / 2 - 100,WIDTH / 2 + 100,HEIGHT / 2 + 100 },
						 { WIDTH / 2 - 50,HEIGHT / 2 - 75,WIDTH / 2 + 50,HEIGHT / 2 - 25},
						 { WIDTH / 2 - 50,HEIGHT / 2 + 25,WIDTH / 2 + 50,HEIGHT / 2 + 75 }, };

/*ANSI char 转换成 LPCTSTR*/
inline wchar_t* ToLPCTSTR(char* buf)
{
	int num = MultiByteToWideChar(0, 0, buf, -1, NULL, 0);
	wchar_t *widebuf = new wchar_t[num];
	MultiByteToWideChar(0, 0, buf, -1, widebuf, num);
	return widebuf;
}
/*LPCTSTR 转换成 ANSI char*/
inline char* ToAnsiStr(wchar_t* widebuf)
{
	int num = WideCharToMultiByte(0, 0, widebuf, -1, NULL, 0, 0, 0);
	char* buf = new char[num];
	WideCharToMultiByte(0, 0, widebuf, -1, buf, num, 0, 0);
	return buf;
}
/*获取当前文件夹中文件的数量*/
inline int GetFileNum()
{
	int i = 0;
	for (i; OnePageFile[i].filetype != 0; i++);
	return i;
}

/*页面事件*/
char* PageEvent()
{
	char cmdbuf[128] = { 0 };
	int ret = -99;
	while (1)
	{
		ZeroMemory(cmdbuf,sizeof(cmdbuf));
		DrawPage();
		ret = MseLClickFb();
		switch (ret)
		{
		case -99:
			break;
		case 1:			//返回上一级目录
		{
			cleardevice();
			return "1 ..";
		}
		case 2:			//上传文件
		{
			strcpy(cmdbuf, "2 ");
			WCHAR input[128] = { 0 };
			if (InputBox(input, 64, _T("输入要上传的文件名"), _T("上传文件"), NULL, 0, 0, false))
			{
				char filename[128] = { 0 };
				strcpy(filename, ToAnsiStr(input));
				//检查本地是否存在此文件
				FILE* file = fopen(filename, "rb");
				if (!file)
				{
					MOUSEMSG m;
					while (1)
					{
						FlushMouseMsgBuffer();
						m = GetMouseMsg();
						DrawError(2);
						if (m.mkLButton)
						{
							cleardevice();
							DrawPage();
							break;
						}
					}
					break;
				}
				else
				{
					fclose(file);
					return strcat(cmdbuf, filename);
				}
			}
			else
			{
				FlushMouseMsgBuffer();
				break;
			}
		}
		case 5:			//新建文件夹
		{
			if (GetFileNum() < MAX_FILE_NUM)
			{
				strcpy(cmdbuf, "5 ");
				WCHAR input[128] = { 0 };
				if (InputBox(input, 64, _T("请输入文件夹名"), _T("新建文件夹"), NULL, 0, 0, false))
				{
					FlushMouseMsgBuffer();
					char filename[128] = { 0 };
					strcpy(filename, ToAnsiStr(input));
					return strcat(cmdbuf, filename);
				}
				else
				{
					FlushMouseMsgBuffer();
					break;
				}
			}
			//若不存在
			//提示错误
			else
			{
				MOUSEMSG m;
				while (1)
				{
					m = GetMouseMsg();
					DrawError(5);
					if (m.mkLButton)
					{
						cleardevice();
						DrawPage();
						break;
					}
				}
			}
			break;
		}
		case 100:		//选中文件后右击
		{
			ret = MseRClickFb();
			strcpy(cmdbuf, CmClClickFb(ret));
			if (strcmp(cmdbuf, " ") != 0)
				return cmdbuf;
			break;
		}
		default:		//屏幕底部的其他命令
		{
			strcpy(cmdbuf, CmClClickFb(ret));
			if (strcmp(cmdbuf, " ") != 0)
				return cmdbuf;
			break;
		}
		}//end while
		
	}//end while
}
/*鼠标左击反馈
return:
 2上传文件
 5创建文件夹
 1返回上一级 cd ..
 0退出
 -99空白
*/
int MseLClickFb()
{
	MOUSEMSG m;
	while (1)
	{
		m = GetMouseMsg();
		if (m.mkLButton)
		{
			RECT r;
pageReClick:
			//设置填充样式为反斜线图案填充
			setfillcolor(BLACK);
			setfillstyle(BS_HATCHED, HS_BDIAGONAL);
			//点到文件
			for (int i = 0; OnePageFile[i].filetype != 0; i++)
			{
				r = OnePageFile[i].zone;
				if (m.x > r.left&&m.x<r.right&&m.y>r.top&&m.y < r.bottom)
				{
					fillrectangle(r.left, r.top, r.right, r.bottom);
					FileBeChose = OnePageFile[i];
					while (1)
					{
						m = GetMouseMsg();
						if (m.mkRButton)
						{
							return 100;
						}
						else if (m.mkLButton)
						{
							DrawPage();
							goto pageReClick;	//重新开始点击
						}
					}
				}
			}
			//点到按钮
			for (int i = 0; i < 4; i++)
			{
				r = ButtonZone[i];
				if (m.x > r.left&&m.x<r.right&&m.y>r.top&&m.y < r.bottom)
				{
					switch (i)
					{
					case 0:
					{
						
						return 2;//上传文件
					}
					case 1:
					{
						return 5;//创建文件夹
					}
					case 2:
						return 1;//返回上一级 cd ..
					case 3:
						return 0;//退出
					}//end switch
				}
			}
			//点到空白处
			break;
		}//end if
	}
	return -99;
}
/*鼠标左击文件后的右击反馈
return:
11打开文件夹
3下载文件
4删除文件
-99空白
*/
int MseRClickFb()
{
	DrawRClickDialog();
	MOUSEMSG m;
	RECT r;
	while (1)
	{
		m = GetMouseMsg();

		BeginBatchDraw();
		if (m.x > FileMrbZone[0].left&&m.x<FileMrbZone[0].right&&
			m.y>FileMrbZone[0].top&&m.y < FileMrbZone[0].bottom)
		{
			cleardevice();
			DrawPageNoBatch();
			DrawRClickNoBatch();
			setfillcolor(BLACK);
			setfillstyle(BS_HATCHED, HS_BDIAGONAL);
			r = FileMrbZone[0];
			fillrectangle(r.left, r.top, r.right, r.bottom);
			EndBatchDraw();
			if (m.mkLButton)
			{
				cleardevice();
				DrawPage();
				if (FileBeChose.filetype == FLOADER)
				{
					return 11;//打开文件夹 cd "pathname"
				}
				else
				{
					return 3;//下载文件
				}
			}
		}
		else if (m.x > FileMrbZone[1].left&&m.x<FileMrbZone[1].right&&
			m.y>FileMrbZone[1].top&&m.y < FileMrbZone[1].bottom)
		{
			DrawRClickDialog();
			setfillcolor(BLACK);
			setfillstyle(BS_HATCHED, HS_BDIAGONAL);
			r = FileMrbZone[1];
			fillrectangle(r.left, r.top, r.right, r.bottom);
			DrawFileInfo();
			EndBatchDraw();
		}
		else if (m.x > FileMrbZone[2].left&&m.x<FileMrbZone[2].right&&
			m.y>FileMrbZone[2].top&&m.y < FileMrbZone[2].bottom)
		{
			cleardevice();
			DrawPageNoBatch();
			DrawRClickNoBatch();
			setfillcolor(BLACK);
			setfillstyle(BS_HATCHED, HS_BDIAGONAL);
			r = FileMrbZone[2];
			fillrectangle(r.left, r.top, r.right, r.bottom);
			EndBatchDraw();
			if (m.mkLButton)
			{
				cleardevice();
				DrawPage();
				return 4;//删除文件
			}
		}
		else if (m.mkLButton)
		{
			cleardevice();
			break;
		}
	}
	return -99;
}
/*左击确认取消窗口反馈*/
char* CmClClickFb(int cmdNo)
{
	DrawCmCl();
	MOUSEMSG m;
	RECT r;
	while (1)
	{
		m = GetMouseMsg();
		BeginBatchDraw();
		if (m.x > MultiFuncZone[1].left&&m.x<MultiFuncZone[1].right&&
			m.y>MultiFuncZone[1].top&&m.y < MultiFuncZone[1].bottom)
		{
			DrawCmCl();
			setfillcolor(BLACK);
			setfillstyle(BS_HATCHED, HS_BDIAGONAL);
			r = MultiFuncZone[1];
			fillrectangle(r.left, r.top, r.right, r.bottom);
			EndBatchDraw();
			if (m.mkLButton)
			{
				//发送命令
				char cmdbuf[128] = {0};
				switch(cmdNo)
				{
				case 0:				//退出
					return "0";		
				case 11:			//打开文件夹
				{
					cleardevice();
					strcpy(cmdbuf,"1 ");
					return strcat(cmdbuf, FileBeChose.filename);
				}
				case 3:				//下载文件
				{
					cleardevice();
					DrawPage();
					strcpy(cmdbuf, "3 ");
					return strcat(cmdbuf, FileBeChose.filename);
				}
				case 4:				//删除文件
				{
					cleardevice();
					strcpy(cmdbuf, "4 ");
					return strcat(cmdbuf, FileBeChose.filename);
				}
				default:
					break;
				}
			}
		}
		else if (m.x > MultiFuncZone[2].left&&m.x<MultiFuncZone[2].right&&
			m.y>MultiFuncZone[2].top&&m.y < MultiFuncZone[2].bottom)
		{
			DrawCmCl();
			setfillcolor(BLACK);
			setfillstyle(BS_HATCHED, HS_BDIAGONAL);
			r = MultiFuncZone[2];
			fillrectangle(r.left, r.top, r.right, r.bottom);
			EndBatchDraw();
			if (m.mkLButton)
			{
				cleardevice();
				break;
			}
		}
	}//end while
	return " ";
}
/*左击登录注册窗口反馈
return
100:登录
200:注册*/
int RegLogClickFb()
{
	DrawRegLog();
	MOUSEMSG m;
	RECT r;
	while (1)
	{
		m = GetMouseMsg();
		BeginBatchDraw();
		if (m.x > MultiFuncZone[1].left&&m.x<MultiFuncZone[1].right&&
			m.y>MultiFuncZone[1].top&&m.y < MultiFuncZone[1].bottom)
		{
			//登录按钮
			DrawRegLog();
			setfillcolor(BLACK);
			setfillstyle(BS_HATCHED, HS_BDIAGONAL);
			r = MultiFuncZone[1];
			fillrectangle(r.left, r.top, r.right, r.bottom);
			EndBatchDraw();
			if (m.mkLButton)
			{
				cleardevice();
				return 100;
			}
		}
		else if (m.x > MultiFuncZone[2].left&&m.x<MultiFuncZone[2].right&&
			m.y>MultiFuncZone[2].top&&m.y < MultiFuncZone[2].bottom)
		{
			//注册按钮
			DrawRegLog();
			setfillcolor(BLACK);
			setfillstyle(BS_HATCHED, HS_BDIAGONAL);
			r = MultiFuncZone[2];
			fillrectangle(r.left, r.top, r.right, r.bottom);
			EndBatchDraw();
			if (m.mkLButton)
			{
				cleardevice();
				return 200;
			}
		}
	}//end while
}



/*画登陆注册窗口*/
inline void DrawRegLog()
{
	BeginBatchDraw();
	RECT r;

	for (int j = 0; j < 3; j++)
	{
		setfillcolor(RED);
		setfillstyle(BS_SOLID);
		r = MultiFuncZone[j];
		switch (j)
		{
		case 0:
			setfillcolor(BLACK);
			fillrectangle(r.left, r.top, r.right, r.bottom);
			break;
		case 1:
			fillrectangle(r.left, r.top, r.right, r.bottom);
			drawtext(_T("登录"), &r,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		case 2:
			fillrectangle(r.left, r.top, r.right, r.bottom);
			drawtext(_T("注册"), &r,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}//end switch
	}
	EndBatchDraw();
}
/*画显示错误窗口*/
inline void DrawError(int cmdNo)
{
	setfillcolor(RED);
	setfillstyle(BS_SOLID);
	BeginBatchDraw();
	RECT r = MultiFuncZone[0];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	r = MultiFuncZone[1];
	drawtext(_T("错误："), &r,
		DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = MultiFuncZone[2];
	switch (cmdNo)
	{
	case 2:
		drawtext(_T("文件不存在"), &r, DT_CENTER | DT_WORDBREAK);
		break;
	case 5:
		drawtext(_T("当前文件夹数量已达到上限"), &r,DT_CENTER| DT_WORDBREAK);
		break;
	case 1000:
		drawtext(_T("用户不存在"), &r, DT_CENTER | DT_WORDBREAK);
		break;
	case 1001:
		drawtext(_T("密码错误"), &r, DT_CENTER | DT_WORDBREAK);
		break;
	case 2000:
		drawtext(_T("该用户名已被使用"), &r, DT_CENTER | DT_WORDBREAK);
		break;
	default:
		break;
	}
	EndBatchDraw();
}
/*画‘确认取消’窗口*/
inline void DrawCmCl()
{
	BeginBatchDraw();
	RECT r;
	
	for (int j = 0; j < 3; j++)
	{
		setfillcolor(RED);
		setfillstyle(BS_SOLID);
		r = MultiFuncZone[j];
		switch (j)
		{
		case 0:
			setfillcolor(BLACK);
			fillrectangle(r.left, r.top, r.right, r.bottom);
			break;
		case 1:
			fillrectangle(r.left, r.top, r.right, r.bottom);
			drawtext(_T("确认"), &r,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		case 2:
			fillrectangle(r.left, r.top, r.right, r.bottom);
			drawtext(_T("取消"), &r,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;		
		}//end switch
	}
	EndBatchDraw();
}
/*画文件详细信息窗口*/
inline void DrawFileInfo()
{
	BeginBatchDraw();
	RECT r;
	setfillcolor(RED);
	setfillstyle(BS_SOLID);
	for (int j = 0; j < 3; j++)
	{
		r = FileInfoZone[j];
		fillrectangle(r.left, r.top, r.right, r.bottom);
		switch (j)
		{
		case 0:
			drawtext(ToLPCTSTR(FileBeChose.filename), &r, 
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		case 1:
			drawtext(ToLPCTSTR(FileBeChose.bufsize), &r,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		case 2:
			if (FileBeChose.filetype == FLOADER)
				drawtext(_T("文件夹"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			else
				drawtext(_T("文件"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}//end switch
	}
	EndBatchDraw();
}
/*画右击文件后的对话框*/
inline void DrawRClickDialog()
{
	BeginBatchDraw();

	RECT r;
	setfillcolor(BLUE);
	setfillstyle(BS_SOLID);
	for (int j = 0; j < 3; j++)
	{
		r = FileMrbZone[j];
		fillrectangle(r.left, r.top, r.right, r.bottom);
		switch (j)
		{
		case 0:
		{
			if (FileBeChose.filetype == FLOADER)
				drawtext(_T("打开"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			else
				drawtext(_T("下载"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}
		case 1:
			drawtext(_T("显示详细信息"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		case 2:
			drawtext(_T("删除"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}//end switch
	}

	EndBatchDraw();
}
/*画页面*/
void DrawPage()
{
	BeginBatchDraw();
	setlinecolor(BLACK);
	settextcolor(BLACK);
	setfillstyle(BS_SOLID);
	int count = 0;
	int rowNum = HEIGHT / 100-1;
	int colNum = WIDTH / 100;
	RECT r;
	for (int i = 0; i < rowNum; i++)
	{
		for (int j = 0; j < colNum; j++)
		{
			if (OnePageFile[count].filetype == FLOADER)
				setfillcolor(YELLOW);
			else if (OnePageFile[count].filetype == DOCUMENT)
				setfillcolor(WHITE);
			else
				goto EndFolder;

			r = { j * 100 + SPACE, i * 100 + SPACE,
				(j + 1) * 100 - SPACE, (i + 1) * 100 - SPACE };
			OnePageFile[count].zone = r;
			fillrectangle(r.left, r.top, r.right, r.bottom);
			drawtext(ToLPCTSTR(OnePageFile[count].filename), &r, 
				DT_CENTER|DT_VCENTER| DT_SINGLELINE| DT_WORD_ELLIPSIS);
			count++;
			
		}
	}//end for
EndFolder:
	//画底部图形:上传、创建文件夹、返回上一级、退出
	setfillcolor(GREEN);
	r = ButtonZone[0];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("上传"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE| DT_WORD_ELLIPSIS);
	r = ButtonZone[1];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("新建文件夹"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = ButtonZone[2];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("返回上一级"), &r, DT_CENTER | DT_VCENTER| DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = ButtonZone[3];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("退出"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	line(r.left, r.top, r.right, r.bottom);
	line(r.left+50, r.top, r.right-50, r.bottom);

	EndBatchDraw();
}

/*当这两个函数连续使用时，在其内部不使用批量绘制,防止闪屏
，等他们画完再一并绘制*/
void DrawPageNoBatch()
{
	BeginBatchDraw();
	setlinecolor(BLACK);
	settextcolor(BLACK);
	setfillstyle(BS_SOLID);
	int count = 0;
	int rowNum = HEIGHT / 100 - 1;
	int colNum = WIDTH / 100;
	RECT r;
	for (int i = 0; i < rowNum; i++)
	{
		for (int j = 0; j < colNum; j++)
		{
			if (OnePageFile[count].filetype == FLOADER)
				setfillcolor(YELLOW);
			else if (OnePageFile[count].filetype == DOCUMENT)
				setfillcolor(WHITE);
			else
				goto EndFolder;

			r = { j * 100 + SPACE, i * 100 + SPACE,
				(j + 1) * 100 - SPACE, (i + 1) * 100 - SPACE };
			OnePageFile[count].zone = r;
			fillrectangle(r.left, r.top, r.right, r.bottom);
			drawtext(ToLPCTSTR(OnePageFile[count].filename), &r,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			count++;

		}
	}//end for
EndFolder:
	//画底部图形:上传、创建文件夹、返回上一级、退出
	setfillcolor(GREEN);
	r = ButtonZone[0];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("上传"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = ButtonZone[1];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("新建文件夹"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = ButtonZone[2];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("返回上一级"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = ButtonZone[3];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("退出"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	line(r.left, r.top, r.right, r.bottom);
	line(r.left + 50, r.top, r.right - 50, r.bottom);

	EndBatchDraw();
}
inline void DrawRClickNoBatch()
{
	RECT r;
	setfillcolor(BLUE);
	setfillstyle(BS_SOLID);
	for (int j = 0; j < 3; j++)
	{
		r = FileMrbZone[j];
		fillrectangle(r.left, r.top, r.right, r.bottom);
		switch (j)
		{
		case 0:
		{
			if (FileBeChose.filetype == FLOADER)
				drawtext(_T("打开"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			else
				drawtext(_T("下载"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}
		case 1:
			drawtext(_T("显示详细信息"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		case 2:
			drawtext(_T("删除"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}//end switch
	}
}