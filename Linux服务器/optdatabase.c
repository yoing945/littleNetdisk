#include "head.h"

/*查询用户：成功返回密文,失败返回NULL*/
char* SearchUser(char* tablename,char* username)
{
	MYSQL* conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="123";
	char* database="yao_netdisk";
	char query[512]="select * from ";
	sprintf(query,"%s%s%s%s%s",query,tablename," where username='",username,"'");
	//连接数据库
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		printf("连接数据库失败\n");
		return NULL;
	}
	else
	{
		printf("连接数据库成功\n");
	}
	//请求
	int ret=mysql_query(conn,query);
	if(ret)
	{
		printf("命令不正确\n");
		return NULL;
	}
	else
	{
		res=mysql_use_result(conn);
		if(res)
		{
			int i;
			row=mysql_fetch_row(res);
			if(row)
			{
				for(i=0;i<mysql_num_fields(res);i++);
				printf("\n");
				return row[i-1];
			}
		}//end if
		mysql_free_result(res);
	}
	mysql_close(conn);
	return NULL;
}
/*插入操作*/
int InsertIntoDB(char* tablename,char* username,char* upwd)
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="123";
	char* database="yao_netdisk";
	char query[512]="insert into ";
	sprintf(query,"%s%s%s%s%s%s%s",query,tablename,"(username,pwd) values('",username,"','",upwd,"')");
	//连接数据库
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		printf("Connect to database failed\n");
		return -1;
	}
	else
		printf("Connect sucess\n");
	//插入请求
	int ret=mysql_query(conn,query);
	if(ret)
	{
		printf("Error:%s\n",mysql_error(conn));
		return -1;
	}
	else
		printf("insert success\n");
	mysql_close(conn);
	return 0;
}
