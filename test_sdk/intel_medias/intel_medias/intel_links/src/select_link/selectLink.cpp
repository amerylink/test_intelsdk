#include "selectLink.h"

void selectLink_createParams_Init(SelectLink_CreateParams *para)
{
	if(NULL == para)
	{
		nslog(NS_ERROR,"para is NULL\n");
		return;
	}
	
	memset(para,0,sizeof(*para));
}

Int32 selectLink_setOutQueChInfo(SELECT_LINK_INFO *info, SelectLink_OutQueChInfo *pPrm)
{
	if(NULL == info || NULL == pPrm)
	{
		nslog(NS_ERROR,"para is NULL\n");
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;
	Uint32 inChNum, outChNum;
	SelectLink_ChInfo *pChInfo;
	SelectLink_OutQueChInfo *pPrevPrm;

	if(pPrm->numOutCh > SYSTEM_MAX_CH_PER_OUT_QUE)
		return LINK_EFAIL;

	if(pPrm->outQueId > info->createArgs.numOutQue)
		return LINK_EFAIL;

	pPrevPrm = &info->prevOutQueChInfo[pPrm->outQueId];

	/* remove prev output queue channel mapping */
	for(outChNum=0; outChNum<pPrevPrm->numOutCh; outChNum++)
	{

		inChNum = pPrevPrm->inChNum[outChNum];

		if(inChNum >= SYSTEM_MAX_CH_PER_OUT_QUE)
		{
			nslog(NS_ERROR,"inChNum(%d) is error\n");
			continue;
		}

		pChInfo = &info->inChInfo[inChNum];

		pChInfo->queId = SELECT_LINK_CH_NOT_MAPPED;
		pChInfo->outChNum = 0;
		pChInfo->rtChInfoUpdate = LINK_FALSE;

	}

	/* mapped input to output channels */
	for(outChNum=0; outChNum<pPrm->numOutCh; outChNum++)
	{
		inChNum = pPrm->inChNum[outChNum];

		if(inChNum >= SYSTEM_MAX_CH_PER_OUT_QUE)
		{
			nslog(NS_ERROR,"inChNum(%d) is error\n");
			continue;
		}

		pChInfo = &info->inChInfo[inChNum];

		pChInfo->queId = pPrm->outQueId;
		pChInfo->outChNum = outChNum;
		pChInfo->rtChInfoUpdate = LINK_TRUE;
	}

	*pPrevPrm = *pPrm;

	return status;
}

Int32 selectLink_getOutQueChInfo(SELECT_LINK_INFO *info, SelectLink_OutQueChInfo *pPrm)
{
	if(NULL == info || NULL == pPrm)
	{
		nslog(NS_ERROR,"para is NULL\n");
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;
	Uint32 outChNum;
	SelectLink_OutQueChInfo *pPrevPrm;

	pPrm->numOutCh = 0;

	if(pPrm->outQueId > info->createArgs.numOutQue)
		return LINK_EFAIL;

	pPrevPrm = &info->prevOutQueChInfo[pPrm->outQueId];

	/* copy current output que info to user supplied pointer */
	pPrm->numOutCh = pPrevPrm->numOutCh;

	if(pPrm->numOutCh > SYSTEM_MAX_CH_PER_OUT_QUE)
	{
		pPrm->numOutCh = 0;
		return LINK_EFAIL;
	}

	for(outChNum=0; outChNum<pPrevPrm->numOutCh; outChNum++)
	{
		pPrm->inChNum[outChNum] = pPrevPrm->inChNum[outChNum];
	}

	return status;
}


static Int32 selectLink_createQue(SELECT_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	Int32 i = 0;

	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	if(info->createArgs.numOutQue > SELECT_LINK_MAX_OUT_QUE)
		return LINK_EFAIL;

	for(i = 0; i < info->createArgs.numOutQue; i++)
	{
		info->outQue[i].isConnect = LINK_FALSE;

		/*创建空包队列*/
		status = LINK_queCreate(&(info->outQue[i].pEmpQue),SELECT_LINK_OUT_QUE_LEN);
		if(LINK_EFAIL == status)
		{
			nslog(NS_ERROR,"create emp que is error!\n");
			return LINK_EFAIL;		
		}
		/*创建满包队列*/
		status = LINK_queCreate(&(info->outQue[i].pFullQue),SELECT_LINK_OUT_QUE_LEN);
		if(LINK_EFAIL == status)
		{
			nslog(NS_ERROR,"create full que is error!\n");
			return LINK_EFAIL;		
		}
	}

	return status;
}

static void* selectLink_sendProcess(void* arg)
{
	if(NULL == arg)
	{
		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}

	Int32 status = LINK_SOK;
	Bool  needPutFrame = LINK_FALSE;
	SELECT_LINK_INFO *info = (SELECT_LINK_INFO *)arg;
	SelectLink_ChInfo *pChInfo;
	SelectLink_ChInfo *prtChInfo;
	FRAME_Buf *frame;

	prctl(PR_SET_NAME, "selectLink_sendProcess", NULL, NULL, NULL);

	while(!(info->exitTsk))
	{
		if(LINK_FALSE == info->tskEnable)
		{
			usleep(30);
			continue;
		}

		if((NULL == info->inQue) || (LINK_FALSE == info->inQue->isConnect))
		{
			usleep(30);
			continue;
		}

		needPutFrame = LINK_FALSE;

		status = LINK_queGet(&(info->inQue->pFullQue),(void **)(&frame),LINK_TIMEOUT_FOREVER);
		if(LINK_EFAIL == status)
		{
			continue;
		}

		if(frame->channelNum >= SYSTEM_MAX_CH_PER_OUT_QUE)
		{
			needPutFrame= LINK_TRUE;
			goto exit;
		}

		pChInfo = &info->inChInfo[frame->channelNum];
		prtChInfo = &info->rtInChInfo[frame->channelNum];
		
		if(LINK_TRUE == pChInfo->rtChInfoUpdate)
		{
			pChInfo->rtChInfoUpdate = LINK_FALSE;
			prtChInfo->outChNum = pChInfo->outChNum;
			prtChInfo->queId = pChInfo->queId;
		}


		//printf("==============selectLink_sendProcess:frame->channelNum:%d,prtChInfo->queId:%d %d,info->createArgs.numOutQue:%d\n",frame->channelNum,prtChInfo->queId,pChInfo->queId,info->createArgs.numOutQue);

		if(prtChInfo->queId >= info->createArgs.numOutQue)
		{
			needPutFrame= LINK_TRUE;
			goto exit;	
		}
		else
		{
			if(LINK_FALSE == info->outQue[prtChInfo->queId].isConnect)
			{
				needPutFrame= LINK_TRUE;
			}
			else
			{
				//printf("=========%d,%d\n",frame->channelNum,prtChInfo->outChNum);
				/* save original channelNum so that we can restore it later while releasing the frame */
				frame->selectOrgChannelNum[info->LinkNo] = frame->channelNum;

				/* channel channel number according to output que channel number */
				frame->channelNum = prtChInfo->outChNum;
	
				/* put the frame in output que */
				LINK_quePut(&(info->outQue[prtChInfo->queId].pFullQue), (void *)(frame),LINK_TIMEOUT_NONE);
			}

		}

exit:
		if(LINK_TRUE == needPutFrame)
		{
			LINK_quePut(&(info->inQue->pEmpQue),(void *)(frame),LINK_TIMEOUT_NONE);
		}

		usleep(2*1000);
		
	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}

static void* selectLink_recvProcess(void* arg)
{
	if(NULL == arg)
	{
		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}

	Int32 status = LINK_SOK;
	Int32 outQue = 0;
	SELECT_LINK_INFO *info = (SELECT_LINK_INFO *)arg;
	FRAME_Buf *frame;

	prctl(PR_SET_NAME, "selectLink_recvProcess", NULL, NULL, NULL);

	while(!(info->exitTsk))
	{
		if(LINK_FALSE == info->tskEnable)
		{
			usleep(30);
			continue;
		}

		if((NULL == info->inQue) || (LINK_FALSE == info->inQue->isConnect))
		{
			usleep(30);
			continue;
		}

		for(outQue = 0; outQue < info->createArgs.numOutQue; outQue++)
		{

			if((LINK_FALSE == info->outQue[outQue].isConnect))
			{
				continue;
			}

			status = LINK_queGet(&(info->outQue[outQue].pEmpQue),(void **)(&frame),LINK_TIMEOUT_NONE);
			if(LINK_EFAIL == status)
			{
				continue;
			}

			frame->channelNum = frame->selectOrgChannelNum[info->LinkNo];

			 LINK_quePut(&(info->inQue->pEmpQue),(void *)(frame),LINK_TIMEOUT_NONE);		
			
		}

		usleep(2*1000);
		
	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}

Int32 selectLink_create(SELECT_LINK_INFO *info)
{
	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;
	info->exitTsk = LINK_FALSE;
	info->tskEnable = LINK_FALSE;
	Uint32 queId,outChNum,inChNum;
	SelectLink_ChInfo *pChInfo;
	SelectLink_ChInfo *prtChInfo;


	for(queId = 0; queId < info->createArgs.numOutQue; queId++)
	{
		for(outChNum=0; outChNum< info->createArgs.outQueChInfo[queId].numOutCh; outChNum++)
		{
			inChNum = info->createArgs.outQueChInfo[queId].inChNum[outChNum];
			pChInfo = &info->inChInfo[inChNum];
			prtChInfo = &info->rtInChInfo[inChNum];

			pChInfo->queId = info->createArgs.outQueChInfo[queId].outQueId;
			pChInfo->outChNum = outChNum;
			pChInfo->rtChInfoUpdate = LINK_TRUE;
		}
	}

	status = selectLink_createQue(info);
	if(LINK_EFAIL == status)
	{
		nslog(NS_ERROR,"selectLink_createQue is error\n");
		return LINK_EFAIL;
	}

	/*创建启动任务*/
	status = LINK_tskCreate(&info->tskSendHandle,selectLink_sendProcess,SELECTLINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
		nslog(NS_ERROR,"create send task is error!\n");
	}

	/*创建启动任务*/
	status = LINK_tskCreate(&info->tskRecvHandle,selectLink_recvProcess,SELECTLINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
		nslog(NS_ERROR,"create recv task is error!\n");
	}
	

	return status;
}

Int32 selectLink_release(SELECT_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->exitTsk = LINK_TRUE;

	LINK_tskJoin(&info->tskSendHandle);
	LINK_tskJoin(&info->tskRecvHandle);

	return status;
}

Int32 selectLink_enable(SELECT_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->tskEnable = LINK_TRUE;

	return status;
}

Int32 selectLink_disable(SELECT_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->tskEnable = LINK_FALSE;

	return status;
}

Int32 selectLink_connect(SELECT_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info || NULL == QueHndl)
	{
		nslog(NS_ERROR,"info or QueHndl is NULL\n");
		return LINK_EFAIL;
	}

	info->inQue= QueHndl;

	info->inQue->isConnect = LINK_TRUE;

	return status;
}

void selectLink_printf(SELECT_LINK_INFO *info)
{
	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return;
	}

	Uint32 outQueNum = 0;

	nslog(NS_INFO,"==============selectLink param %d:==============\n",info->LinkNo);

	nslog(NS_INFO,
		"tskEnable:%d\n"
		"exitTsk:%d\n"
		,info->tskEnable,
		info->exitTsk
		);

	nslog(NS_INFO,"=============selectLink createArgs:===============\n");

	nslog(NS_INFO,
		"numOutQue:%d\n"
		,
		info->createArgs.numOutQue
		);

	for(outQueNum = 0; outQueNum < info->createArgs.numOutQue; outQueNum++)
	{	
		nslog(NS_INFO,"\n=============selectLink outQue:outQueNo:%d,isConnect:%d============\n",outQueNum,info->outQue[outQueNum].isConnect);
		nslog(NS_INFO,"pEmpQue:\n");
		nslog(NS_INFO,
			"len:%d\n"
			"count:%d\n",
			info->outQue[outQueNum].pEmpQue.len,
			info->outQue[outQueNum].pEmpQue.count
			);

		nslog(NS_INFO,"pFullQue:\n");
		nslog(NS_INFO,
			"len:%d\n"
			"count:%d\n",
			info->outQue[outQueNum].pFullQue.len,
			info->outQue[outQueNum].pFullQue.count
			);
	}
}




