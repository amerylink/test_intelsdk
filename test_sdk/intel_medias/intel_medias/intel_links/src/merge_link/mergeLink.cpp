#include "inc/merge_link/mergeLink.h"


void mergeLink_createParams_Init(MergeLink_CreateParams *para)
{
	if(NULL == para)
	{
//		nslog(NS_ERROR,"para is NULL\n");
		return;
	}
	
	memset(para,0,sizeof(*para));
}

Int32 mergeLink_set_queChMap(MERGE_LINK_INFO *info)
{
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;
	
	Uint32 inQue, numOutCh, chId;

	if(info->createParams.numInQue > MERGE_LINK_MAX_IN_QUE)
	{
//		nslog(NS_ERROR,"numInQue(%d) is error\n",info->createParams.numInQue);
		return LINK_EFAIL;
	}

	numOutCh = 0;

	for (inQue = 0; inQue < info->createParams.numInQue; inQue++)
	{
		info->inQue[inQue] = NULL;

		if(info->createParams.numChInQue[inQue] > MERGE_LINK_MAX_CH_PER_IN_QUE)
		{
//			nslog(NS_ERROR,"numChInQue(%d/%d) is error\n",inQue,info->createParams.numChInQue[inQue]);
			return LINK_EFAIL;
		}

		for (chId = 0; chId < info->createParams.numChInQue[inQue]; chId++)
		{
			info->inQueChNumMap[inQue][chId] = numOutCh;
			info->outQueChToInQueMap[numOutCh] = inQue;
			info->outQueChMap[numOutCh] = chId;

			numOutCh++;
			if(numOutCh >= MERGE_LINK_OUT_QUE_MAX_CH)
			{
//				nslog(NS_ERROR,"numOutCh(%d) is error\n",numOutCh);
				return LINK_EFAIL;
			}
		}

		info->inQueMaxCh[inQue] = info->createParams.numChInQue[inQue];
	}

	return status;
}

static Int32 mergeLink_createQue(MERGE_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	Int32 i = 0;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->outQue.isConnect = LINK_FALSE;
	
	/*创建空包队列*/
	status = LINK_queCreate(&(info->outQue.pEmpQue),MERGE_LINK_OUT_QUE_LEN);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create emp que is error!\n");
		return LINK_EFAIL;		
	}
	/*创建满包队列*/
	status = LINK_queCreate(&(info->outQue.pFullQue),MERGE_LINK_OUT_QUE_LEN);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create full que is error!\n");
		return LINK_EFAIL;		
	}

	return status;
}

