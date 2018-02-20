#include "gui.h"

FileStat FileBeChose;	//��ѡ�е��ļ�
FileStat OnePageFile[MAX_FILE_NUM];
//���ڵײ����ĸ���ť����
RECT ButtonZone[4] = {  { 0,HEIGHT - 50,50,HEIGHT },
						{ 50,HEIGHT - 50,200,HEIGHT },
						{ 200,HEIGHT - 50 ,WIDTH - 50,HEIGHT },
						{ WIDTH - 50,HEIGHT - 50,WIDTH,HEIGHT } };
//ѡ���ļ����һ��Ĵ�������
RECT FileMrbZone[3] = { { WIDTH / 2 - 150,HEIGHT / 2 - 100,WIDTH / 2 ,HEIGHT/2 - 50},
						{ WIDTH / 2 - 150,HEIGHT / 2 - 50,WIDTH / 2 ,HEIGHT/2 },
						{ WIDTH / 2 - 150,HEIGHT / 2,WIDTH / 2 ,HEIGHT/2 + 50 } };
//��ʾ�ļ���ϸ��Ϣ������
RECT FileInfoZone[3]= { { WIDTH / 2 ,HEIGHT / 2 - 100,WIDTH / 2 + 150 ,HEIGHT/2 - 50 },
						{ WIDTH / 2 ,HEIGHT / 2 - 50,WIDTH / 2 + 150 ,HEIGHT/2 },
						{ WIDTH / 2 ,HEIGHT / 2,WIDTH / 2 + 150 ,HEIGHT/2 + 50 }};
//�๦�ܴ�������
RECT MultiFuncZone[3] = {{ WIDTH / 2 - 100,HEIGHT / 2 - 100,WIDTH / 2 + 100,HEIGHT / 2 + 100 },
						 { WIDTH / 2 - 50,HEIGHT / 2 - 75,WIDTH / 2 + 50,HEIGHT / 2 - 25},
						 { WIDTH / 2 - 50,HEIGHT / 2 + 25,WIDTH / 2 + 50,HEIGHT / 2 + 75 }, };

/*ANSI char ת���� LPCTSTR*/
inline wchar_t* ToLPCTSTR(char* buf)
{
	int num = MultiByteToWideChar(0, 0, buf, -1, NULL, 0);
	wchar_t *widebuf = new wchar_t[num];
	MultiByteToWideChar(0, 0, buf, -1, widebuf, num);
	return widebuf;
}
/*LPCTSTR ת���� ANSI char*/
inline char* ToAnsiStr(wchar_t* widebuf)
{
	int num = WideCharToMultiByte(0, 0, widebuf, -1, NULL, 0, 0, 0);
	char* buf = new char[num];
	WideCharToMultiByte(0, 0, widebuf, -1, buf, num, 0, 0);
	return buf;
}
/*��ȡ��ǰ�ļ������ļ�������*/
inline int GetFileNum()
{
	int i = 0;
	for (i; OnePageFile[i].filetype != 0; i++);
	return i;
}

