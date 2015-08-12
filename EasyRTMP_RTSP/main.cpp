/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
#define _CRTDBG_MAP_ALLOC
#include <stdio.h>
#include "EasyRTMPAPI.h"
#include "EasyRTSPClientAPI.h"
#include "windows.h"

#define RTSPURL "rtsp://admin:admin@192.168.1.108/cam/realmonitor?channel=1&subtype=1"

#define SRTMP "rtmp://www.easydss.com/oflaDemo/rtsp"

Easy_RTMP_Handle rtmpHandle = 0;
Easy_RTSP_Handle fNVSHandle = 0;

/* NVSource��RTSPClient��ȡ���ݺ�ص����ϲ� */
int Easy_APICALL __RTSPSourceCallBack( int _chid, int *_chPtr, int _mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo)
{
	if (NULL != frameinfo)
	{
		if (frameinfo->height==1088)		frameinfo->height=1080;
		else if (frameinfo->height==544)	frameinfo->height=540;
	}
	bool bRet = false;

	//Ŀǰֻ������Ƶ
	if (_mediatype == MEDIA_TYPE_VIDEO)
	{
		if(frameinfo && frameinfo->length)
		{
			if( frameinfo->type == FRAMETYPE_I)
			{
				/* �ؼ�֡��SPS��PPS��IDR(������00 00 00 01)�����,reserved1��sps��β��ƫ��,reserved2��pps��β��ƫ�� */
				if(rtmpHandle == 0)
				{
					rtmpHandle = EasyRTMP_Session_Create();

					bRet = EasyRTMP_Connect(rtmpHandle, SRTMP);
					if (!bRet)
					{
						printf("Fail to EasyRTMP_Connect ...\n");
					}

					bRet = EasyRTMP_InitMetadata(rtmpHandle, (const char*)pbuf+4,
						frameinfo->reserved1-4, (const char*)pbuf+frameinfo->reserved1+4 ,
						frameinfo->reserved2 - frameinfo->reserved1 -4, 25, 8000);
					if (bRet)
					{
						printf("Fail to InitMetadata ...\n");
					}
				}
				bRet = EasyRTMP_SendH264Packet(rtmpHandle, (unsigned char*)pbuf+frameinfo->reserved2+4, frameinfo->length-frameinfo->reserved2-4, true, (frameinfo->rtptimestamp)/90);
				if (!bRet)
				{
					printf("Fail to EasyRTMP_SendH264Packet(I-frame) ...\n");
				}
				else
				{
					printf("I");
				}
			}
			else
			{
				if(rtmpHandle)
				{
					bRet = EasyRTMP_SendH264Packet(rtmpHandle, (unsigned char*)pbuf+4, frameinfo->length-4, false, (frameinfo->rtptimestamp)/90);
					if (!bRet)
					{
						printf("Fail to EasyRTMP_SendH264Packet(P-frame) ...\n");
					}
					else
					{
						printf("P");
					}
				}
			}				
		}	
	}
	return 0;
}

int main()
{
	//����EasyNVSource
	EasyRTSP_Init(&fNVSHandle);
	if (NULL == fNVSHandle) return 0;

	unsigned int mediaType = MEDIA_TYPE_VIDEO;
	//mediaType |= MEDIA_TYPE_AUDIO;	//��ΪNVSource, ��������

	//�������ݻص�
	EasyRTSP_SetCallback(fNVSHandle, __RTSPSourceCallBack);
	//��RTSP���紮��
	EasyRTSP_OpenStream(fNVSHandle, 0, RTSPURL, RTP_OVER_TCP, mediaType, 0, 0, NULL, 1000, 0);

	while(1)
	{
		Sleep(10);	
	};

	//�Ƿ�RTMP����
    EasyRTMP_Session_Release(rtmpHandle);
    rtmpHandle = 0;
   
	//�ر�EasyNVSource��ȡ
	EasyRTSP_CloseStream(fNVSHandle);
	//�ͷ�EasyNVSource
	EasyRTSP_Deinit(&fNVSHandle);
	fNVSHandle = NULL;

    return 0;
}