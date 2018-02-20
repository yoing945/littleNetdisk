#include "communicate.h"
#include "gui.h"

inline void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	printf("error code:%d\n", GetLastError());
}
/*编码转换：Linux UTF-8转Windows GBK*/
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
/*连接到服务器*/
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
	printf("连接成功！IP:%s PORT:%s\n",ipPort[0],ipPort[1]);
	//对话
	ret=Communication(hServer);

	//关闭socket句柄
	shutdown(hServer, SD_BOTH);
	closesocket(hServer);
	return ret;
}
/*通信
	  各个命令含义
	 1 "pathname"    :cd "pathname"
	 2 "filename"    :puts "filename"
	 3 "filename"    :gets "filename"
	 4 "filename"    :remove "filename"
	 5 "direntname"  :mkdir "direntname"
	 6:ls (操作文件后自动发送)
	 7:pwd(现在的功能用不到)
	 0:exit
	 100:登陆
	 200:注册
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
		//创建用户根目录(若已存在服务器不会再创建)
		ZeroMemory(&ct, sizeof(ct));
		ct.flagFile = 5;
		sprintf(ct.buf, "%s%s", "5 ", username);
		LoopSend(hServer, (char*)&ct, sizeof(ct));
		LoopRecv(hServer,(char*)&ct,sizeof(ct));
		//cd到用户根目录
		ZeroMemory(&ct, sizeof(ct));
		ct.flagFile = 1;
		sprintf(ct.buf, "%s%s", "1 ", username);
		LoopSend(hServer, (char*)&ct, sizeof(ct));
		LoopRecv(hServer, (char*)&ct, sizeof(ct));
	}
	while (1)
	{
		ZeroMemory(&ct, sizeof(CMDTomas));
		//自动发送ls并接受回复
		ret=GetFileList(hServer);
		//GUI操作获得命令
		strcpy(ct.buf, PageEvent());
		ret=LoopSend(hServer, (char*)&ct, sizeof(CMDTomas));
		//正常退出
		if (strcmp(ct.buf, "0") == 0)
			return 1;
		//接收并处理回复
		ret=HandlngReply(hServer);
	}
exitLabel:
	return 0;
}
/*发送ls命令接收内容并存入OnePageFile*/
int GetFileList(SOCKET hServer)
{
	
	CMDTomas ct;
	int ret = 0;
	ZeroMemory(&ct, sizeof(CMDTomas));
	int count = 0;
	char filename[128] = { 0 };
	char bufsize[128] = { 0 };
	FileStat tempFile;
	//发送ls命令
	ZeroMemory(OnePageFile, sizeof(OnePageFile));
	strcpy(ct.buf, "6");
	LoopSend(hServer, (char*)&ct, sizeof(CMDTomas));
	//接收并把文件信息存入OnePageFile
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
			//如果是文件夹
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
			//由于一页最多只能显示MAX_FILE_NUM个文件，不知道咋创建滚动条
			//如果当前文件夹中文件的数量大于MAX_FILE_NUM，不显示
			if (count > MAX_FILE_NUM)
			{
				break;
			}
		}
	} while (ct.flagFile == -1);
	return 0;
}
/*处理服务器回复的消息*/
inline int HandlngReply(SOCKET hServer)
{
	int ret = 0;
	CMDTomas ct;
	char buf[sizeof(CMDTomas)];
	//flagFile=-1:需要连续回复
	do
	{
		ZeroMemory(&ct, sizeof(CMDTomas));
		ZeroMemory(buf, sizeof(buf));
		ret=LoopRecv(hServer, (char*)&ct, sizeof(CMDTomas));
		strcpy(buf, UtfToGbk(ct.buf));
		puts(buf);
	} while (ct.flagFile==-1);
	//flagFile=1:服务器准备好发送文件
	if (ct.flagFile == 1)
	{
		ret = RecvFile(hServer);
		if (ret == 0)
			puts("下载成功");
		else
			puts("下载失败");
	}
	//flagFile=2:服务器准备好接受上传的文件
	if (ct.flagFile == 2)
	{
		//本地文件不存在,通知服务器
		ret=SendFile(hServer, ct.buf);
		if (ret == 0)
			puts("上传成功");
		else
			puts("上传失败");
	}
	printf("-------------------------------------\n");
	return ret;
}
/*处理登录或者注册*/
char* RegOrLog(SOCKET hServer)
{
	int ret = RegLogClickFb();
	
	if (ret == 100)
	{
		//登录
		return GUILogin(hServer);
	}
		//注册
	return GUIRegister(hServer);

}
/*登录,返回用户名*/
char* GUILogin(SOCKET hServer)
{
	WCHAR input[128] = { 0 };
	char* username;
	CMDTomas ct;
	//发登录命令
	ZeroMemory(&ct, sizeof(ct));
	strcpy(ct.buf, "100");
	LoopSend(hServer, (char*)&ct, sizeof(ct));
	while (1)
	{
		ZeroMemory(&ct, sizeof(ct));
		ZeroMemory(input, sizeof(input));
		if (InputBox(input, 64, _T("输入用户名"), _T("登录"), NULL, 0, 0, false))
		{
			//发送用户名
			ct.flagFile = 100;
			username = ToAnsiStr(input);
			strcpy(ct.buf, username);
			LoopSend(hServer, (char*)&ct, sizeof(ct));
			ZeroMemory(&ct, sizeof(ct));
			LoopRecv(hServer, (char*)&ct, sizeof(ct));
			//用户不存在
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
		//用户点击了取消
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
		if (InputBox(input, 64, _T("输入密码（明文!）"),
			_T("登录"), NULL, 0, 0, false))
		{
			ct.flagFile = 100;
			strcpy(ct.buf, ToAnsiStr(input));
			LoopSend(hServer, (char*)&ct, sizeof(ct));
			ZeroMemory(&ct, sizeof(ct));
			LoopRecv(hServer, (char*)&ct, sizeof(ct));
			//密码错误
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
		//用户点击了取消
		else
		{
			ct.flagFile = 0;
			LoopSend(hServer, (char*)&ct, sizeof(ct));
			return NULL;
		}
	}
	return username;
}
/*注册*/
char* GUIRegister(SOCKET hServer)
{
	WCHAR input[128] = { 0 };
	char* username;
	CMDTomas ct;
	//发注册命令
	ZeroMemory(&ct, sizeof(ct));
	strcpy(ct.buf,"200");
	LoopSend(hServer,(char*)&ct,sizeof(ct));
	while (1)
	{
		ZeroMemory(&ct, sizeof(ct));
		ZeroMemory(input, sizeof(input));
		if (InputBox(input, 64, _T("输入用户名"), _T("注册"), NULL, 0, 0, false))
		{
			//发送用户名
			ct.flagFile = 200;
			username = ToAnsiStr(input);
			strcpy(ct.buf,username);
			LoopSend(hServer, (char*)&ct, sizeof(ct));
			ZeroMemory(&ct,sizeof(ct));
			LoopRecv(hServer, (char*)&ct, sizeof(ct));
			//用户名已存在
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
		//用户点击了取消
		else
		{
			ct.flagFile = 0;
			LoopSend(hServer, (char*)&ct, sizeof(ct));
			return NULL;
		}
	}//end while
	//发送密码
	ZeroMemory(&ct, sizeof(ct));
	ZeroMemory(input, sizeof(input));
	InputBox(input, 64, _T("输入密码（明文！）"),
		_T("注册"), NULL, 0, 0, true);
	strcpy(ct.buf, ToAnsiStr(input));
	LoopSend(hServer, (char*)&ct, sizeof(ct));
	return username;
}


/*发送文件*/
int SendFile(SOCKET hServer,char* filename)
{
	//获取文件大小
	unsigned _int64 size = 0;
	unsigned _int64 sum = 0;
	FILE* file = fopen(filename, "rb");
	if (!file)
	{
		printf("发送文件失败\n");
		return -1;
	}
	size = (unsigned _int64  )_filelengthi64(fileno(file));
	//用货运托马斯发货！
	Tomas tomas;
	ZeroMemory(&tomas, sizeof(Tomas));
	//发送文件名和文件大小
	strcpy(tomas.buf, filename);
	tomas.len = strlen(tomas.buf) + sizeof(unsigned _int64  );
	strcpy(tomas.buf + strlen(tomas.buf), (char*)&size);
	int ret = LoopSend(hServer, (char*)&tomas, sizeof(int)+tomas.len);
	if (ret <0)
		return ret;
	//接收并显示进度
	setfillcolor(RED);
	setfillstyle(BS_SOLID);
	setbkmode(OPAQUE);
	RECT r = MultiFuncZone[0];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	r = MultiFuncZone[1];
	drawtext(_T("正在上传"), &r,
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
	//发送len=0代表发送结束
	tomas.len = 0;
	LoopSend(hServer, (char*)&tomas, sizeof(int));
	r = MultiFuncZone[1];
	drawtext(_T("上传成功"), &r,
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
/*接收文件*/
int RecvFile(SOCKET hServer)
{
	unsigned _int64 size = 0;
	unsigned _int64 sum = 0;
	Tomas tomas;
	ZeroMemory(&tomas, sizeof(Tomas));
	//拿到文件名和文件大小
	int ret = 0;
	ret=LoopRecv(hServer, (char*)&tomas, sizeof(int));
	if (ret < 0)
	{
		printf("接收文件失败\n");
		return ret;
	}
	ret=LoopRecv(hServer, tomas.buf, tomas.len);
	if (ret < 0)
	{
		printf("接收文件失败\n");
		return ret;
	}
	char filename[128] = { 0 };
	for (int i = 0; i < tomas.len - sizeof(unsigned _int64 ); i++)
		filename[i] = tomas.buf[i];
	for (int i = 0; i < sizeof(unsigned _int64 ); i++)
		((char*)&size)[i] = tomas.buf[strlen(filename)+i];
	printf("filename:%s\n", filename);
	//准备接收文件
	FILE* file = fopen(filename,"wb+");
	if (!file)
	{
		printf("接收文件失败\n");
		return ret;
	}
	//接收并显示进度
	setfillcolor(RED);
	setfillstyle(BS_SOLID);
	setbkmode(OPAQUE);
	RECT r = MultiFuncZone[0];
	fillrectangle(r.left, r.top, r.right, r.bottom);
	r = MultiFuncZone[1];
	drawtext(_T("正在下载"), &r,
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
			//计算百分比
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
	drawtext(_T("下载成功"), &r,
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
		printf("接收文件失败\n");
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
