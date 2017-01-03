#ifndef _DEC_LINK_H_
#define _DEC_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"
#include "SDK_codec.h"


#define DEC_LINK_MAX_ALLOC_BUFF_NUM 10
#define DEC_LINK_DEFAULT_ALLOC_BUFF_NUM 6

/*dec link 创建参数*/
typedef struct{
	Uint32 queLen;
	Uint32 chNo;
}DEC_CREATE_INFO;

/*dec link的数据信息*/
typedef struct{
	Bool			tskEnable;							//启动任务
	Bool			exitTsk;								//退出任务
	Bool			isDecHeadFinish;						//解码头信息是否完成
	DEC_CREATE_INFO createInfo;                     //创建参数
	sAPI_DecoderInputParams decInputParam;           //解码输入参数
	sAPI_EncoderInputParams AllocFramePara;		//分配frame参数
	LINK_TskHndl tskHandle;							//任务句柄
	LINK_EmpFullQueHndl *PreEmpFullQueHndl;	//空满包队列句柄
	FRAME_Buf 			frameBuff[DEC_LINK_MAX_ALLOC_BUFF_NUM];
	LINK_EmpFullQueHndl EmpFullQueHndl;				//空满包队列句柄
	int				id;
}DEC_LINK_INFO;

Int32 decLink_create(DEC_LINK_INFO *info);
Int32 decLink_release(DEC_LINK_INFO *info);
Int32 decLink_enable(DEC_LINK_INFO *info);
Int32 decLink_disable(DEC_LINK_INFO *info);
Int32 decLink_connect(DEC_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);
void decLink_printf(DEC_LINK_INFO *info);
Int32 decLink_inputParaInit(sAPI_DecoderInputParams *pPrama);
Int32 decLink_createParaInit(DEC_CREATE_INFO *createInfo);
Int32 decLink_kill(DEC_LINK_INFO *info);

#endif
