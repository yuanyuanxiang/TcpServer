#include <winsock2.h>
#include <stdio.h>
#include <process.h>

#pragma once
#include <time.h>

#define MAX_CLIENT 100

// �߳�״̬��־
enum STATUS
{
	Exit = 0,
	CheckIO = 1,
	Process = 2,
	maxSta, // �߳�״̬��
};

// ��ʱ��
class tick
{
public:
	inline tick() : m_start(now()) { }
	inline void start() { m_start = now(); }
	// ��ʱ[ms]
	inline clock_t time() const { return (now() - m_start); }
	~tick() { }

private:
	clock_t m_start;
	inline clock_t now() const { return clock(); }
};

// �ͻ�������
class Connect
{
private:
	int g_fd_ArrayC;
	tick m_tick;
	// ��ʱʱ��(ms)
	static int TIME_OUT;

public:
	// ���ó�ʱʱ��[s]
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
	// �ٽ���
	CRITICAL_SECTION m_cs;

	// ��������
	Connect g_fd_ArrayC[MAX_CLIENT];
	// �߳�״̬
	int m_nStatus;
	// �ͻ�����
	int m_nNum;
	// ��������
	int m_nMaxListen;
	// �����SOCKET
	SOCKET m_server;
	// FD SET
	fd_set m_fdSet;
	// �߳��Ƿ��˳�
	BOOL m_bExit[maxSta];

	// �������Ŵ���
	static UINT __stdcall PROCESS(void* param);

	// �������״̬
	static UINT __stdcall CHECKIO(void* param);

public:
	SocketPlus();

	~SocketPlus();

	// ��ʼ��
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
			printf(" ========== �����ѵ��� ========== \n");
		return TRUE;
	}

	// ֹͣ����
	BOOL Stop()
	{
		// ����3��
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

	// �رտͻ���
	void CloseClient(int nIndex)
	{
		EnterCriticalSection(&m_cs);
		closesocket(g_fd_ArrayC[nIndex]);
		// ���Ѿ��رյ�SOCKET��FD����ɾ��
		FD_CLR(g_fd_ArrayC[nIndex], &m_fdSet);
		g_fd_ArrayC[nIndex] = 0;
		-- m_nNum;
		LeaveCriticalSection(&m_cs);
		printf("-��ǰ�ͻ��˸���: %d. \n", m_nNum);
	}

	// ��ӿͻ���
	BOOL AddClient(SOCKET client)
	{
		if (m_nNum < m_nMaxListen)
		{
			BOOL bRet = FALSE;
			for (int nLoopi = 0; nLoopi < m_nMaxListen; ++nLoopi)
			{
				if (g_fd_ArrayC[nLoopi] == 0)
				{
					// ����µĿ�������
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
					printf("-�رճ�ʱ�ͻ���[%d]. \n", g_fd_ArrayC[nLoopi]);
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
