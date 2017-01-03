#include "../inc/enc_link/encLink.h"
#include "../inc/dec_link/decLink.h"
#include "mfx_samples_config.h"
#include "pipeline_encode.h"
#include "pipeline_user.h"
#include "pipeline_region_encode.h"
#include "pipeline_decode.h"
#include "SDK_codec.h"

#define usleep(x)	Sleep(x/1000 == 0?1:x/1000)


#include "../DDrawDisplay.h"
extern CDDrawDisplay* g_pDisplay[8 + 2];
Uint8* pyv12;
extern FILE *fp1080;
extern FILE *fp360;
extern FILE *fpdec1;

void NV12toYV12p(uint8_t *inyuv, uint8_t *outyuv, int numfmt, int width, int heihgt)
{
	int i, j, ysize;
	uint8_t *up_out = NULL, *vp_out = NULL, *yp_out = NULL, *yp_in = NULL, *up_in = NULL, *vp_in = NULL;


	ysize = width * heihgt;

	up_out = outyuv + ysize;
	vp_out = outyuv + ysize * 5 / 4;
	yp_out = outyuv;

	up_in = inyuv + ysize;
	vp_in = inyuv + ysize + 1;
	yp_in = inyuv;

	memcpy(yp_out, yp_in, ysize);

	for (i = 0; i < width / 2; i++){
		for (j = 0; j < heihgt / 2; j++){
			//			printf("i,j,numfmt: %d %d %d\n",i, j, numfmt);
			*up_out = *up_in;
			*vp_out = *vp_in;
			up_out += 1;
			vp_out += 1;
			up_in += 2;
			vp_in += 2;
		}
	}

}

Int32 decLink_kill(DEC_LINK_INFO *info)
{
	return 0;
}

Int32 decLink_createParaInit(DEC_CREATE_INFO *createInfo)
{
	Int32 status = LINK_SOK;

	if(NULL == createInfo)
	{
//		nslog(NS_ERROR,"createInfo is NULL\n");
		return LINK_EFAIL;
	}

	createInfo->chNo = 0;
	createInfo->queLen = DEC_LINK_DEFAULT_ALLOC_BUFF_NUM;
	pyv12 = (Uint8*)malloc(1920*1080* 3 / 2);

	return status;
}

Int32 decLink_inputParaInit(sAPI_DecoderInputParams *pPrama)
{
	Int32 status = LINK_SOK;

	if(NULL == pPrama)
	{
//		nslog(NS_ERROR,"pPrama is NULL\n");
		return LINK_EFAIL;
	}

	pPrama->nBuffNum = LINK_DEFAULT_ALLOC_BUFF_NUM;
	pPrama->nDecNo = 0;
	pPrama->pHandle = NULL;

	//encLink_EncInputParaInit(pPrama);
	
	return status;
}

