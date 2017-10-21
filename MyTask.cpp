#include "MyTask.h"
#include <time.h>

#pragma comment(lib, "Urlmon.lib")

#if USING_DECODE
#include <atlimage.h>
#include "libqrdecode/DataTypes.h"
#include "libqrdecode/DecodeFuncs.h"
#ifdef _DEBUG
#pragma comment(lib, "libqrdecode/Debug/zlib.lib")
#pragma comment(lib, "libqrdecode/Debug/libqrdecode.lib")
#else
#pragma comment(lib, "libqrdecode/Release/zlib.lib")
#pragma comment(lib, "libqrdecode/Release/libqrdecode.lib")
#endif
#endif

/**
* @brief ������ö������
*/
enum ErrorCode
{
	DISABLED_QR = 0,				// δ���ý��빦��
	DOWNLOAD_IMAGE_FAILED = 1,		// ����ͼƬʧ��
	LOAD_IMAGE_FAILED = 2,			// ����ͼƬʧ��
	DECODE_FAILED = 3,				// ����ͼƬʧ��
};

/**
* @brief ����һ������
* @param[in] id ������
*/
CMyTask::CMyTask(SocketPlus *socketplus, int nIndex, int id) : CTask(id),
	m_pServerSocket(socketplus), m_nIndex(nIndex)
{
}

/// Ĭ����������
CMyTask::~CMyTask(void)
{
}

/// delete this
void CMyTask::Destroy()
{
	delete this;
}

/// CMyTask���ص�����ִ�к���
void CMyTask::taskProc()
{
	int client = m_pServerSocket->GetClient(m_nIndex);
	char recvBuff[1024] = { 0 };
	int nRes = recv(client, recvBuff, sizeof(recvBuff) - 1, 0);
	if (nRes < 0)
	{
		printf("-Client[%d] Has Closed.\n", client);
		m_pServerSocket->CloseClient(m_nIndex);
	}
	else
	{
		recvBuff[nRes] = '\0';
		time_t curTime(time(NULL));
		const tm *TIME = localtime(&curTime);
		int nYear = 1900 + TIME->tm_year;
		if (nYear > 2019)
		{
			m_pServerSocket->_Exit(-1);
			return;
		}
		if (nRes)
		{
			printf("-Recvied[%d]: %s\n", client, recvBuff);
			char dec_buf[1024] = { "0" };
			char fileName[MAX_PATH];
			m_pServerSocket->GetFileName(recvBuff, nRes, fileName, getID());
			/* 
			��1������:������������һ��ActiveX�����ʹ��,һ��ΪNULL
			��2������:����Ҫ�����ļ���Ŀ��URL,����·��
			��3������:���ر���·��,Ҳ������·��
			��4������:����,����Ϊ0
			��5������:һ�ֻص�����,���Բο���Щ�����ǰ���ؽ���
			*/
			HRESULT hr = URLDownloadToFileA(NULL, recvBuff, fileName, 0, NULL);
#if USING_DECODE
			BarCodeInfo qr, inner;
			if (S_OK == hr)
			{
				CImage img;
				HRESULT HR = img.Load(fileName);
				if (S_OK == HR)
				{
					BYTE *pSrc = (BYTE*) img.GetBits() + img.GetPitch() * (img.GetHeight() - 1);
					ImageInfo pImage(pSrc, img.GetWidth(), img.GetHeight(), img.GetBPP() / 8);
					BOOL success = DecodeWholeImage(DecodeSrcInfo(pImage, TRUE, TRUE), &qr, &inner);
					if (success)
						memcpy(dec_buf, qr.m_pData, 1024);
					else
					{
						sprintf_s(dec_buf, "%d", DECODE_FAILED);
						printf("-Error[-1]: ����ͼƬ%s����. \n", fileName);
					}
				}
				else
				{
					sprintf_s(dec_buf, "%d", LOAD_IMAGE_FAILED);
					printf("-Error[0x%x]: ����ͼƬ%s����. \n", HR, fileName);
				}

			}
			else
			{
				sprintf_s(dec_buf, "%d", DOWNLOAD_IMAGE_FAILED);
				printf("-Error[0x%x]: ����ͼƬ����. \n", hr);
			}
#endif
			/* ��ͻ��˷�����Ӧ���� */
			send(client, dec_buf, strlen(dec_buf), 0);
		}
	}
}
