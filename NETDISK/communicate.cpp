#include "communicate.h"
#include "gui.h"

inline void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	printf("error code:%d\n", GetLastError());
}
/*����ת����Linux UTF-8תWindows GBK*/
char* UtfToGbk(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}
/*���ӵ�������*/
int NetToServer(char* ipPort[])
{
	//socket
	SOCKET hServer;
	hServer = socket(PF_INET, SOCK_STREAM, 0);
	if (hServer == INVALID_SOCKET)
	{
		ErrorHandling("socket() error");
		return -1;
	}
	SOCKADDR_IN serAddr;
	ZeroMemory(&serAddr, sizeof(serAddr));
	serAddr.sin_family = AF_INET;
	serAddr.sin_addr.s_addr = inet_addr(ipPort[0]);
	serAddr.sin_port = htons(atoi(ipPort[1]));
	//connect
	int ret = connect(hServer, (SOCKADDR*)&serAddr, sizeof(serAddr));
	if (ret != 0)
	{
		closesocket(hServer);
		ErrorHandling("connect() error");
		return ret;
	}
	printf("���ӳɹ���IP:%s PORT:%s\n",ipPort[0],ipPort[1]);
	//�Ի�
	ret=Communication(hServer);

	//�ر�socket���
	shutdown(hServer, SD_BOTH);
	closesocket(hServer);
	return ret;
}
/*ͨ��
	  ���������
	 1 "pathname"    :cd "pathname"
	 2 "filename"    :puts "filename"
	 3 "filename"    :gets "filename"
	 4 "filename"    :remove "filename"
	 5 "direntname"  :mkdir "direntname"
	 6:ls (�����ļ����Զ�����)
	 7:pwd(���ڵĹ����ò���)
	 0:exit
	 100:��½
	 200:ע��
 */