/*ҳ���¼�*/
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
		case 1:			//������һ��Ŀ¼
		{
			cleardevice();
			return "1 ..";
		}
		case 2:			//�ϴ��ļ�
		{
			strcpy(cmdbuf, "2 ");
			WCHAR input[128] = { 0 };
			if (InputBox(input, 64, _T("����Ҫ�ϴ����ļ���"), _T("�ϴ��ļ�"), NULL, 0, 0, false))
			{
				char filename[128] = { 0 };
				strcpy(filename, ToAnsiStr(input));
				//��鱾���Ƿ���ڴ��ļ�
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
		case 5:			//�½��ļ���
		{
			if (GetFileNum() < MAX_FILE_NUM)
			{
				strcpy(cmdbuf, "5 ");
				WCHAR input[128] = { 0 };
				if (InputBox(input, 64, _T("�������ļ�����"), _T("�½��ļ���"), NULL, 0, 0, false))
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
			//��������
			//��ʾ����
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
		case 100:		//ѡ���ļ����һ�
		{
			ret = MseRClickFb();
			strcpy(cmdbuf, CmClClickFb(ret));
			if (strcmp(cmdbuf, " ") != 0)
				return cmdbuf;
			break;
		}
		default:		//��Ļ�ײ�����������
		{
			strcpy(cmdbuf, CmClClickFb(ret));
			if (strcmp(cmdbuf, " ") != 0)
				return cmdbuf;
			break;
		}
		}//end while
		
	}//end while
}
/*����������
return:
 2�ϴ��ļ�
 5�����ļ���
 1������һ�� cd ..
 0�˳�
 -99�հ�
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
			//���������ʽΪ��б��ͼ�����
			setfillcolor(BLACK);
			setfillstyle(BS_HATCHED, HS_BDIAGONAL);
			//�㵽�ļ�
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
							goto pageReClick;	//���¿�ʼ���
						}
					}
				}
			}
			//�㵽��ť
			for (int i = 0; i < 4; i++)
			{
				r = ButtonZone[i];
				if (m.x > r.left&&m.x<r.right&&m.y>r.top&&m.y < r.bottom)
				{
					switch (i)
					{
					case 0:
					{
						
						return 2;//�ϴ��ļ�
					}
					case 1:
					{
						return 5;//�����ļ���
					}
					case 2:
						return 1;//������һ�� cd ..
					case 3:
						return 0;//�˳�
					}//end switch
				}
			}
			//�㵽�հ״�
			break;
		}//end if
	}
	return -99;
}
/*�������ļ�����һ�����
return:
11���ļ���
3�����ļ�
4ɾ���ļ�
-99�հ�
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
					return 11;//���ļ��� cd "pathname"
				}
				else
				{
					return 3;//�����ļ�
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
				return 4;//ɾ���ļ�
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
/*���ȷ��ȡ�����ڷ���*/
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
				//��������
				char cmdbuf[128] = {0};
				switch(cmdNo)
				{
				case 0:				//�˳�
					return "0";		
				case 11:			//���ļ���
				{
					cleardevice();
					strcpy(cmdbuf,"1 ");
					return strcat(cmdbuf, FileBeChose.filename);
				}
				case 3:				//�����ļ�
				{
					cleardevice();
					DrawPage();
					strcpy(cmdbuf, "3 ");
					return strcat(cmdbuf, FileBeChose.filename);
				}
				case 4:				//ɾ���ļ�
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
/*�����¼ע�ᴰ�ڷ���
return
100:��¼
200:ע��*/
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
			//��¼��ť
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
			//ע�ᰴť
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



/*����½ע�ᴰ��*/
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
			drawtext(_T("��¼"), &r,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		case 2:
			fillrectangle(r.left, r.top, r.right, r.bottom);
			drawtext(_T("ע��"), &r,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}//end switch
	}
	EndBatchDraw();
}
/*����ʾ���󴰿�*/
inline void DrawError(int cmdNo)
{
	setfillcolor(RED);
	setfillstyle(BS_SOLID);
	BeginBatchDraw();
	RECT r = MultiFuncZone[0];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	r = MultiFuncZone[1];
	drawtext(_T("����"), &r,
		DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = MultiFuncZone[2];
	switch (cmdNo)
	{
	case 2:
		drawtext(_T("�ļ�������"), &r, DT_CENTER | DT_WORDBREAK);
		break;
	case 5:
		drawtext(_T("��ǰ�ļ��������Ѵﵽ����"), &r,DT_CENTER| DT_WORDBREAK);
		break;
	case 1000:
		drawtext(_T("�û�������"), &r, DT_CENTER | DT_WORDBREAK);
		break;
	case 1001:
		drawtext(_T("�������"), &r, DT_CENTER | DT_WORDBREAK);
		break;
	case 2000:
		drawtext(_T("���û����ѱ�ʹ��"), &r, DT_CENTER | DT_WORDBREAK);
		break;
	default:
		break;
	}
	EndBatchDraw();
}
/*����ȷ��ȡ��������*/
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
			drawtext(_T("ȷ��"), &r,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		case 2:
			fillrectangle(r.left, r.top, r.right, r.bottom);
			drawtext(_T("ȡ��"), &r,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;		
		}//end switch
	}
	EndBatchDraw();
}
/*���ļ���ϸ��Ϣ����*/
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
				drawtext(_T("�ļ���"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			else
				drawtext(_T("�ļ�"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}//end switch
	}
	EndBatchDraw();
}
/*���һ��ļ���ĶԻ���*/
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
				drawtext(_T("��"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			else
				drawtext(_T("����"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}
		case 1:
			drawtext(_T("��ʾ��ϸ��Ϣ"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		case 2:
			drawtext(_T("ɾ��"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}//end switch
	}

	EndBatchDraw();
}
/*��ҳ��*/
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
	//���ײ�ͼ��:�ϴ��������ļ��С�������һ�����˳�
	setfillcolor(GREEN);
	r = ButtonZone[0];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("�ϴ�"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE| DT_WORD_ELLIPSIS);
	r = ButtonZone[1];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("�½��ļ���"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = ButtonZone[2];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("������һ��"), &r, DT_CENTER | DT_VCENTER| DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = ButtonZone[3];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("�˳�"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	line(r.left, r.top, r.right, r.bottom);
	line(r.left+50, r.top, r.right-50, r.bottom);

	EndBatchDraw();
}

/*����������������ʹ��ʱ�������ڲ���ʹ����������,��ֹ����
�������ǻ�����һ������*/
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
	//���ײ�ͼ��:�ϴ��������ļ��С�������һ�����˳�
	setfillcolor(GREEN);
	r = ButtonZone[0];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("�ϴ�"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = ButtonZone[1];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("�½��ļ���"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = ButtonZone[2];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("������һ��"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = ButtonZone[3];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	drawtext(_T("�˳�"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
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
				drawtext(_T("��"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			else
				drawtext(_T("����"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}
		case 1:
			drawtext(_T("��ʾ��ϸ��Ϣ"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		case 2:
			drawtext(_T("ɾ��"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			break;
		}//end switch
	}
}