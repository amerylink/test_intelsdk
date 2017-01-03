#ifndef _NULL_SRC_LINK_H_
#define _NULL_SRC_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"
#include "SDK_codec.h"

#define NULLSRC_LINK_MAX_ALLOC_BUFF_NUM 10
#define NULLSRC_LINK_DEFAULT_ALLOC_BUFF_NUM 6
#define NULLSRC_LINK_MSG_NUM					10

/*nullsrc link ��������*/
typedef struct
{
	Uint32 width;
	Uint32 height;
	Uint32 buffNum;
	Uint32 timerPeriod;/*���ڴ���ʱ��ms*/
	Uint32 chNo;
} NULLSRC_LINK_CREATE_INFO;

// enum
// {
//     SIGNAL,
//     NO_SIGNAL,
// } SignalState;

struct buffer
{
	void *                  start;
	size_t                  length;
};

typedef struct
{
	Uint32 DevesId;
	Uint32 width;
	Uint32 height;
	Uint32 videoMode;
	Uint32 RtWidth;
	Uint32 RtHeight;
	Uint32 RtSignalState;
	Uint32 SignalState;
	Uint32 Update;
	LINK_TskHndl tskHandle;
	buffer *buffers;
	int n_buffers;
} CaptureLink_InstObj;

/*nullsrc link��������Ϣ*/
typedef struct
{
	Bool			tskEnable;							//��������
	Bool			exitTsk;								//�˳�����
	NULLSRC_LINK_CREATE_INFO createInfo;                     //��������
	sAPI_EncoderInputParams AllocFramePara;		//����frame����
	LINK_TskHndl tskHandle;							//������
	LINK_TskHndl tskHandle_msg;
	FRAME_Buf 			frameBuff[NULLSRC_LINK_MAX_ALLOC_BUFF_NUM];
	LINK_EmpFullQueHndl EmpFullQueHndl;				//���������о��
	Uint32			     Msg[NULLSRC_LINK_MSG_NUM];
	LINK_EmpFullQueHndl MsgQueHandle;						//��Ϣ���о��
	CaptureLink_InstObj instObj;
} NULLSRC_LINK_INFO;

Int32 nullSrcLink_createParaInit(NULLSRC_LINK_CREATE_INFO *createInfo);
Int32 nullSrcLink_create(NULLSRC_LINK_INFO *info);
Int32 nullSrcLink_release(NULLSRC_LINK_INFO *info);
Int32 nullSrcLink_enable(NULLSRC_LINK_INFO *info);
Int32 nullSrcLink_disable(NULLSRC_LINK_INFO *info);
void nullSrcLink_printf(NULLSRC_LINK_INFO *info);
#endif
