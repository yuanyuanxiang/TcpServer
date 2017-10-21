// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#ifdef _WIN32 // Windows

#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>

#include "Winsock2.h"
#pragma comment(lib, "ws2_32.lib")

#else // LINUX

#include "stdlib.h"
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "netdb.h"
#include "unistd.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "sys/socket.h"

#endif

//////////////////////////////////////////////////////////////////////////
// defines

/// 默认通信端口
#define PORT 9007

/// 默认缓冲区大小
#define DEFAULT_BUFFER 1024

/// 最多可同时连接的客户端数量
#define MAX_LISTEN 32

/// 获取本机ip（by 袁沅祥,2016/11/28）
inline const char* GetLocalHost()
{
	char hostname[128] = { 0 }, *hostip = 0;
	static char localhost[128] = { 0 };
	if (0 == gethostname(hostname, 128))
	{
		hostent *host = gethostbyname(hostname);
		// 将ip转换为字符串
		hostip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
		strcpy(localhost, hostip);
	}
	else
	{
		return "127.0.0.1";
	}

	return localhost;
}