static void* mergeLink_sendProcess(void* arg)
{
	if(NULL == arg)
	{
//		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}

	Int32 status = LINK_SOK;
	Int32 inQue = 0;
	MERGE_LINK_INFO *info = (MERGE_LINK_INFO *)arg;
	FRAME_Buf *frame;

//	prctl(PR_SET_NAME, "mergeLink_sendProcess", NULL, NULL, NULL);

	while(!(info->exitTsk))
	{
		if(LINK_FALSE == info->tskEnable)
		{
			Sleep(10);
			continue;
		}

		if(LINK_FALSE == info->outQue.isConnect)
		{
			usleep(10);
			continue;
		}

		for(inQue = 0; inQue < info->createParams.numInQue; inQue++)
		{

			if((NULL == info->inQue[inQue]) || (LINK_FALSE == info->inQue[inQue]->isConnect))
			{
				continue;
			}

			status = LINK_queGet(&(info->inQue[inQue]->pFullQue),(void **)(&frame),LINK_TIMEOUT_NONE);
			if(LINK_EFAIL == status)
			{
				continue;
			}

			if(frame->channelNum > info->inQueMaxCh[inQue])
			{
//				nslog(NS_ERROR,"mergeLink_sendProcess inQue[%d] channelNum(%d) is error\n",inQue,frame->channelNum);
                  LINK_quePut(&(info->inQue[inQue]->pEmpQue),(void *)(frame),LINK_TIMEOUT_FOREVER);		
				continue;
			}

             frame->channelNum =info->inQueChNumMap[inQue][frame->channelNum];

			 LINK_quePut(&(info->outQue.pFullQue),(void *)(frame),LINK_TIMEOUT_NONE);		
			
		}

		usleep(2);
		
	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}

static void* mergeLink_recvProcess(void* arg)
{
	if(NULL == arg)
	{
//		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}

	Int32 status = LINK_SOK;
	Int32 inQue = 0;
	MERGE_LINK_INFO *info = (MERGE_LINK_INFO *)arg;
	FRAME_Buf *frame;


//	prctl(PR_SET_NAME, "mergeLink_recvProcess", NULL, NULL, NULL);

	while(!(info->exitTsk))
	{
		if(LINK_FALSE == info->tskEnable)
		{
			usleep(30);
			continue;
		}

		if(LINK_FALSE == info->outQue.isConnect)
		{
			usleep(30);
			continue;
		}

		for(inQue = 0; inQue < info->createParams.numInQue; inQue++)
		{

			if((NULL == info->inQue[inQue]) || (LINK_FALSE == info->inQue[inQue]->isConnect))
			{
				continue;
			}

			status = LINK_queGet(&(info->outQue.pEmpQue),(void **)(&frame),LINK_TIMEOUT_NONE);
			if(LINK_EFAIL == status)
			{
				continue;
			}
			/* 获取输入队列ID */
		        inQue = info->outQueChToInQueMap[frame->channelNum];

			/* 获取通道ID */
		        frame->channelNum = info->outQueChMap[frame->channelNum];

			 LINK_quePut(&(info->inQue[inQue]->pEmpQue),(void *)(frame),LINK_TIMEOUT_NONE);		
			
		}

		usleep(2);
		
	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}


Int32 mergeLink_create(MERGE_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->exitTsk = LINK_FALSE;
	info->tskEnable = LINK_FALSE;

	status = mergeLink_createQue(info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"mergeLink_createQue is error!\n");
		return LINK_EFAIL;
	}

	/*创建启动任务*/
	status = LINK_tskCreate(&info->tskSendHandle,mergeLink_sendProcess,MERGELINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create send task is error!\n");
	}

	/*创建启动任务*/
	status = LINK_tskCreate(&info->tskRecvHandle,mergeLink_recvProcess,MERGELINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create recv task is error!\n");
	}

	return status;
}


Int32 mergeLink_release(MERGE_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->exitTsk = LINK_TRUE;

	LINK_tskJoin(&info->tskSendHandle);
	LINK_tskJoin(&info->tskRecvHandle);

	return status;
}

Int32 mergeLink_enable(MERGE_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->tskEnable = LINK_TRUE;

	return status;
}

Int32 mergeLink_disable(MERGE_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->tskEnable = LINK_FALSE;

	return status;
}

void mergeLink_printf(MERGE_LINK_INFO *info)
{
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return;
	}

	Uint32 inQueNum = 0;

//	nslog(NS_INFO,"==============mergeLink param %d:==============\n",info->LinkNo);

//	nslog(NS_INFO,
// 		"tskEnable:%d\n"
// 		"exitTsk:%d\n"
// 		,info->tskEnable,
// 		info->exitTsk
// 		);

//	nslog(NS_INFO,"=============mergeLink createParams:===============\n");

//	nslog(NS_INFO,
// 		"numInQue:%d\n"
// 		,
// 		info->createParams.numInQue
// 		);

	for(inQueNum = 0; inQueNum < info->createParams.numInQue; inQueNum++)
	{
// 		nslog(NS_INFO,
// 			"InQueNo:%d  numChInQue:%d\n"
// 			,
// 			inQueNum,
// 			info->createParams.numChInQue[inQueNum]
// 			);		
	}

// 	nslog(NS_INFO,"\n=============mergeLink outQue:isConnect:%d============\n",info->outQue.isConnect);
// 	nslog(NS_INFO,"pEmpQue:\n");
// 	nslog(NS_INFO,
// 		"len:%d\n"
// 		"count:%d\n",
// 		info->outQue.pEmpQue.len,
// 		info->outQue.pEmpQue.count
// 		);
// 
// 	nslog(NS_INFO,"pFullQue:\n");
// 	nslog(NS_INFO,
// 		"len:%d\n"
// 		"count:%d\n",
// 		info->outQue.pFullQue.len,
// 		info->outQue.pFullQue.count
// 		);

	return;
}

Int32 mergeLink_connect(MERGE_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl,Uint32 inQueNo)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info || NULL == QueHndl)
	{
//		nslog(NS_ERROR,"info or QueHndl is NULL\n");
		return LINK_EFAIL;
	}

	if(inQueNo >= info->createParams.numInQue)
	{
//		nslog(NS_ERROR,"inQueNo(%d) is error\n",inQueNo);
		return LINK_EFAIL;
	}

	info->inQue[inQueNo]= QueHndl;

	info->inQue[inQueNo]->isConnect = LINK_TRUE;

	return status;
}



