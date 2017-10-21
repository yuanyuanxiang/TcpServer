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
* @brief 错误码枚举类型
*/
enum ErrorCode
{
	DISABLED_QR = 0,				// 未启用解码功能
	DOWNLOAD_IMAGE_FAILED = 1,		// 下载图片失败
	LOAD_IMAGE_FAILED = 2,			// 加载图片失败
	DECODE_FAILED = 3,				// 解码图片失败
};

/**
* @brief 构造一个任务
* @param[in] id 任务编号
*/
CMyTask::CMyTask(SocketPlus *socketplus, int nIndex, int id) : CTask(id),
	m_pServerSocket(socketplus), m_nIndex(nIndex)
{
}

/// 默认析构函数
CMyTask::~CMyTask(void)
{
}

/// delete this
void CMyTask::Destroy()
{
	delete this;
}

/// CMyTask重载的任务执行函数
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
			第1个参数:仅当调用者是一个ActiveX对象才使用,一般为NULL
			第2个参数:就是要下载文件的目标URL,完整路径
			第3个参数:本地保存路径,也是完整路径
			第4个参数:保留,必须为0
			第5个参数:一种回调机制,可以参考这些来活动当前下载进度
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
						printf("-Error[-1]: 解码图片%s出错. \n", fileName);
					}
				}
				else
				{
					sprintf_s(dec_buf, "%d", LOAD_IMAGE_FAILED);
					printf("-Error[0x%x]: 加载图片%s出错. \n", HR, fileName);
				}

			}
			else
			{
				sprintf_s(dec_buf, "%d", DOWNLOAD_IMAGE_FAILED);
				printf("-Error[0x%x]: 下载图片出错. \n", hr);
			}
#endif
			/* 向客户端发送响应数据 */
			send(client, dec_buf, strlen(dec_buf), 0);
		}
	}
}
