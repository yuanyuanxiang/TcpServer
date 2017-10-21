// client.cpp : 定义控制台应用程序的入口点。
//

/** Copyright notice \n
* Copyright (c) 2016, BCECR
* All rights reserved.
*
* @file		server.cpp
* @brief	PROJECT_NAME 应用程序的主程序
*
* @version	1.0
* @date		2016/9/28
* @author	wanghongying
* @email	wanghy@bcecr.com
*/


#include "stdafx.h"

#ifdef _WIN32

#include <process.h>
#include "confReader.h"

/// 发消息线程的参数
struct PARAM
{
	SOCKET *socket;
	fd_set *fdWrite;
	BOOL *Start;
	PARAM(SOCKET *s, fd_set *f, BOOL *start) : 
		socket(s), fdWrite(f), Start(start){ }
};

/// 发送消息的子线程
unsigned int __stdcall SendBuffer(void *param)
{
	PARAM *arguments = (PARAM*)param;
	SOCKET *pSocket = arguments->socket;
	BOOL *bStart = arguments->Start;
	char sendBuff[DEFAULT_BUFFER];
	printf(">>");
	while (true)
	{
		// 等待主线程命令
		while (FALSE == bStart)
			Sleep(100);
		if (FD_ISSET(*pSocket, arguments->fdWrite))
		{
			fflush(stdin);
			memset(sendBuff, 0, sizeof(sendBuff));
			fgets(sendBuff, sizeof(sendBuff) - 1, stdin);
			int nInputLen = strlen(sendBuff);
			if(1 == nInputLen)// 直接回车，未输入
				continue;
			sendBuff[nInputLen - 1] = '\0';
			--nInputLen;
			int nIndex = 0;
			int nLeft = nInputLen;
			if (strcmp(sendBuff, "Quit") == 0)
			{
				ExitProcess(0xDEADDEAD);
			}
			// 将输入的数据发送过去
			while (nLeft > 0)
			{
				int nError = send(*pSocket, &sendBuff[nIndex], nLeft, 0);
				if (nError == SOCKET_ERROR)
				{
					printf("发送数据失败，错误码：%d.\n", WSAGetLastError());
					closesocket(*pSocket);
					WSACleanup();
					system("pause");
					return -1;
				}
				else if (nError == 0)
				{
					printf("Send OK.\n");
					break;
				}
				nLeft -= nError;
				nIndex += nError;
			}
		}
	}
	return 0xDEAD;
}

