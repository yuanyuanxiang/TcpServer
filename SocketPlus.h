#include <winsock2.h>
#include <stdio.h>
#include <process.h>

#pragma once
#include <time.h>

#define MAX_CLIENT 100

// 线程状态标志
enum STATUS
{
	Exit = 0,
	CheckIO = 1,
	Process = 2,
	maxSta, // 线程状态数
};

// 计时器
class tick
{
public:
	inline tick() : m_start(now()) { }
	inline void start() { m_start = now(); }
	// 计时[ms]
	inline clock_t time() const { return (now() - m_start); }
	~tick() { }

private:
	clock_t m_start;
	inline clock_t now() const { return clock(); }
};

// 客户端连接
class Connect
{
private:
	int g_fd_ArrayC;
	tick m_tick;
	// 超时时间(ms)
	static int TIME_OUT;

public:
	// 设置超时时间[s]
	static void SetTimeOutTime(int nTime) { TIME_OUT = nTime * 1000; }
	inline Connect(): g_fd_ArrayC(0), m_tick() { }
	inline Connect(int Number) : g_fd_ArrayC(Number), m_tick() { }
	inline operator int() const { return g_fd_ArrayC; }
	inline bool timeout() const { return g_fd_ArrayC && m_tick.time() > TIME_OUT; }
	inline void start_tick() { m_tick.start(); }
};

// Socket ++
class SocketPlus
{
private:
	// 临界区
	CRITICAL_SECTION m_cs;

	// 所有连接
	Connect g_fd_ArrayC[MAX_CLIENT];
	// 线程状态
	int m_nStatus;
	// 客户端数
	int m_nNum;
	// 最大监听数
	int m_nMaxListen;
	// 服务端SOCKET
	SOCKET m_server;
	// FD SET
	fd_set m_fdSet;
	// 线程是否退出
	BOOL m_bExit[maxSta];

	// 进行收信处理
	static UINT __stdcall PROCESS(void* param);

	// 检测收信状态
	static UINT __stdcall CHECKIO(void* param);

public:
	SocketPlus();

	~SocketPlus();

	// 初始化
	bool Init(const char *ip, int port, int maxListen = 32);

	// Start
	void Start()
	{
		_beginthreadex(NULL, NULL, &CHECKIO, this, NULL, NULL);
		_beginthreadex(NULL, NULL, &PROCESS, this, NULL, NULL);
	}

	BOOL _Exit(int nCode = Exit)
	{
		m_nStatus = Exit;
		if (nCode == -1)
			printf(" ========== 服务已到期 ========== \n");
		return TRUE;
	}

	// 停止服务
	BOOL Stop()
	{
		// 最多等3秒
		int K = 300;
		while (m_bExit[CheckIO] == FALSE || m_bExit[Process] == FALSE && K--)
			Sleep(1);
		for (int nLoopi = 0; nLoopi < m_nMaxListen; ++nLoopi)
		{
			if (g_fd_ArrayC[nLoopi])
			{
				CloseClient(nLoopi);
			}
		}
		return K;
	}

	// 关闭客户端
	void CloseClient(int nIndex)
	{
		EnterCriticalSection(&m_cs);
		closesocket(g_fd_ArrayC[nIndex]);
		// 将已经关闭的SOCKET从FD集中删除
		FD_CLR(g_fd_ArrayC[nIndex], &m_fdSet);
		g_fd_ArrayC[nIndex] = 0;
		-- m_nNum;
		LeaveCriticalSection(&m_cs);
		printf("-当前客户端个数: %d. \n", m_nNum);
	}

	// 添加客户端
	BOOL AddClient(SOCKET client)
	{
		if (m_nNum < m_nMaxListen)
		{
			BOOL bRet = FALSE;
			for (int nLoopi = 0; nLoopi < m_nMaxListen; ++nLoopi)
			{
				if (g_fd_ArrayC[nLoopi] == 0)
				{
					// 添加新的可用连接
					EnterCriticalSection(&m_cs);
					g_fd_ArrayC[nLoopi] = client;
					g_fd_ArrayC[nLoopi].start_tick();
					++ m_nNum;
					LeaveCriticalSection(&m_cs);
					bRet = TRUE;
					break;
				}
			}
			for (int nLoopi = 0; nLoopi < m_nMaxListen; ++nLoopi)
			{
				if (g_fd_ArrayC[nLoopi].timeout())
				{
					printf("-关闭超时客户端[%d]. \n", g_fd_ArrayC[nLoopi]);
					CloseClient(nLoopi);
				}
			}
			return bRet;
		}
		return FALSE;
	}

	int GetClient(int nIndex) const { return g_fd_ArrayC[nIndex]; };

	BOOL IsRuning() const { return m_nStatus; }

	void Lock() { EnterCriticalSection(&m_cs); }

	void UnLock() { LeaveCriticalSection(&m_cs); }

	void GetFileName(const char *recv_buf, int nLen, char *pName, int taskId);
};