static Int32 decLink_createQue(DEC_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	Int32 i = 0;
	//FILE *fSrcWrite = NULL;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	//fSrcWrite = fopen("src.yuv","w");

	if(info->createInfo.queLen > DEC_LINK_MAX_ALLOC_BUFF_NUM || 0 == info->createInfo.queLen)
	{
		info->createInfo.queLen = DEC_LINK_MAX_ALLOC_BUFF_NUM;
	}

	info->EmpFullQueHndl.isConnect = LINK_FALSE;
	
	/*创建空包队列*/
	status = LINK_queCreate(&(info->EmpFullQueHndl.pEmpQue),info->createInfo.queLen);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create emp que is error!\n");
		return LINK_EFAIL;		
	}
	/*创建满包队列*/
	status = LINK_queCreate(&(info->EmpFullQueHndl.pFullQue),info->createInfo.queLen);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create full que is error!\n");
		return LINK_EFAIL;		
	}

	encLink_EncInputParaInit(&info->AllocFramePara);
	
	info->AllocFramePara.nBuffNum = info->createInfo.queLen;

	API_IntelMedia_SDK_HWAllocParaInit(&info->AllocFramePara);

	for(i = 0; i < info->createInfo.queLen; i++)
	{
		info->frameBuff[i].pSurfaces = NULL;
	
		info->frameBuff[i].pSurfaces = API_IntelMedia_SDK_HW_GetBuff(&info->AllocFramePara);

		if(NULL == info->frameBuff[i].pSurfaces)
		{
//			nslog(NS_ERROR,"LINK_GetBuff[%d] is error!\n",i);
			return LINK_EFAIL;
		}

		//info->frameBuff[i].pSurfaces->Info.CropX = 0;
		//info->frameBuff[i].pSurfaces->Info.CropY = 0;
		//info->frameBuff[i].pSurfaces->Info.CropW = info->frameBuff[i].pSurfaces->Info.Width = info->AllocFramePara.nSrcWidth;
		//info->frameBuff[i].pSurfaces->Info.CropH = info->frameBuff[i].pSurfaces->Info.Height = info->AllocFramePara.nSrcHeight;

		info->frameBuff[i].channelNum = info->createInfo.chNo;

#if 0
		if(NULL != fSrcWrite)
		{
			fwrite(info->frameBuff[i].pSurfaces->Data.Y,info->frameBuff[i].pSurfaces->Data.Pitch * info->frameBuff[i].pSurfaces->Info.Height * 3 / 2,1, fSrcWrite);
		}
#endif	
		printf("=============%d,%p,%p\n",i,&(info->frameBuff[i]),info->frameBuff[i].pSurfaces);
		status = LINK_quePut(&(info->EmpFullQueHndl.pEmpQue),(void *)(&(info->frameBuff[i])),LINK_TIMEOUT_NONE);
		if(status != LINK_SOK)
		{
//			nslog(NS_ERROR,"LINK_quePut[%d] is error!\n",i);
			return LINK_EFAIL;
		}
	}

	return status;
}

