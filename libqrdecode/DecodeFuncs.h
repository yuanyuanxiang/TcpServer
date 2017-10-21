/**
* @file DecodeFuncs.h
* @brief ��ɫ��ά����뺯����ANDROID����
*/

#include "ImageSrc.h"


/** 
* @brief ��������ά��ͼ������Դ���н���
* @remark �˺����ǽ����ɫ��ά���Ψһ�ӿ�
* @param[in]	&pSrc	��ά���������Դ
* @param[out]	*qr		QR��ά����Ϣ
* @param[out]	*inner	��ɫ��ά����Ϣ
* @return �ɹ���ʧ��. ����QR��ɹ�����1�������ɫ�ɹ�����2.
*/
BOOL DecodeWholeImage(const DecodeSrcInfo &pSrc, BarCodeInfo *qr, BarCodeInfo *inner);
