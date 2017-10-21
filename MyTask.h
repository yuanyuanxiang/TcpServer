/** 
* @file MyTask.h
* @brief ����ʵ��
*/

#pragma once
#include "ThreadpoolLib/task.h"
#include "SocketPlus.h"

/**
* @class CMyTask 
* @brief ��ĿThreadPool������ʵ��
*/
class CMyTask : public CTask
{
public:
	CMyTask(SocketPlus *socketplus, int nIndex, int id);
protected:
	~CMyTask(void);
	SocketPlus *m_pServerSocket;
	int m_nIndex;

public:
	_inline virtual void Destroy();
	virtual void taskProc();
};
