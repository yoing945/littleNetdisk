#include "communicate.h"
#include "gui.h"

int main(int argc, char* argv[])
{
	
	//读取文件配置信息
	char** figInfo;
	figInfo = (char**)calloc(2,sizeof(char*));
	figInfo[0] = (char*)calloc(128, sizeof(char));
	figInfo[1]= (char*)calloc(128, sizeof(char));
	FILE* fig = fopen("figure.txt","r");
	fgets(figInfo[0],128,fig);
	fgets(figInfo[1], 128, fig);
	if (figInfo[0][strlen(figInfo[0]) - 1] == '\n')
		figInfo[0][strlen(figInfo[0]) - 1] = 0;
	if (figInfo[1][strlen(figInfo[1]) - 1] == '\n')
		figInfo[1][strlen(figInfo[1]) - 1] = 0;

	fclose(fig);
	char* ipPort[] = {figInfo[0],figInfo[1]};

	initgraph(WIDTH, HEIGHT, NOCLOSE);
	setbkcolor(WHITE);
	setbkmode(TRANSPARENT);
	cleardevice();
	WSADATA wsaData;

	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		ErrorHandling("WSAStartup");
		return ret;
	}
	//连接并通信
	ret=NetToServer(ipPort);
	free(figInfo);
	WSACleanup();
	closegraph();
	return ret;
}