int Communication(SOCKET hServer)
{
	CMDTomas ct;
	int ret = 0;
	char* username;
	if ((username=RegOrLog(hServer)) == NULL)
		goto exitLabel;
	else
	{
		//�����û���Ŀ¼(���Ѵ��ڷ����������ٴ���)
		ZeroMemory(&ct, sizeof(ct));
		ct.flagFile = 5;
		sprintf(ct.buf, "%s%s", "5 ", username);
		LoopSend(hServer, (char*)&ct, sizeof(ct));
		LoopRecv(hServer,(char*)&ct,sizeof(ct));
		//cd���û���Ŀ¼
		ZeroMemory(&ct, sizeof(ct));
		ct.flagFile = 1;
		sprintf(ct.buf, "%s%s", "1 ", username);
		LoopSend(hServer, (char*)&ct, sizeof(ct));
		LoopRecv(hServer, (char*)&ct, sizeof(ct));
	}
	while (1)
	{
		ZeroMemory(&ct, sizeof(CMDTomas));
		//�Զ�����ls�����ܻظ�
		ret=GetFileList(hServer);
		//GUI�����������
		strcpy(ct.buf, PageEvent());
		ret=LoopSend(hServer, (char*)&ct, sizeof(CMDTomas));
		//�����˳�
		if (strcmp(ct.buf, "0") == 0)
			return 1;
		//���ղ�����ظ�
		ret=HandlngReply(hServer);
	}
exitLabel:
	return 0;
}
/*����ls����������ݲ�����OnePageFile*/
int GetFileList(SOCKET hServer)
{
	
	CMDTomas ct;
	int ret = 0;
	ZeroMemory(&ct, sizeof(CMDTomas));
	int count = 0;
	char filename[128] = { 0 };
	char bufsize[128] = { 0 };
	FileStat tempFile;
	//����ls����
	ZeroMemory(OnePageFile, sizeof(OnePageFile));
	strcpy(ct.buf, "6");
	LoopSend(hServer, (char*)&ct, sizeof(CMDTomas));
	//���ղ����ļ���Ϣ����OnePageFile
	do
	{
		ZeroMemory(&tempFile,sizeof(tempFile));
		ZeroMemory(&ct, sizeof(CMDTomas));
		ZeroMemory(filename, sizeof(filename));
		ZeroMemory(bufsize, sizeof(bufsize));
		ret = LoopRecv(hServer, (char*)&ct, sizeof(CMDTomas));
		if (ret < 0)
			return -1;
		if (ct.flagFile==-1)
		{
			sscanf(UtfToGbk(ct.buf), "%s%s", filename, bufsize);
			//������ļ���
			if (filename[strlen(filename) - 1] == '/')
			{
				filename[strlen(filename) - 1] = 0;
				strcpy(tempFile.filename, filename);
				tempFile.filetype = FLOADER;
				memcpy(OnePageFile + count, &tempFile, sizeof(FileStat));
			}
			else
			{
				strcpy(tempFile.filename, filename);
				strcpy(tempFile.bufsize, bufsize);
				tempFile.filetype = DOCUMENT;
				memcpy(OnePageFile + count, &tempFile, sizeof(FileStat));
			}
			count++;
			//����һҳ���ֻ����ʾMAX_FILE_NUM���ļ�����֪��զ����������
			//�����ǰ�ļ������ļ�����������MAX_FILE_NUM������ʾ
			if (count > MAX_FILE_NUM)
			{
				break;
			}
		}
	} while (ct.flagFile == -1);
	return 0;
}
/*����������ظ�����Ϣ*/
inline int HandlngReply(SOCKET hServer)
{
	int ret = 0;
	CMDTomas ct;
	char buf[sizeof(CMDTomas)];
	//flagFile=-1:��Ҫ�����ظ�
	do
	{
		ZeroMemory(&ct, sizeof(CMDTomas));
		ZeroMemory(buf, sizeof(buf));
		ret=LoopRecv(hServer, (char*)&ct, sizeof(CMDTomas));
		strcpy(buf, UtfToGbk(ct.buf));
		puts(buf);
	} while (ct.flagFile==-1);
	//flagFile=1:������׼���÷����ļ�
	if (ct.flagFile == 1)
	{
		ret = RecvFile(hServer);
		if (ret == 0)
			puts("���سɹ�");
		else
			puts("����ʧ��");
	}
	//flagFile=2:������׼���ý����ϴ����ļ�
	if (ct.flagFile == 2)
	{
		//�����ļ�������,֪ͨ������
		ret=SendFile(hServer, ct.buf);
		if (ret == 0)
			puts("�ϴ��ɹ�");
		else
			puts("�ϴ�ʧ��");
	}
	printf("-------------------------------------\n");
	return ret;
}
/*�����¼����ע��*/
char* RegOrLog(SOCKET hServer)
{
	int ret = RegLogClickFb();
	
	if (ret == 100)
	{
		//��¼
		return GUILogin(hServer);
	}
		//ע��
	return GUIRegister(hServer);

}
/*��¼,�����û���*/
char* GUILogin(SOCKET hServer)
{
	WCHAR input[128] = { 0 };
	char* username;
	CMDTomas ct;
	//����¼����
	ZeroMemory(&ct, sizeof(ct));
	strcpy(ct.buf, "100");
	LoopSend(hServer, (char*)&ct, sizeof(ct));
	while (1)
	{
		ZeroMemory(&ct, sizeof(ct));
		ZeroMemory(input, sizeof(input));
		if (InputBox(input, 64, _T("�����û���"), _T("��¼"), NULL, 0, 0, false))
		{
			//�����û���
			ct.flagFile = 100;
			username = ToAnsiStr(input);
			strcpy(ct.buf, username);
			LoopSend(hServer, (char*)&ct, sizeof(ct));
			ZeroMemory(&ct, sizeof(ct));
			LoopRecv(hServer, (char*)&ct, sizeof(ct));
			//�û�������
			if (ct.flagFile == 1000)
			{
				MOUSEMSG m;
				while (1)
				{
					FlushMouseMsgBuffer();
					m = GetMouseMsg();
					DrawError(1000);
					if (m.mkLButton)
					{
						cleardevice();
						break;
					}
				}
				continue;
			}
			else
				break;
		}
		//�û������ȡ��
		else
		{
			ct.flagFile = 0;
			LoopSend(hServer, (char*)&ct, sizeof(ct));
			return NULL;
		}
	}//end while
	while (1)
	{
		ZeroMemory(&ct, sizeof(ct));
		ZeroMemory(input, sizeof(input));
		if (InputBox(input, 64, _T("�������루����!��"),
			_T("��¼"), NULL, 0, 0, false))
		{
			ct.flagFile = 100;
			strcpy(ct.buf, ToAnsiStr(input));
			LoopSend(hServer, (char*)&ct, sizeof(ct));
			ZeroMemory(&ct, sizeof(ct));
			LoopRecv(hServer, (char*)&ct, sizeof(ct));
			//�������
			if (ct.flagFile == 1001)
			{
				MOUSEMSG m;
				while (1)
				{
					FlushMouseMsgBuffer();
					m = GetMouseMsg();
					DrawError(1001);
					if (m.mkLButton)
					{
						cleardevice();
						break;
					}
				}
				continue;
			}
			else
				break;
		}
		//�û������ȡ��
		else
		{
			ct.flagFile = 0;
			LoopSend(hServer, (char*)&ct, sizeof(ct));
			return NULL;
		}
	}
	return username;
}
/*ע��*/
char* GUIRegister(SOCKET hServer)
{
	WCHAR input[128] = { 0 };
	char* username;
	CMDTomas ct;
	//��ע������
	ZeroMemory(&ct, sizeof(ct));
	strcpy(ct.buf,"200");
	LoopSend(hServer,(char*)&ct,sizeof(ct));
	while (1)
	{
		ZeroMemory(&ct, sizeof(ct));
		ZeroMemory(input, sizeof(input));
		if (InputBox(input, 64, _T("�����û���"), _T("ע��"), NULL, 0, 0, false))
		{
			//�����û���
			ct.flagFile = 200;
			username = ToAnsiStr(input);
			strcpy(ct.buf,username);
			LoopSend(hServer, (char*)&ct, sizeof(ct));
			ZeroMemory(&ct,sizeof(ct));
			LoopRecv(hServer, (char*)&ct, sizeof(ct));
			//�û����Ѵ���
			if (ct.flagFile == 2000)
			{
				MOUSEMSG m;
				while (1)
				{
					FlushMouseMsgBuffer();
					m = GetMouseMsg();
					DrawError(2000);
					if (m.mkLButton)
					{
						cleardevice();
						break;
					}
				}
				continue;
			}
			else
				break;
		}
		//�û������ȡ��
		else
		{
			ct.flagFile = 0;
			LoopSend(hServer, (char*)&ct, sizeof(ct));
			return NULL;
		}
	}//end while
	//��������
	ZeroMemory(&ct, sizeof(ct));
	ZeroMemory(input, sizeof(input));
	InputBox(input, 64, _T("�������루���ģ���"),
		_T("ע��"), NULL, 0, 0, true);
	strcpy(ct.buf, ToAnsiStr(input));
	LoopSend(hServer, (char*)&ct, sizeof(ct));
	return username;
}


