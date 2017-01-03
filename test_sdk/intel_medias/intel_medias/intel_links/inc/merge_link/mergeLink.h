#ifndef _MERGE_LINK_H_
#define _MERGE_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"

#include "windows.h"
#ifdef WIN32
#define usleep	Sleep
#endif


#define MERGE_LINK_MAX_IN_QUE (10)
#define MERGE_LINK_MAX_CH_PER_IN_QUE    (SYSTEM_MAX_CH_PER_OUT_QUE)
#define MERGE_LINK_OUT_QUE_LEN (MERGE_LINK_MAX_IN_QUE*10)
#define MERGE_LINK_OUT_QUE_MAX_CH (MERGE_LINK_MAX_IN_QUE * MERGE_LINK_MAX_CH_PER_IN_QUE)

typedef struct
{
	Uint32 numInQue;
	/**< Number of inputs queue's */
    	Uint32 numChInQue[MERGE_LINK_MAX_IN_QUE];
	/*输入队列的通道数*/
} MergeLink_CreateParams;

typedef struct
{
	LINK_TskHndl tskSendHandle;						//任务句柄
	LINK_TskHndl tskRecvHandle;						//任务句柄
	Bool			tskEnable;							//启动任务
	Bool			exitTsk;								//退出任务

	Uint32            LinkNo;

	MergeLink_CreateParams createParams;

	LINK_EmpFullQueHndl*   inQue[MERGE_LINK_MAX_IN_QUE];
	/**< Input queue information */

	LINK_EmpFullQueHndl   outQue;
	/**< Output queue information */

	/* max channel number possible in a input que */
	Uint32 inQueMaxCh[MERGE_LINK_MAX_IN_QUE];

	/* incoming channel number to outgoing channel number map */
	Uint32 inQueChNumMap[MERGE_LINK_MAX_IN_QUE][MERGE_LINK_MAX_CH_PER_IN_QUE];

	/* outgoing channel number to input que ID map */
	Uint32 outQueChToInQueMap[MERGE_LINK_MAX_IN_QUE *
	                          MERGE_LINK_MAX_CH_PER_IN_QUE];

	/* outgoing channel number to incoming channel number map, reverse of
	 * inQueChNumMap[] */
	Uint32 outQueChMap[MERGE_LINK_MAX_IN_QUE * MERGE_LINK_MAX_CH_PER_IN_QUE];
}MERGE_LINK_INFO;

void mergeLink_createParams_Init(MergeLink_CreateParams *para);
Int32 mergeLink_set_queChMap(MERGE_LINK_INFO *info);
Int32 mergeLink_create(MERGE_LINK_INFO *info);
Int32 mergeLink_release(MERGE_LINK_INFO *info);
Int32 mergeLink_enable(MERGE_LINK_INFO *info);
Int32 mergeLink_disable(MERGE_LINK_INFO *info);
Int32 mergeLink_connect(MERGE_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl,Uint32 inQueNo);
void mergeLink_printf(MERGE_LINK_INFO *info);
#endif
