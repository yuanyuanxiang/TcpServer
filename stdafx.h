// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
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

/// Ĭ��ͨ�Ŷ˿�
#define PORT 9007

/// Ĭ�ϻ�������С
#define DEFAULT_BUFFER 1024

/// ����ͬʱ���ӵĿͻ�������
#define MAX_LISTEN 32

/// ��ȡ����ip��by Ԭ����,2016/11/28��
inline const char* GetLocalHost()
{
	char hostname[128] = { 0 }, *hostip = 0;
	static char localhost[128] = { 0 };
	if (0 == gethostname(hostname, 128))
	{
		hostent *host = gethostbyname(hostname);
		// ��ipת��Ϊ�ַ���
		hostip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
		strcpy(localhost, hostip);
	}
	else
	{
		return "127.0.0.1";
	}

	return localhost;
}
