#ifndef _SELECT_LINK_H_
#define _SELECT_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"

#define SELECT_LINK_MAX_OUT_QUE	(7)

#define SELECT_LINK_OUT_QUE_LEN (10)

#define SELECT_LINK_CH_NOT_MAPPED   (0xFFFF)

/**
    \brief Information about which channels from input are selected at the output
*/
typedef struct
{
    Uint32 outQueId;
    /**< Que ID for which this is applicable
            - IGNORED when used with SelectLink_CreateParams
        */

    Uint32 numOutCh;
    /**< number of output channels in this output queue */

    Uint32 inChNum[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< input channel number which maps to this output queue */

} SelectLink_OutQueChInfo;

/**
    \brief SELECT link create parameters
*/
typedef struct
{
    Uint32 numOutQue;
    /**< Number of inputs queue's */

    SelectLink_OutQueChInfo   outQueChInfo[SELECT_LINK_MAX_OUT_QUE];
    /**< Information about which channels from input are selected at the output */

} SelectLink_CreateParams;

typedef struct {

    Uint32 queId;
    Uint32 outChNum;

    Bool   rtChInfoUpdate;

} SelectLink_ChInfo;


typedef struct
{
	LINK_TskHndl tskSendHandle;						//任务句柄
	LINK_TskHndl tskRecvHandle;						//任务句柄
	Bool			tskEnable;							//启动任务
	Bool			exitTsk;								//退出任务

	Uint32            LinkNo;

	SelectLink_CreateParams createArgs;

	LINK_EmpFullQueHndl*     inQue;
	/**< Input queue information. */

	LINK_EmpFullQueHndl   outQue[SELECT_LINK_MAX_OUT_QUE];
	/**< Output queue information */

	SelectLink_ChInfo   inChInfo[SELECT_LINK_MAX_OUT_QUE * SYSTEM_MAX_CH_PER_OUT_QUE];

	SelectLink_ChInfo   rtInChInfo[SELECT_LINK_MAX_OUT_QUE * SYSTEM_MAX_CH_PER_OUT_QUE];

	SelectLink_OutQueChInfo   prevOutQueChInfo[SELECT_LINK_MAX_OUT_QUE];
	
}SELECT_LINK_INFO;


void selectLink_createParams_Init(SelectLink_CreateParams *para);
Int32 selectLink_setOutQueChInfo(SELECT_LINK_INFO *info, SelectLink_OutQueChInfo *pPrm);
Int32 selectLink_getOutQueChInfo(SELECT_LINK_INFO *info, SelectLink_OutQueChInfo *pPrm);
Int32 selectLink_create(SELECT_LINK_INFO *info);
Int32 selectLink_release(SELECT_LINK_INFO *info);
Int32 selectLink_enable(SELECT_LINK_INFO *info);
Int32 selectLink_disable(SELECT_LINK_INFO *info);
Int32 selectLink_connect(SELECT_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);
void selectLink_printf(SELECT_LINK_INFO *info);


#endif
