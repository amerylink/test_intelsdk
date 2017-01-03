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

/*nullsrc link 创建参数*/
typedef struct
{
	Uint32 width;
	Uint32 height;
	Uint32 buffNum;
	Uint32 timerPeriod;/*周期处理时间ms*/
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

/*nullsrc link的数据信息*/
typedef struct
{
	Bool			tskEnable;							//启动任务
	Bool			exitTsk;								//退出任务
	NULLSRC_LINK_CREATE_INFO createInfo;                     //创建参数
	sAPI_EncoderInputParams AllocFramePara;		//分配frame参数
	LINK_TskHndl tskHandle;							//任务句柄
	LINK_TskHndl tskHandle_msg;
	FRAME_Buf 			frameBuff[NULLSRC_LINK_MAX_ALLOC_BUFF_NUM];
	LINK_EmpFullQueHndl EmpFullQueHndl;				//空满包队列句柄
	Uint32			     Msg[NULLSRC_LINK_MSG_NUM];
	LINK_EmpFullQueHndl MsgQueHandle;						//消息队列句柄
	CaptureLink_InstObj instObj;
} NULLSRC_LINK_INFO;

Int32 nullSrcLink_createParaInit(NULLSRC_LINK_CREATE_INFO *createInfo);
Int32 nullSrcLink_create(NULLSRC_LINK_INFO *info);
Int32 nullSrcLink_release(NULLSRC_LINK_INFO *info);
Int32 nullSrcLink_enable(NULLSRC_LINK_INFO *info);
Int32 nullSrcLink_disable(NULLSRC_LINK_INFO *info);
void nullSrcLink_printf(NULLSRC_LINK_INFO *info);
#endif