/*�����ļ�*/
int SendFile(SOCKET hServer,char* filename)
{
	//��ȡ�ļ���С
	unsigned _int64 size = 0;
	unsigned _int64 sum = 0;
	FILE* file = fopen(filename, "rb");
	if (!file)
	{
		printf("�����ļ�ʧ��\n");
		return -1;
	}
	size = (unsigned _int64  )_filelengthi64(fileno(file));
	//�û�������˹������
	Tomas tomas;
	ZeroMemory(&tomas, sizeof(Tomas));
	//�����ļ������ļ���С
	strcpy(tomas.buf, filename);
	tomas.len = strlen(tomas.buf) + sizeof(unsigned _int64  );
	strcpy(tomas.buf + strlen(tomas.buf), (char*)&size);
	int ret = LoopSend(hServer, (char*)&tomas, sizeof(int)+tomas.len);
	if (ret <0)
		return ret;
	//���ղ���ʾ����
	setfillcolor(RED);
	setfillstyle(BS_SOLID);
	setbkmode(OPAQUE);
	RECT r = MultiFuncZone[0];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	r = MultiFuncZone[1];
	drawtext(_T("�����ϴ�"), &r,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = MultiFuncZone[2];
	int percent;
	WCHAR numbuf[16] = { 0 };
	while (ZeroMemory(&tomas,sizeof(Tomas)),
		(tomas.len=fread(tomas.buf,sizeof(char),sizeof(tomas.buf),file))>0)
	{
		ZeroMemory(numbuf, sizeof(numbuf));
		sum += tomas.len;
		percent = sum * 100 / size;
		wsprintfW(numbuf, L"%d", percent);
		drawtext(numbuf, &r,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
		ret = LoopSend(hServer, (char*)&tomas, tomas.len + sizeof(int));
		if (ret < 0)
			goto end;
	}
	//����len=0�����ͽ���
	tomas.len = 0;
	LoopSend(hServer, (char*)&tomas, sizeof(int));
	r = MultiFuncZone[1];
	drawtext(_T("�ϴ��ɹ�"), &r,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	setbkmode(TRANSPARENT);
end:
	fclose(file);
	
	MOUSEMSG m;
	while (1)
	{
		FlushMouseMsgBuffer();
		m = GetMouseMsg();
		if (m.mkLButton)
		{
			cleardevice();
			DrawPage();
			break;
		}
	}
	return 0;
}
/*Loop Send*/
int LoopSend(SOCKET hServer,char* buf,int len)
{
	int total = 0;
	int ret = 0;
	while (total < len)
	{
		ret = send(hServer, buf, len - total, 0);
		if (ret<0)
		{
			printf("socket error:%d\n", GetLastError());
			return ret;
		}
		total =total + ret;
	}
	return 0;
}
/*�����ļ�*/
int RecvFile(SOCKET hServer)
{
	unsigned _int64 size = 0;
	unsigned _int64 sum = 0;
	Tomas tomas;
	ZeroMemory(&tomas, sizeof(Tomas));
	//�õ��ļ������ļ���С
	int ret = 0;
	ret=LoopRecv(hServer, (char*)&tomas, sizeof(int));
	if (ret < 0)
	{
		printf("�����ļ�ʧ��\n");
		return ret;
	}
	ret=LoopRecv(hServer, tomas.buf, tomas.len);
	if (ret < 0)
	{
		printf("�����ļ�ʧ��\n");
		return ret;
	}
	char filename[128] = { 0 };
	for (int i = 0; i < tomas.len - sizeof(unsigned _int64 ); i++)
		filename[i] = tomas.buf[i];
	for (int i = 0; i < sizeof(unsigned _int64 ); i++)
		((char*)&size)[i] = tomas.buf[strlen(filename)+i];
	printf("filename:%s\n", filename);
	//׼�������ļ�
	FILE* file = fopen(filename,"wb+");
	if (!file)
	{
		printf("�����ļ�ʧ��\n");
		return ret;
	}
	//���ղ���ʾ����
	setfillcolor(RED);
	setfillstyle(BS_SOLID);
	setbkmode(OPAQUE);
	RECT r = MultiFuncZone[0];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	r = MultiFuncZone[1];
	drawtext(_T("��������"), &r,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	r = MultiFuncZone[2];
	int percent;
	WCHAR numbuf[16] = {0};
	while (1)
	{
		ZeroMemory(&tomas,sizeof(Tomas));
		ZeroMemory(numbuf, sizeof(numbuf));
		ret=LoopRecv(hServer, (char*)&tomas, sizeof(int));
		
		if (tomas.len > 0)
		{
			
			LoopRecv(hServer, tomas.buf, tomas.len);
			fwrite(tomas.buf, sizeof(char), tomas.len, file);
			//����ٷֱ�
			sum += tomas.len;
			percent = sum * 100 / size;
			wsprintfW(numbuf, L"%d", percent);
			drawtext(numbuf, &r,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
		}
		else
			break;
	}
	r = MultiFuncZone[1];
	drawtext(_T("���سɹ�"), &r,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	setbkmode(TRANSPARENT);
	fclose(file);
	MOUSEMSG m;
	while (1)
	{
		FlushMouseMsgBuffer();
		m=GetMouseMsg();
		if (m.mkLButton)
		{
			cleardevice();
			DrawPage();
			break;
		}
	}
	if (ret<0)
	{
		printf("�����ļ�ʧ��\n");
		return ret;
	}
	
	return 0;
}

/*loop receive*/
int LoopRecv(SOCKET hServer,char *buf,int len)
{
	int total = 0;
	int ret = 0;
	while (total < len)
	{
		ret = recv(hServer, buf + total, len - total, 0);
		if (ret < 0)
			return ret;
		total = total + ret;
	}
	return 0;
}