/**
* @fn main
* @brief TCP Client
* @details 使用方法： TcpClient.exe
*/
int main()
{
	int nError;
	printf(">>>>>> TCP客户端启动 <<<<<<\n");
	WSADATA wsData;
	WSAStartup(MAKEWORD(2,2), &wsData);

	const char* localhost = GetLocalHost();
	confReader reader("settings.ini");
	reader.setSection("client");
	string sIP = reader.readStr("server", localhost);

	// 服务器IP
	const char *ServerIP = sIP.c_str();
	// 端口号
	int nPort = reader.readInt("port", PORT);
	// 是否绑定
	BOOL Bind = reader.readInt("bind", TRUE);

	SOCKET sServer;
	sockaddr_in addrServer;
	sockaddr_in addrClient;
	char sendBuff[DEFAULT_BUFFER] = {0};
	char recvBuff[DEFAULT_BUFFER] = {0};

	printf("-创建客户端用SOCKET - ");
	sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sServer == INVALID_SOCKET)
	{
		printf("创建socket失败，错误码：%d.\n", WSAGetLastError());
		WSACleanup();
		system("pause");
		return -1;
	}
	printf("OK.\n");

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = inet_addr(ServerIP);
	addrServer.sin_port = htons(nPort);
	printf("-设定服务器地址信息:%s:%d.\n",
		inet_ntoa(addrServer.sin_addr), ntohs(addrServer.sin_port));
	// 设定本地用地址和端口
	addrClient.sin_family = AF_INET;

	// 获取本机ip（by 袁沅祥,2016/11/28）
	sIP = reader.readStr("client", localhost);
	addrClient.sin_addr.S_un.S_addr = inet_addr(sIP.c_str());
	addrClient.sin_port = htons(nPort);
	if (TRUE == Bind)
	{
		printf("-绑定本地用地址和端口:%s:%d - ",
			inet_ntoa(addrClient.sin_addr), ntohs(addrClient.sin_port));
		nError = bind(sServer, (sockaddr *)&addrClient, sizeof(addrClient));
		if (nError == SOCKET_ERROR)
		{
			printf("绑定端口失败，错误码：%d.\n", WSAGetLastError());
			WSACleanup();
			system("pause");
			return -1;
		}
		printf("OK.\n");
	}

	printf("-连接指定服务器 - ");
	nError = connect(sServer, (const sockaddr*)&addrServer, sizeof(addrServer));
	if (nError == SOCKET_ERROR)
	{
		printf("Failed! Last error code %d.\n", WSAGetLastError());
		closesocket(sServer);
		WSACleanup();
		system("pause");
		return -1;
	}
	printf("OK.\n");

	//////////////////////////////////////////////////////////////////////////
	//客户端SOCKET为非阻塞模式
	//
	//////////////////////////////////////////////////////////////////////////
	DWORD nMode = 1;
	nError = ioctlsocket(sServer, FIONBIO, &nMode);
	if (nError == SOCKET_ERROR)
	{
		printf("函数 ioctlsocket 失败，错误码：%d.\n", WSAGetLastError());
		closesocket(sServer);
		WSACleanup();
		system("pause");
		return -1;
	}

	printf("-SOCKET模式设定:%s.\n",(nMode == 0 ? "阻塞模式设定" : "非阻塞模式设定"));

	printf("-开始准备送信受信.\n");
	int nInputLen = 0;
	int nIndex= 0;
	int nLeft = 0;
	fd_set fdRead;
	fd_set fdWrite;
	timeval tv = {10, 0};
	// 2017-4-22 开启发消息线程
	BOOL start = FALSE;
	PARAM arg(&sServer, &fdWrite, &start);
	_beginthreadex(0, 0, &SendBuffer, &arg, 0, 0);
	while (true)
	{
		start = FALSE;
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_SET(sServer,&fdRead);
		FD_SET(sServer,&fdWrite);
		nError = select(0, &fdRead, &fdWrite, NULL, &tv);
		if (nError == 0)
		{
			// 超时,继续
			continue;
		}
		else if (nError < 0)
		{
			printf("函数 select 失败，错误码：%d.\n", WSAGetLastError());
			break;
		}
		// 发现SOCKET可读可写
		if (FD_ISSET(sServer, &fdRead))
		{
			memset(recvBuff, 0, sizeof(recvBuff));
			nError = recv(sServer, recvBuff, sizeof(recvBuff) - 1, 0);
			if (nError == SOCKET_ERROR)
			{
				printf("The Server may have closed.\n");
				break;
			}
			else if (nError == 0)
			{
				printf("Server Has Closed.\n");
				break;
			}
			recvBuff[nError] = '\0';
			printf("-Received: %s\n>>", recvBuff);
		}
		Sleep(10);
		start = TRUE;
	}//while(true)
	printf("-关闭SOCKET.\n");
	closesocket(sServer);
	WSACleanup();
	Sleep(3000);

	return 0;
}

#else 

int main(int argc, char** argv)
{
	int sockfd;
	char buffer[DEFAULT_BUFFER];
	struct sockaddr_in server_addr;

	// Server IP
	const char *ServerIP = "127.0.0.1";
	// 端口号
	int nPort = PORT;

	// 用户输入参数
	if(argc == 3)
	{
		ServerIP = argv[1];
		nPort = atoi(argv[2]);
	}

	fprintf(stderr, "Server: %s::%d.\n", ServerIP, nPort);

	struct hostent* host = gethostbyname(ServerIP);

	//AF_INET: Internet; SOCK_STREAM:TCP
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Socket Error: %s.\n", strerror(errno));
		exit(1);
	}

	bzero(&server_addr, sizeof(server_addr));
	// 初始化，置0
	server_addr.sin_family = AF_INET; //IPV4
	// 将本机上的short数据转化为网络上的short数据
	server_addr.sin_port = htons(nPort);
	// IP地址
	server_addr.sin_addr = *((struct in_addr*)host->h_addr);

	if(connect(sockfd, (struct sockaddr*)(&server_addr),
		sizeof(struct sockaddr)) == -1)
	{
		fprintf(stderr, "Connect Error: %s.\n", strerror(errno));
		exit(1);
	}

	while(1)
	{
		printf(">>");
		fgets(buffer, DEFAULT_BUFFER, stdin);
		write(sockfd, buffer, strlen(buffer));
	}
	close(sockfd);
	exit(0);
}

#endif // _WIN32
