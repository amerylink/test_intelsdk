#include "inc/dup_link/dupLink.h"

void dupLink_createParams_Init(DUPLink_CreateParams *info)
{
	if(NULL == info)
	{
		nslog(NS_ERROR,"para is NULL\n");
		return;
	}
	
	memset(info, 0, sizeof(DUPLink_CreateParams));

    info->numOutQue = 1;
}


static Int32 dupLink_createQue(DUP_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	Int32 i = 0;

	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	if(info->createArgs.numOutQue > DUP_LINK_MAX_OUT_QUE)
	{
         printf("numOutQue > DUP_LINK_MAX_OUT_QUE !!!\n");
		return LINK_EFAIL;
	}

	for(i = 0; i < info->createArgs.numOutQue; i++)
	{
		info->outQue[i].isConnect = LINK_FALSE;

		/*创建空包队列*/
		status = LINK_queCreate(&(info->outQue[i].pEmpQue),DUP_LINK_OUT_QUE_LEN);
		if(LINK_EFAIL == status)
		{
			nslog(NS_ERROR,"create emp que is error!\n");
			return LINK_EFAIL;		
		}
		/*创建满包队列*/
		status = LINK_queCreate(&(info->outQue[i].pFullQue),DUP_LINK_OUT_QUE_LEN);
		if(LINK_EFAIL == status)
		{
			nslog(NS_ERROR,"create full que is error!\n");
			return LINK_EFAIL;		
		}
	}

	return status;
}

static void* dupLink_sendProcess(void* arg)
{
	if(NULL == arg)
	{
		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}

	Int32 status = LINK_SOK;
	DUP_LINK_INFO *info = (DUP_LINK_INFO *)arg;
	FRAME_Buf *frame;
    Int32 i = 0;

//	prctl(PR_SET_NAME, "dupLink_sendProcess", NULL, NULL, NULL);

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

     
		status = LINK_queGet(&(info->inQue->pFullQue),(void **)(&frame),LINK_TIMEOUT_FOREVER);
		// printf(" 1frame->channelNum %d %d\n",frame->channelNum,info->createArgs.numOutQue);
		if(frame->channelNum >= SYSTEM_MAX_CH_PER_OUT_QUE || info->createArgs.numOutQue <= 0)
		{
			 LINK_quePut(&(info->inQue->pEmpQue),(void *)(frame),LINK_TIMEOUT_FOREVER);
              continue;
		}


         //printf(" 2frame->channelNum %d %d\n",frame->channelNum,info->createArgs.numOutQue);
	
		//printf("==============dupLink_sendProcess:frame->channelNum:%d,prtChInfo->queId:%d %d,info->createArgs.numOutQue:%d\n",frame->channelNum,prtChInfo->queId,pChInfo->queId,info->createArgs.numOutQue);

         for(i = 0; i < info->createArgs.numOutQue; i++)
         {
               //LINK_queGet(&(info->outQue[i]->pEmpQue),(void **)(&frame),LINK_TIMEOUT_FOREVER);
               frame->dupCount++;
               LINK_quePut(&(info->outQue[i].pFullQue),(void *)(frame),LINK_TIMEOUT_FOREVER);
         }
	    

		//LINK_quePut(&(info->inQue->pEmpQue),(void *)(frame),LINK_TIMEOUT_FOREVER);
		
		usleep(2);
	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}

static void* dupLink_recvProcess(void* arg)
{
	if(NULL == arg)
	{
		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}

	Int32 status = LINK_SOK;
	Int32 outQue = 0;
	DUP_LINK_INFO *info = (DUP_LINK_INFO *)arg;
	FRAME_Buf *frame;

//	prctl(PR_SET_NAME, "dupLink_recvProcess", NULL, NULL, NULL);

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

			 status = LINK_queGet(&(info->outQue[outQue].pEmpQue),(void **)(&frame),LINK_TIMEOUT_FOREVER);
              frame->dupCount--;

             
              if(frame->dupCount == 0)
              {
                 // printf("dupLink_recvProcess1 [%d] [%d] %p\n",frame->channelNum,frame->dupCount,&(info->inQue->pEmpQue));
                  LINK_quePut(&(info->inQue->pEmpQue),(void *)(frame),LINK_TIMEOUT_FOREVER);	
                 // printf("dupLink_recvProcess2 [%d] [%d] %p\n",frame->channelNum,frame->dupCount,&(info->inQue->pEmpQue));
              }
              else if(frame->dupCount < 0)
              {
                    printf("dupCount < 0 !!!!!!\n");
              }
        	
		}

		usleep(2);
		
	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}

Int32 dupLink_create(DUP_LINK_INFO *info)
{
	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;
	info->exitTsk = LINK_FALSE;
	info->tskEnable = LINK_FALSE;
	
	status = dupLink_createQue(info);
	if(LINK_EFAIL == status)
	{
		nslog(NS_ERROR,"dupLink_createQue is error\n");
		return LINK_EFAIL;
	}

	/*创建启动任务*/
	status = LINK_tskCreate(&info->tskSendHandle,dupLink_sendProcess,DUPLINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
		nslog(NS_ERROR,"create send task is error!\n");
	}

	/*创建启动任务*/
	status = LINK_tskCreate(&info->tskRecvHandle,dupLink_recvProcess,DUPLINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
		nslog(NS_ERROR,"create recv task is error!\n");
	}
	

	return status;
}

Int32 dupLink_release(DUP_LINK_INFO *info)
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

Int32 dupLink_enable(DUP_LINK_INFO *info)
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

Int32 dupLink_disable(DUP_LINK_INFO *info)
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

Int32 dupLink_connect(DUP_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl)
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

void dupLink_printf(DUP_LINK_INFO *info)
{
	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return;
	}

	Uint32 outQueNum = 0;

	nslog(NS_INFO,"==============dupLink param %d:==============\n",info->LinkNo);

	nslog(NS_INFO,
		"tskEnable:%d\n"
		"exitTsk:%d\n"
		,info->tskEnable,
		info->exitTsk
		);

	nslog(NS_INFO,"=============dupLink createArgs:===============\n");

	nslog(NS_INFO,
		"numOutQue:%d\n"
		,
		info->createArgs.numOutQue
		);

	for(outQueNum = 0; outQueNum < info->createArgs.numOutQue; outQueNum++)
	{	
		nslog(NS_INFO,"\n=============dupLink outQue:outQueNo:%d,isConnect:%d============\n",outQueNum,info->outQue[outQueNum].isConnect);
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




