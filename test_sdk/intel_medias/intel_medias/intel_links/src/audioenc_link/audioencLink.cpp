#include "audioencLink.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>

static Int32 AudioEncLink_drvProcessData(AudioEncLink_InstObj *pObj, audio_buf *pInFrame, audio_buf *pOutFrame)
{
    Int32 ret = LINK_SOK;
    int num_samples = 0, read_samples = 0;
    char *bufptr = NULL;

    if(NULL == pObj || NULL == pInFrame || NULL == pOutFrame) 
    {
        return LINK_EFAIL;
    }

    
    MediaPacket outpkt;
    TransContext * pTransCtx; 
    pTransCtx = &pObj->TransCtx;


   if(AudioAddPcmData(pTransCtx, (char *)pInFrame->addr, 8192) < 0)
   {
        return LINK_EFAIL;
   }
   
   // AudioFilterInputFrame(pTransCtx);
  //  AudioFilterOutputFrame(pTransCtx);
    
  //  printf("pTransCtx->bGetAudioDecFrame %d %d\n",pTransCtx->bGetAudioDecFrame,pTransCtx->pAudioEncCtx->frame_size);
    while (pTransCtx->bGetAudioDecFrame)
    {
        outpkt.pData = (uint8_t *)pOutFrame->addr;
        AudioEncode(pTransCtx,&outpkt);
        pOutFrame->fill_length = outpkt.iLen;
    //    AudioFilterOutputFrame(pTransCtx);
    }

   // printf("pOutFrame->fill_length %d\n",pOutFrame->fill_length);

	return ret;
}



static Int32 AudioEncLink_CreateInstObj(AudioEncLink_InstObj *pObj)
{

    Int32 ret = LINK_SOK;
    	if(NULL == pObj)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
    	}

    TransContext * pTransCtx; 
    pTransCtx = &pObj->TransCtx;

    MediaSysInit();
    MediaTranscodeInit(pTransCtx,NULL,NULL);
    AudioPcmInfoInit(pTransCtx, pObj->samplerate , pObj->channel, AV_SAMPLE_FMT_S16);
    AudioEncInfoInit(pTransCtx, AV_CODEC_ID_AAC, pObj->samplerate, 64000, pObj->channel, AV_SAMPLE_FMT_S16);
  //  AudioFilterInit(pTransCtx,"aformat=s16:44100:stereo");
    
    return LINK_SOK;
}


Int32 AudioEncLink_createParaInit(AUDIOENC_LINK_CREATE_INFO *createInfo)
{
    Int32 status = LINK_SOK;

    if(NULL == createInfo)
    {
         nslog(NS_ERROR,"createInfo is NULL\n");
         return LINK_EFAIL;
    }

    createInfo->buffNum = AUDIOENC_LINK_DEFAULT_ALLOC_BUFF_NUM;
    createInfo->channel = 2;
    createInfo->samplerate = 44100;
    strcpy((char *)createInfo->DevName,(char *)"hw:1,0");

    return status;
}



static Int32 AudioEncLink_createQue(AUDIOENC_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	Int32 i = 0;
	//FILE *fSrcWrite = NULL;

	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	//fSrcWrite = fopen("src.yuv","w");

	if(info->createInfo.buffNum > AUDIOENC_LINK_MAX_ALLOC_BUFF_NUM || 0 == info->createInfo.buffNum)
	{
		info->createInfo.buffNum = AUDIOENC_LINK_DEFAULT_ALLOC_BUFF_NUM;
	}

	info->EmpFullQueHndl.isConnect = LINK_FALSE;
	
	/*创建空包队列*/
	status = LINK_queCreate(&(info->EmpFullQueHndl.pEmpQue),info->createInfo.buffNum);
	if(LINK_EFAIL == status)
	{
		nslog(NS_ERROR,"create emp que is error!\n");
		return LINK_EFAIL;		
	}
	/*创建满包队列*/
	status = LINK_queCreate(&(info->EmpFullQueHndl.pFullQue),info->createInfo.buffNum);
	if(LINK_EFAIL == status)
	{
		nslog(NS_ERROR,"create full que is error!\n");
		return LINK_EFAIL;		
	}



	for(i = 0; i < info->createInfo.buffNum; i++)
	{
		
		if(NULL == info->frameBuff[i].addr)
		{
             info->frameBuff[i].buf_size = 4096*2;
		    info->frameBuff[i].addr = malloc(info->frameBuff[i].buf_size);
		}

         info->frameBuff[i].bit_rate = 64000;
         info->frameBuff[i].sample_rate = info->createInfo.samplerate;
        
		//printf("=============%d,%p,%p\n",i,&(info->frameBuff[i]),info->frameBuff[i].pSurfaces);
		status = LINK_quePut(&(info->EmpFullQueHndl.pEmpQue),(void *)(&(info->frameBuff[i])),LINK_TIMEOUT_FOREVER);
		if(status != LINK_SOK)
		{
			nslog(NS_ERROR,"LINK_quePut[%d] is error!\n",i);
			return LINK_EFAIL;
		}
	}

	return status;
}