static void* decLink_Process(void* arg)
{
	if(NULL == arg)
	{
//		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}

	Int32 status = LINK_SOK;
	video_info_t dec_info;
	DEC_LINK_INFO *info = (DEC_LINK_INFO *)arg;
	Int32 i = 0;
	Bool NeedGetBits = LINK_TRUE;
	Bool NeedGetFrame = LINK_TRUE;
	FILE * pwrite = NULL;

	Uint32 framNum = 0;
	Uint32 startTime = 0;
	Uint32 CurrenTime = 0;
	
	//pwrite = fopen("dec.yuv","w");

//	prctl(PR_SET_NAME, "decLink_Process", NULL, NULL, NULL);

	while(!(info->exitTsk))
	{
		if(LINK_FALSE == info->tskEnable)
		{
//			usleep(30);
			Sleep(3);
			continue;
		}


		if((NULL == info->PreEmpFullQueHndl) || (LINK_FALSE == info->PreEmpFullQueHndl->isConnect))
		{
//			usleep(30);
			Sleep(3);
			continue;
		}

		//if(LINK_FALSE == info->EmpFullQueHndl.isConnect)
		//{
		//	usleep(3);
		//	continue;
		//}

		/*获取输入buff*/
		if(NeedGetBits)
		{
			status = LINK_queGet(&(info->PreEmpFullQueHndl->pFullQue),(void **)(&dec_info.pEncBitStrameBuff),LINK_TIMEOUT_FOREVER);
			if(LINK_EFAIL == status)
			{
				usleep(3);
				continue;
			}
			static int iframeCount = 0;
			printf("get:%d,%d\n", ++iframeCount, &(info->PreEmpFullQueHndl->pFullQue));
			
		}

		if(!(info->isDecHeadFinish))
		{
			int offset,len;
			offset = dec_info.pEncBitStrameBuff->Bitstream.DataOffset;
			len = dec_info.pEncBitStrameBuff->Bitstream.DataLength;
			printf("info->decInputParam:%d\n", info->decInputParam.nDecNo);
			status = API_IntelMedia_SDK_HW_DecInitial_DecHeader((void *)(&info->decInputParam),dec_info.pEncBitStrameBuff);
			if(LINK_SOK != status)
			{
				/*还输入空包*/
				//status = LINK_quePut(&(info->PreEmpFullQueHndl->pEmpQue),(void *)dec_info.pEncBitStrameBuff,LINK_TIMEOUT_FOREVER);
//				nslog(NS_INFO,"=========API_IntelMedia_SDK_HW_DecInitial_DecHeader is error!\n");
				continue;
			}
			info->isDecHeadFinish = LINK_TRUE;
			
			API_IntelMedia_SDK_HW_DecInitial_CreateDec((void *)(&info->decInputParam));

			printf("=================pre:offset:%d,len:%d,curr:offset:%d,len:%d\n",offset,len,dec_info.pEncBitStrameBuff->Bitstream.DataOffset,dec_info.pEncBitStrameBuff->Bitstream.DataLength);
		}

		if(NeedGetFrame)
		{
			/*获取输出buff*/
			status = LINK_queGet(&(info->EmpFullQueHndl.pEmpQue),(void **)(&dec_info.pEncSurfaces),LINK_TIMEOUT_FOREVER);
			if(LINK_EFAIL == status)
			{
				usleep(30);
				continue;
			}
		}
		//CurrenTime = link_get_run_time();
		dec_info.EncFrameBuffList.numBufs = 0;
		dec_info.EncBitStrameBuffList.numBufs = 0;
		status = API_IntelMedia_SDK_HW_DecFrame((void *)(&info->decInputParam),(void *)&dec_info);
		
		if(LINK_SOK != status)
		{
//			nslog(NS_INFO,"=========API_IntelMedia_SDK_HW_DecFrame is error!\n");
		}
		//printf("===========sclr process time:%d\n",link_get_run_time() - CurrenTime);
#if 1		
		/*还输出满包*/
		for(i = 0; i < dec_info.EncFrameBuffList.numBufs; i++)
		{
			if(dec_info.EncFrameBuffList.bufs[i]->ValidFrame == 1)
			{
				//status = LINK_quePut(&(info->EmpFullQueHndl.pFullQue), (void *)dec_info.EncFrameBuffList.bufs[i], LINK_TIMEOUT_NONE);
				status = LINK_quePut(&(info->EmpFullQueHndl.pEmpQue), (void *)dec_info.EncFrameBuffList.bufs[i], LINK_TIMEOUT_NONE);
				NeedGetFrame = LINK_TRUE;
				memset(pyv12, 0, 1920 * 1080 * 3 / 2);
				NV12toYV12p(dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Data.Y, pyv12, 1, dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Info.CropW, dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Info.CropH);
				g_pDisplay[info->id + 2]->DisplayVideo((char*)pyv12/*(char*)dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Data.Y*/, dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Info.CropW*dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Info.CropH * 3 / 2);
			}
			else
			{
				NeedGetFrame = LINK_FALSE;
			}
		}
#else
		/*还输出空包*/
		for(i = 0; i < dec_info.EncFrameBuffList.numBufs; i++)
		{
			if(dec_info.EncFrameBuffList.bufs[i]->ValidFrame == 1)
			{
				if(pwrite)
				{
					fwrite(dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Data.Y,(dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Info.CropW) * (dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Info.CropH)
						,1,pwrite);
					fwrite(dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Data.UV,((dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Info.CropW) * (dec_info.EncFrameBuffList.bufs[i]->pSurfaces->Info.CropH) >> 1)
						,1,pwrite);
				}

				status = LINK_quePut(&(info->EmpFullQueHndl.pEmpQue),(void *)dec_info.EncFrameBuffList.bufs[i],LINK_TIMEOUT_NONE);
				if(LINK_EFAIL == status)
				{
					usleep(30);
					continue;
				}

				NeedGetFrame = LINK_TRUE;
			}
			else
			{
				NeedGetFrame = LINK_FALSE;
			}
		}
#endif
		/*还输入空包*/
		if(dec_info.EncBitStrameBuffList.NeedGetBits == 0)
		{
			NeedGetBits = LINK_FALSE;
		}
		else
		{
			NeedGetBits = LINK_TRUE;
			for(i = 0; i < dec_info.EncBitStrameBuffList.numBufs; i++)
			{
				status = LINK_quePut(&(info->PreEmpFullQueHndl->pEmpQue),(void *)dec_info.EncBitStrameBuffList.bufs[i],LINK_TIMEOUT_FOREVER);
			}
		}

		framNum++;
		CurrenTime = link_get_run_time();
		if((CurrenTime - startTime) >= 5000)
		{
//			nslog(NS_INFO,"=========decLink_Process:fps:%d\n",framNum / 5);
			framNum = 0;
			startTime = link_get_run_time();
			
		}
		
	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}

Int32 decLink_create(DEC_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}
	
	info->exitTsk = LINK_FALSE;
	info->tskEnable = LINK_FALSE;
	info->isDecHeadFinish = LINK_FALSE;
	info->PreEmpFullQueHndl = NULL;

	status = decLink_createQue(info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create que is error!\n");
		return LINK_EFAIL;
	}
#if 1
	status == API_IntelMedia_SDK_HW_DecInitial_CreateSession((void *)(&(info->decInputParam)));
	if(LINK_SOK != status)
	{
//		nslog(NS_ERROR,"dec create Session is error!\n");
	}
#endif	
#if 0
       encLink_EncInputParaInit(&(info->decInputParam));
	status == API_IntelMedia_SDK_HW_DecInitial_reach((void *)(&(info->decInputParam)));
	if(LINK_SOK != status)
	{
		nslog(NS_ERROR,"API_IntelMedia_SDK_HW_DecInitial_reach is error!\n");
	}
#endif
	
	/*创建启动任务*/
	status = LINK_tskCreate(&info->tskHandle,decLink_Process,DECLINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create task is error!\n");
	}

	return status;
}

Int32 decLink_release(DEC_LINK_INFO *info)
{
// 	Int32 status = LINK_SOK;
// 
// 	if(NULL == info)
// 	{
// //		nslog(NS_ERROR,"info is NULL\n");
// 		return LINK_EFAIL;
// 	}
// 
// 	info->exitTsk = LINK_TRUE;
// 
// 	LINK_tskJoin(&info->tskHandle);
	API_IntelMedia_SDK_HW_DecRelease(NULL);

	return LINK_TRUE;
}

Int32 decLink_enable(DEC_LINK_INFO *info)
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

Int32 decLink_disable(DEC_LINK_INFO *info)
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

void decLink_printf(DEC_LINK_INFO *info)
{
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return;
	}

//	nslog(NS_INFO,"==============decLink param %d:==============\n",info->createInfo.chNo);

//	nslog(NS_INFO,
// 		"tskEnable:%d\n"
// 		"exitTsk:%d\n"
// 		"isDecHeadFinish:%d\n"
// 		,info->tskEnable,
// 		info->exitTsk,
// 		info->isDecHeadFinish
// 		);

//	nslog(NS_INFO,"=============decLink createInfo:===============\n");

//	nslog(NS_INFO,
// 		"queLen:%d\n"
// 		,
// 		info->createInfo.queLen
// 		);

//	nslog(NS_INFO,"=============decLink inputPara:===============\n");

//	nslog(NS_INFO,
// 		"nBuffNum:%d\n"
// 		"pHandle:%p\n",
// 		info->decInputParam.nBuffNum,
// 		info->decInputParam.pHandle
// 		);

//	nslog(NS_INFO,"=============decLink AllocFramePara:===============\n");

//	nslog(NS_INFO,
// 		"nWidth:%d\n"
// 		"nHeight:%d\n"
// 		"nBuffNum:%d\n",
// 		info->AllocFramePara.nSrcWidth,
// 		info->AllocFramePara.nSrcHeight,
// 		info->AllocFramePara.nBuffNum
// 		);

//	nslog(NS_INFO,"\n=============decLink FrameQue:isConnect:%d============\n",info->EmpFullQueHndl.isConnect);
	
//	nslog(NS_INFO,"pEmpQue:\n");
//	nslog(NS_INFO,
// 		"len:%d\n"
// 		"count:%d\n",
// 		info->EmpFullQueHndl.pEmpQue.len,
// 		info->EmpFullQueHndl.pEmpQue.count
// 		);

//	nslog(NS_INFO,"pFullQue:\n");
//	nslog(NS_INFO,
// 		"len:%d\n"
// 		"count:%d\n",
// 		info->EmpFullQueHndl.pFullQue.len,
// 		info->EmpFullQueHndl.pFullQue.count
// 		);

	return;
}

Int32 decLink_connect(DEC_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info || NULL == QueHndl)
	{
//		nslog(NS_ERROR,"info or QueHndl is NULL\n");
		return LINK_EFAIL;
	}

	info->PreEmpFullQueHndl = QueHndl;

	info->PreEmpFullQueHndl->isConnect = LINK_TRUE;

	return status;
}


