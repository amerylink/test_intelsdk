#ifndef _DUP_LINK_H_
#define _DUP_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"

#ifdef WIN32
#include <windows.h>
#define usleep(x) Sleep(x/1000 == 0?1:x/1000)
#define nslog(x,y)  
#endif

#define DUP_LINK_MAX_OUT_QUE	(4)

#define DUP_LINK_OUT_QUE_LEN (10)

#define DUP_LINK_CH_NOT_MAPPED   (0xFFFF)

/**
    \brief Information about which channels from input are DUPed at the output
*/
typedef struct
{
    Uint32 outQueId;
    /**< Que ID for which this is applicable
            - IGNORED when used with DUPLink_CreateParams
        */

    Uint32 numOutCh;
    /**< number of output channels in this output queue */

    Uint32 inChNum[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< input channel number which maps to this output queue */

} DUPLink_OutQueChInfo;

/**
    \brief DUP link create parameters
*/
typedef struct
{
    Uint32 numOutQue;
    /**< Number of inputs queue's */

  //  DUPLink_OutQueChInfo   outQueChInfo[DUP_LINK_MAX_OUT_QUE];
    /**< Information about which channels from input are DUPed at the output */

} DUPLink_CreateParams;

typedef struct {

    Uint32 queId;
    Uint32 outChNum;

    Bool   rtChInfoUpdate;

} DUPLink_ChInfo;


typedef struct
{
	LINK_TskHndl tskSendHandle;						//任务句柄
	LINK_TskHndl tskRecvHandle;						//任务句柄
	Bool			tskEnable;							//启动任务
	Bool			exitTsk;								//退出任务

	Uint32            LinkNo;

	DUPLink_CreateParams createArgs;

	LINK_EmpFullQueHndl*     inQue;
	/**< Input queue information. */

	LINK_EmpFullQueHndl   outQue[DUP_LINK_MAX_OUT_QUE];
	/**< Output queue information */
}DUP_LINK_INFO;


void  dupLink_createParams_Init(DUPLink_CreateParams *info);
Int32 dupLink_create(DUP_LINK_INFO *info);
Int32 dupLink_release(DUP_LINK_INFO *info);
Int32 dupLink_enable(DUP_LINK_INFO *info);
Int32 dupLink_disable(DUP_LINK_INFO *info);
Int32 dupLink_connect(DUP_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);
void DUPLink_printf(DUP_LINK_INFO *info);


#endif
