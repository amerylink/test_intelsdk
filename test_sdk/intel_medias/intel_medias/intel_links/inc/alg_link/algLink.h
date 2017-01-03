#ifndef _ALG_LINK_H_
#define _ALG_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"
#include "SDK_codec.h"

#include "PPT_Index_ALL.h"

#define ALG_LINK_MAX_ALLOC_BUFF_NUM 10
#define ALG_LINK_DEFAULT_ALLOC_BUFF_NUM 6
#define ALG_LINK_MSG_NUM					10


typedef struct
{
   
} AlgLink_LayoutPrm;

typedef struct AlgLink_chObj
{
	LINK_EmpFullQueHndl inQue;
   	FRAME_Buf *pCurInFrame;
    Bool         isPlaybackChannel;
    Bool chRtInInfoUpdate;
    Bool chRtInfoUpdate;
} AlgLink_chObj;



typedef struct{
	Uint32 width;
	Uint32 height;
}AlgScdParams;


typedef struct{
	CUSTOM_SET_ENC_PARAM m_user_param;
    lImage m_Image;
	Int8 *ptr[3];
	void (*action)(Int32);
}AlgScdObj;

/*swms link 创建参数*/
typedef struct{
	AlgScdParams algscdPrm;
	Uint32 queLen;
}ALG_CREATE_INFO;


typedef struct{
	Bool			tskEnable;							//启动任务
	Bool			exitTsk;								//退出任务
	pthread_mutex_t lock;
	ALG_CREATE_INFO createInfo;                     //创建参数
	LINK_TskHndl tskHandle;							//任务句柄
	LINK_EmpFullQueHndl *PreEmpFullQueHndl;	//帧空满包队列句柄
	
	LINK_EmpFullQueHndl EmpFullQueHndl;				//空满包队列句柄
	
	AlgScdObj algscdObj;
}ALG_LINK_INFO;

Int32 algLink_createParaInit(ALG_CREATE_INFO *createInfo);
Int32 algLink_create(ALG_LINK_INFO *info);
Int32 algLink_release(ALG_LINK_INFO *info);
Int32 algLink_enable(ALG_LINK_INFO *info);
Int32 algLink_disable(ALG_LINK_INFO *info);
void  algLink_printf(ALG_LINK_INFO *info);
Int32 algLink_connect(ALG_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);
Int32 AlgSetScdAction(AlgScdObj *scdObj, void (*Action)(Int32));
#endif
