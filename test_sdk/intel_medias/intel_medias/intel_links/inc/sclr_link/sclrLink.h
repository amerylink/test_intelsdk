#ifndef _SCLR_LINK_H_
#define _SCLR_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"
#include "SDK_codec.h"

#ifdef WIN32
#include <windows.h>
#define usleep Sleep
#endif

#define SCLR_LINK_MAX_ALLOC_BUFF_NUM 10
#define SCLR_LINK_DEFAULT_ALLOC_BUFF_NUM 6

/*sclr link ���ò���*/
typedef struct{
	Uint32 dstX;
	Uint32 dstY;
	Uint32 dstWidth;
	Uint32 dstHeight;
}SCLR_SET_INFO;

/*sclr link ��������*/
typedef struct{
	Uint32 dstX;
	Uint32 dstY;
	Uint32 dstWidth;
	Uint32 dstHeight;
	Uint32 queLen;
	Uint32 chNo;
}SCLR_CREATE_INFO;

/*sclr link��������Ϣ*/
typedef struct{
	Bool			tskEnable;							//��������
	Bool			exitTsk;								//�˳�����
	pthread_mutex_t lock;
	SCLR_CREATE_INFO createInfo;                     //��������
	Bool			   rtSclrInfoUpdate;
	SCLR_SET_INFO rtSclrInfo;
	sAPI_EncoderInputParams AllocFramePara;		//����frame����
	LINK_TskHndl tskHandle;							//������
	LINK_EmpFullQueHndl *PreEmpFullQueHndl;	//֡���������о��
	FRAME_Buf 			frameBuff[SCLR_LINK_MAX_ALLOC_BUFF_NUM];
	LINK_EmpFullQueHndl EmpFullQueHndl;				//���������о��
}SCLR_LINK_INFO;

Int32 sclrLink_createParaInit(SCLR_CREATE_INFO *createInfo);
Int32 sclrLink_create(SCLR_LINK_INFO *info);
Int32 sclrLink_release(SCLR_LINK_INFO *info);
Int32 sclrLink_enable(SCLR_LINK_INFO *info);
Int32 sclrLink_disable(SCLR_LINK_INFO *info);
Int32 sclrLink_setParam(SCLR_LINK_INFO *info,SCLR_SET_INFO *setPara);
Int32 sclrLink_connect(SCLR_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);
void sclrLink_printf(SCLR_LINK_INFO *info);

#endif
