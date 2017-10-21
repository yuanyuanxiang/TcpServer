// server.cpp : 定义控制台应用程序的入口点.
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

#include "SocketPlus.h"
#include "confReader.h"
#include "ThreadpoolLib\MyThreadPool.h"

// Socket ++
SocketPlus g_Socket;

// Thread Pool
CMyThreadPool *g_pMyPool = NULL;

int RUN = 1;

BOOL WINAPI callback(DWORD CtrlType)
{ 
	if(CTRL_CLOSE_EVENT == CtrlType)
	{
		g_Socket._Exit();
		while (RUN)
			Sleep(1);
	}
	return FALSE;
}

int main()
{
	SetConsoleCtrlHandler(&callback, TRUE);
	confReader reader("settings.ini");
	reader.setSection("server");
	// IP
	const char *hostip = GetLocalHost();
	string sIP = reader.readStr("server", hostip);
	// 端口
	int nPort = reader.readInt("port", PORT);
	// 最大监听数
	int maxListen = reader.readInt("listen", MAX_LISTEN);
	if(!g_Socket.Init(sIP.c_str(), nPort, min(maxListen, MAX_CLIENT)))
		return -1;
	// 超时时间(s)
	int Time = reader.readInt("timeout", 10);
	Connect::SetTimeOutTime(Time);

	// 初始化线程池
	g_pMyPool = new CMyThreadPool(max(2, maxListen / 4));

	RUN = TRUE;
	g_Socket.Start();
	while (g_Socket.IsRuning())
		Sleep(20);
	g_Socket.Stop();

	printf("-关闭服务器端SOCKET.\n");
	OutputDebugStringA("MSG | 关闭服务器端SOCKET.\n");

	g_pMyPool->destroyThreadPool();
	delete g_pMyPool;
	OutputDebugStringA("MSG | 线程池已销毁完毕.\n");

	RUN = FALSE;
	return 0;
}

#else

int main(int argc, char** argv)
{
	int listen_fd, accept_fd;
	struct sockaddr_in client_addr;
	int n;

	int nPort = PORT;
	if (argc >= 2)
		nPort = atoi(argv[1]);

	// 创建SOCKET
	if( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Socket Error: %s.\n", strerror(errno));
		exit(1);
	}

	bzero(&client_addr, sizeof(struct sockaddr_in));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(nPort);
	client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	n = 1;

	// 绑定
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));
	if(bind(listen_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0)
	{
		printf("Bind error: %s.\n", strerror(errno));
		exit(1);
	}

	// 监听
	listen(listen_fd, 5);

	//等待客户端连接
	while(1)
	{
		accept_fd = accept(listen_fd, NULL, NULL);
		if((accept_fd < 0) && (errno == EINTR))
			continue;
		else if(accept_fd < 0)
		{
			printf("Accept Error: %s.\n", strerror(errno));
			continue;
		}

		if((n = fork()) == 0)
		{
			// 子进程
			char buffer[DEFAULT_BUFFER] = { 0 };

			close(listen_fd);

			while(read(accept_fd, buffer, DEFAULT_BUFFER) != 0 )
			{
				if (buffer[0] == '\n')
					continue;
				printf("Receive: %s\n", buffer);
				write(accept_fd, buffer, n);
				bzero(buffer, sizeof(buffer));
			}
			close(accept_fd);
			exit(0);
		}
		else if(n < 0)
		{
			printf("Fork Error: %s.\n", strerror(errno));
		}
		close(accept_fd);
	}

	return 0;
}

#endif // _WIN32