static void* AudioEncLink_Process(void* arg)
{
	if(NULL == arg)
	{
		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}

	Int32 status = LINK_SOK;
	Uint32 *msg = NULL;
	audio_buf *InFrame = NULL;
    audio_buf *OutFrame = NULL;
	AUDIOENC_LINK_INFO *info = (AUDIOENC_LINK_INFO *)arg;

	Uint32 framNum = 0;
	Uint32 startTime = 0;
	Uint32 CurrenTime = 0;

	prctl(PR_SET_NAME, "ADCaptureLink_Process", NULL, NULL, NULL);
  //  FILE *file = fopen("test.aac","a+");
	while(!(info->exitTsk))
	{
		if(LINK_FALSE == info->tskEnable)
		{
			usleep(30);
			continue;
		}

    
		if((NULL == info->PreEmpFullQueHndl) || (LINK_FALSE == info->PreEmpFullQueHndl->isConnect))
		{
			usleep(30);
			continue;
		}

         status = LINK_queGet(&(info->PreEmpFullQueHndl->pFullQue),(void **)(&InFrame),LINK_TIMEOUT_FOREVER);
         

		status = LINK_queGet(&(info->EmpFullQueHndl.pEmpQue),(void **)(&OutFrame),LINK_TIMEOUT_FOREVER);

        
         AudioEncLink_drvProcessData(&info->instObj, InFrame, OutFrame);

         if(OutFrame->fill_length > 0)
         {
              //fwrite(OutFrame->addr,1,OutFrame->fill_length,file);
         }

         status = LINK_quePut(&(info->PreEmpFullQueHndl->pEmpQue),(void *)InFrame,LINK_TIMEOUT_FOREVER);
		
		status = LINK_quePut(&(info->EmpFullQueHndl.pFullQue),(void *)OutFrame,LINK_TIMEOUT_FOREVER);
		
		framNum++;
		CurrenTime = link_get_run_time();
		if((CurrenTime - startTime) >= 5000)
		{
			nslog(NS_INFO,"=========AudioEncLink_Process:fps:%d\n",framNum /5);
			framNum = 0;
			startTime = link_get_run_time();
			
		}
		
	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}




Int32 AudioEncLink_create(AUDIOENC_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->exitTsk = LINK_FALSE;
	info->tskEnable = LINK_FALSE;
    
    info->instObj.channel    = info->createInfo.channel;
    info->instObj.samplerate = info->createInfo.samplerate;
    strcpy((char *)info->instObj.DevName, (char *)info->createInfo.DevName);
    
    AudioEncLink_CreateInstObj(&info->instObj);
    
	/*创建输出队列*/
	status = AudioEncLink_createQue(info);
	if(LINK_EFAIL == status)
	{
		nslog(NS_ERROR,"create Que is error!\n");
		return LINK_EFAIL;
	}

	/*创建启动任务*/
	status = LINK_tskCreate(&info->tskHandle,AudioEncLink_Process,AUDIOENCLINK_TSK_PRI, 0, info);
	if(LINK_EFAIL == status)
	{
		nslog(NS_ERROR,"create task is error!\n");
	}

  
	return status;
}

Int32 AudioEncLink_release(AUDIOENC_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->exitTsk = LINK_TRUE;

	LINK_tskJoin(&info->tskHandle);

    for(Int32 i = 0; i < info->createInfo.buffNum; i++)
	{
		
		if(NULL != info->frameBuff[i].addr)
		{
            free(info->frameBuff[i].addr);
         }
    }

	return status;
}

Int32 AudioEncLink_connect(AUDIOENC_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info || NULL == QueHndl)
	{
		nslog(NS_ERROR,"info or QueHndl is NULL\n");
		return LINK_EFAIL;
	}

	info->PreEmpFullQueHndl = QueHndl;

	info->PreEmpFullQueHndl->isConnect = LINK_TRUE;

	return status;
}

Int32 AudioEncLink_enable(AUDIOENC_LINK_INFO *info)
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

Int32 AudioEncLink_disable(AUDIOENC_LINK_INFO *info)
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
 
