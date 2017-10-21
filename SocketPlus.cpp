#include "SocketPlus.h"
#include "ThreadpoolLib\MyThreadPool.h"
#include "MyTask.h"

extern CMyThreadPool *g_pMyPool;

int Connect::TIME_OUT = 10 * 1000;

/**
* @brief ��ȡ����ͼ�������
* @param[in] recv_buf ����ͼ���ַhttp
* @param[in] nLen recv_buf����
* @param[in] pName ����ͼ������
* @param[in] taskId ����Id
*/
void SocketPlus::GetFileName(const char *recv_buf, int nLen, char *pName, int taskId)
{
	const char *p = recv_buf;
	int n = nLen, maxLen = 6;
	while(*p) ++p;
	while(*p != '.' && --n && --maxLen) --p;
	++p;
	sprintf(pName, "download%03d.%s", taskId+1, maxLen && n ? p : "jpeg");
}


SocketPlus::SocketPlus() : m_nStatus(CheckIO), m_nNum(0), m_nMaxListen(10), 
	m_server(INVALID_SOCKET)
{
	//memset(g_fd_ArrayC, 0, MAX_CLIENT * sizeof(int));
	memset(&m_fdSet, 0, sizeof(fd_set));
	m_bExit[CheckIO] = m_bExit[Process] = TRUE;
	InitializeCriticalSection(&m_cs);
	WSADATA wsData;
	WSAStartup(MAKEWORD(2,2), &wsData);
}


SocketPlus::~SocketPlus()
{
	if (INVALID_SOCKET == m_server)
		closesocket(m_server);
	WSACleanup();
	DeleteCriticalSection(&m_cs);
}


bool SocketPlus::Init(const char *ip, int port, int maxListen)
{
	m_nMaxListen = maxListen;
	printf(">>>>>> TCP������������ <<<<<< \n");

	printf("-����һ��SOCKET - ");
	m_server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (SOCKET_ERROR == m_server)
	{
		printf("Error��%d.\n", WSAGetLastError());
		system("pause");
		return false;
	}
	printf("OK.\n");

	printf("-�趨�����������˿�: %d.\n", port);
	sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = inet_addr(ip);
	addrServer.sin_port = htons(port);
	printf("-��SOCKET��ָ�������˿ڣ�%s:%d.\n",
		inet_ntoa(addrServer.sin_addr), ntohs(addrServer.sin_port));
	if (SOCKET_ERROR == bind(m_server,(const sockaddr*)&addrServer, sizeof(addrServer)))
	{
		printf("�󶨶˿�ʧ�ܣ������룺%d.\n", WSAGetLastError());
		system("pause");
		return false;
	}
	printf("-�����˿� - ");
	if (listen(m_server, maxListen) == SOCKET_ERROR)
	{
		printf("Error��%d.\n", WSAGetLastError());
		system("pause");
		return false;
	}
	printf("OK.\n");

	//////////////////////////////////////////////////////////////////////////
	//������ģʽ�趨
	//
	//////////////////////////////////////////////////////////////////////////
	DWORD nMode = 1;
	if (SOCKET_ERROR == ioctlsocket(m_server, FIONBIO, &nMode))
	{
		printf("���� ioctlsocket ʧ�ܣ������룺%d.\n",WSAGetLastError());
		system("pause");
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	printf("-���÷�������ģʽ:%s.\n", nMode == 0 ? "����ģʽ" : "������ģʽ");
	printf("-��ʼ׼����������...\n");
	return true;
}


UINT __stdcall SocketPlus::PROCESS(void* param)
{
	SocketPlus *pThis = (SocketPlus *)param;
	pThis->m_bExit[Process] = FALSE;
	OutputDebugStringA("MSG | Process �߳�������. \n");
	do
	{
		if (pThis->m_nStatus == CheckIO)
		{
			Sleep(10);
			continue;
		}
		pThis->m_nStatus = Process;
		for (int nLoopi = 0; nLoopi < pThis->m_nMaxListen; ++nLoopi)
		{
			if (FD_ISSET(pThis->g_fd_ArrayC[nLoopi], &pThis->m_fdSet))
			{
				g_pMyPool->addTask(new CMyTask(pThis, nLoopi, nLoopi), PRIORITY::NORMAL);
			}
		}
		while (g_pMyPool->GetActiveThreadNum())
			Sleep(10);
		pThis->m_nStatus = CheckIO;
	}while(pThis->m_nStatus);
	OutputDebugStringA("MSG | Process �߳����˳�. \n");
	pThis->m_bExit[Process] = TRUE;
	return 0xdead01;
}


UINT __stdcall SocketPlus::CHECKIO(void* param)
{
	SocketPlus *pThis = (SocketPlus *)param;
	SOCKET sServer = pThis->m_server;
	fd_set *fdRead = &pThis->m_fdSet;
	timeval tv = {1, 0};
	pThis->m_bExit[CheckIO] = FALSE;
	OutputDebugStringA("MSG | CheckIO �߳�������. \n");
	do
	{
		if (pThis->m_nStatus == Process)
		{
			Sleep(10);
			continue;
		}
		pThis->m_nStatus = CheckIO;
		FD_ZERO(fdRead);
		FD_SET(sServer, fdRead);
		// ������������SOCKET����fdRead���н���Dslect����
		for (int nLoopi = 0; nLoopi < pThis->m_nMaxListen; ++nLoopi)
		{
			if (pThis->g_fd_ArrayC[nLoopi] != 0)
			{
				FD_SET(pThis->g_fd_ArrayC[nLoopi], fdRead);
			}
		}
		// ����selectģʽ���м���
		int nRes = select(0, fdRead, NULL, NULL, &tv);
		if (0 == nRes)
		{
			// ��ʱ������
			continue;
		}
		else if (nRes < 0)
		{
			printf("���� select ʧ�ܣ������룺%d.\n", WSAGetLastError());
			system("pause");
			pThis->_Exit();
			continue;
		}
		// ����Ƿ����µ����ӽ���
		if (FD_ISSET(sServer, fdRead))
		{
			sockaddr_in addrClient;
			int addrClientLen = sizeof(addrClient);
			SOCKET sClient = accept(sServer, (sockaddr*)&addrClient, &addrClientLen);
			printf("-����һ���µĿͻ�����[%d]: %s:%d.\n", 
				sClient, inet_ntoa(addrClient.sin_addr), ntohs(addrClient.sin_port));
			if (sClient == WSAEWOULDBLOCK)
			{
				printf("������ģʽ�趨accept���ò���ȷ.\n");
				continue;
			}
			else if (sClient == INVALID_SOCKET)
			{
				printf("���� accept ʧ�ܣ������룺%d.\n",WSAGetLastError());
				continue;
			}
			// ���һ���ͻ���
			if (!pThis->AddClient(sClient))
			{
				char noresponseBuff[128] = {"���������������������޷�����.\n"};
				printf(noresponseBuff);
				send(sClient, noresponseBuff, strlen(noresponseBuff), 0);
				closesocket(sClient);
			}
		}//if (FD_ISSET(sListen, fdRead))
		pThis->m_nStatus = Process;
	}while(pThis->m_nStatus);
	OutputDebugStringA("MSG | CheckIO �߳����˳�. \n");
	pThis->m_bExit[CheckIO] = TRUE;
	return 0xdead02;
}
