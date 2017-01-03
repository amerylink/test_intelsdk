#include "inc/enc_link/encLink.h"
#include "inc/sclr_link/sclrLink.h"
#include "mfx_samples_config.h"
#include "pipeline_encode.h"
#include "pipeline_user.h"
#include "pipeline_region_encode.h"
#include "pipeline_decode.h"
#include "SDK_codec.h"

#include <atltrace.h>

#ifdef WIN32
#
#endif
Int32 sclrLink_createParaInit(SCLR_CREATE_INFO *createInfo)
{
	Int32 status = LINK_SOK;
	
	if(NULL == createInfo)
	{
//		nslog(NS_ERROR,"createInfo is NULL\n");
		return LINK_EFAIL;
	}
	
	createInfo->chNo = 0;
	createInfo->queLen = SCLR_LINK_DEFAULT_ALLOC_BUFF_NUM;
	createInfo->dstX = 0;
	createInfo->dstY = 0;
	createInfo->dstWidth = VIDEO_WIDTH;
	createInfo->dstHeight = VIDEO_HEIGHT;
	
	return status;
}

static Int32 sclrLink_createQue(SCLR_LINK_INFO *info)
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
	
	if(info->createInfo.queLen > SCLR_LINK_MAX_ALLOC_BUFF_NUM || 0 == info->createInfo.queLen)
	{
		info->createInfo.queLen = SCLR_LINK_MAX_ALLOC_BUFF_NUM;
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
// 	if(status != 0)
// 	{
//		nslog(NS_ERROR,"LINK_AllocBuff is error!\n",i);
// 		return LINK_EFAIL;
// 	}
	
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
		//printf("=============%d,%p,%p\n",i,&(info->frameBuff[i]),info->frameBuff[i].pSurfaces);
		status = LINK_quePut(&(info->EmpFullQueHndl.pEmpQue),(void *)(&(info->frameBuff[i])),LINK_TIMEOUT_NONE);
		if(status != LINK_SOK)
		{
//			nslog(NS_ERROR,"LINK_quePut[%d] is error!\n",i);
			return LINK_EFAIL;
		}
	}
	
	return status;
}
#if 1
#if 0
/*双线性插值法*/
static Int32 sclrLink_sclrProcess(SCLR_LINK_INFO *info,FRAME_Buf *src, FRAME_Buf *dst)
{

	if(NULL == info || NULL == src || NULL == dst)
	{
		nslog(NS_ERROR,"para is NULL\n");
		return LINK_EFAIL;
	}
	
	Int32 status = LINK_SOK;
	mfxFrameInfo& pSrcInfo = src->pSurfaces->Info;
	mfxFrameData& pSrcData = src->pSurfaces->Data;
	mfxFrameInfo& pDstInfo = dst->pSurfaces->Info;
	mfxFrameData& pDstData = dst->pSurfaces->Data;
	Uint32 x0Start= pSrcInfo.CropX;
	Uint32 y0Start = pSrcInfo.CropY;
	Uint32 w0 = pSrcInfo.CropW;
	Uint32 h0 = pSrcInfo.CropH;
	Uint32 pitch0 = pSrcData.Pitch;
	
	//pthread_mutex_lock(&info->lock);
	Uint32 x1Start= info->rtSclrInfo.dstX;
	Uint32 y1Start = info->rtSclrInfo.dstY;
	Uint32 w1 = info->rtSclrInfo.dstWidth;
	Uint32 h1 = info->rtSclrInfo.dstHeight;
	//pthread_mutex_unlock(&info->lock);
	
	Uint32 pitch1 = pDstData.Pitch;
	Uint8* pSrcY = pSrcData.Y;
	Uint8* pSrcY1 = pSrcData.Y;
	Uint8* pSrcY2 = pSrcData.Y;
	Uint8* pSrcUV = pSrcData.UV;
	Uint8* pSrcUV1 = pSrcData.UV;
	Uint8* pSrcUV2 = pSrcData.UV;
	Uint8* pDstY = pDstData.Y;
	Uint8* pDstUV = pDstData.UV;
	Uint8* p1Y = NULL;
	Uint8* p1UV = NULL;
	float fw = float(w0-1) / (w1-1);
	float fh = float(h0-1) / (h1-1);
	float x0, y0;
	Uint32 y1, y2, x1, x2;
	float fx1, fx2, fy1, fy2;
	
	if((x1Start + w1) > VIDEO_WIDTH || (y1Start + h1) > VIDEO_HEIGHT)
	{
		nslog(NS_ERROR,"x:%d,y:%d,w:%d,h:%d,\n",x1Start,y1Start,w1,h1);
		return LINK_EFAIL;
	}
	
	Uint32* arr_x1 = new Uint32[w1];
	Uint32* arr_x2 = new Uint32[w1];
	float* arr_fx1 = new float[w1];
	Uint32* arr_y1 = new Uint32[h1];
	Uint32* arr_y2 = new Uint32[h1];
	float* arr_fy1 = new float[h1];
	
	pDstInfo.CropX = x1Start;
	pDstInfo.CropY = y1Start;
	pDstInfo.CropW = w1;
	pDstInfo.CropH = h1;
	
	for(Uint32 x=x1Start; x < (x1Start + w1); x++)
	{
		x0 = x*fw;
		arr_x1[x] = int(x0);
#if 0
		if(arr_x1[x] == (x0Start + w0 - 1))
			arr_x2[x] = arr_x1[x];
		else
			arr_x2[x] = int(x0+0.5f);
#else
		arr_x2[x] = int(x0+0.5f);
#endif
		arr_fx1[x] = x0 - arr_x1[x];
	}
	
	for(Uint32 y=y1Start; y< (y1Start + h1); y++)
	{
		y0 = y*fh;
		arr_y1[y] = int(y0);
#if 0
		if(arr_y1[y] == (y0Start + h0 - 1))
			arr_y2[y] = arr_y1[y];
		else
			arr_y2[y] = int(y0+0.5f);
#else
		arr_y2[y] = int(y0+0.5f);
#endif
		arr_fy1[y] = y0 - arr_y1[y];
	}
	/*处理y分量*/
	for(Uint32 y=y1Start; y< (y1Start + h1); y++)
	{
		y1 =  arr_y1[y] ;
		y2 = arr_y2[y] ;
		fy1 = arr_fy1[y];
		fy2 = 1.0f - fy1;
		p1Y = pDstY + y*pitch1 + x1Start;
		pSrcY1 = pSrcY + pitch0*y1 ;
		pSrcY2 = pSrcY + pitch0*y2 ;
		for(int x=x1Start; x<(x1Start + w1); x++)
		{
			x1 = arr_x1[x];
			x2 = arr_x2[x];
			fx1 = arr_fx1[x];
			fx2 = 1.0f-fx1;
			float s1 = fx2*fy2;
			float s2 = fx1*fy2;
			float s3 = fx1*fy1;
			float s4 = fx2*fy1;
			
			Uint8* p11 = pSrcY1 + x1;
			Uint8* p12 = pSrcY1  + x2;
			Uint8* p21 = pSrcY2 + x1;
			Uint8* p22 = pSrcY2 + x2;
			*p1Y = (Uint8)((*p11)*s1 + (*p12)*s2 + (*p21)*s4 + (*p22)*s3);
			p1Y++;
		}
	}
	
	for(Uint32 x=(x1Start >> 1); x < ((x1Start + w1) >> 1); x++)
	{
		x0 = x*fw;
		arr_x1[x] = int(x0);
		arr_fx1[x] = x0 - arr_x1[x];
	}
	
	for(Uint32 y=(y1Start >> 1); y< ((y1Start + h1) >> 1); y++)
	{
		y0 = y*fh;
		arr_y1[y] = int(y0);
#if 0
		if(arr_y1[y] == (((y0Start + h0) >> 1) - 1))
			arr_y2[y] = arr_y1[y];
		else
			arr_y2[y] = int(y0+0.5f);
#else
		arr_y2[y] = int(y0+0.5f);
#endif
		arr_fy1[y] = y0 - arr_y1[y];
	}
	
	/*处理UV分量*/
	for(Uint32 y=(y1Start >> 1); y< ((y1Start + h1) >> 1); y++)
	{
		y1 =  arr_y1[y] ;
		y2 = arr_y2[y] ;
		fy1 = arr_fy1[y];
		fy2 = 1.0f - fy1;
		p1UV = pDstUV + y*pitch1 + ((x1Start >> 1) << 1);
		pSrcUV1 = pSrcUV + pitch0*y1;
		pSrcUV2 = pSrcUV + pitch0*y2;
		for(Uint32 x=(x1Start >> 1); x < ((x1Start + w1) >> 1); x++)
		{
			x1 = arr_x1[x];
			fx1 = arr_fx1[x];
			fx2 = 1.0f-fx1;
			float s1 = fx2*fy2;
			float s2 = fx1*fy2;
			float s3 = fx1*fy1;
			float s4 = fx2*fy1;
			Uint8* p11 = NULL;
			Uint8* p12 = NULL;
			Uint8* p21 = NULL;
			Uint8* p22 = NULL;
			/*处理U分量*/
			p11 = pSrcUV1 + (x1 << 1);
			p21 = pSrcUV2 + (x1 << 1);
#if 0
			if(((x1 << 1) + 2) >= (x1Start + w1 -1))
			{
				p12 = p11;
				p22 = p21;
			}
			else
#endif
			{
				p12 = p11 + 2;
				p22 = p21 + 2;
			}
			
			*p1UV = (Uint8)((*p11)*s1 + (*p12)*s2 + (*p21)*s4 + (*p22)*s3);
			p1UV++;
			/*处理V分量*/
			p11 = p11 + 1;
			p21 = p21 + 1;
#if 0
			if(((x1 << 1) + 3) >= (x1Start + w1 -1))
			{
				p12 = p11;
				p22 = p21;
			}
			else
#endif
			{
				p12 = p11 + 2;
				p22 = p21 + 2;
			}
			*p1UV = (Uint8)((*p11)*s1 + (*p12)*s2 + (*p21)*s4 + (*p22)*s3);
			p1UV++;
		}
	}
	
	delete []arr_x1;
	delete []arr_x2;
	delete []arr_fx1;
	
	delete []arr_y1;
	delete []arr_y2;
	delete []arr_fy1;
	
	return status;
}
#else
/*相邻法*/
static Int32 sclrLink_sclrProcess(SCLR_LINK_INFO *info,FRAME_Buf *src, FRAME_Buf *dst)
{

	if(NULL == info || NULL == src || NULL == dst)
	{
//		nslog(NS_ERROR,"para is NULL\n");
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;

	mfxFrameInfo& pSrcInfo = src->pSurfaces->Info;
	mfxFrameData& pSrcData = src->pSurfaces->Data;
	mfxFrameInfo& pDstInfo = dst->pSurfaces->Info;
	mfxFrameData& pDstData = dst->pSurfaces->Data;
	Uint32 x0Start= pSrcInfo.CropX;
	Uint32 y0Start = pSrcInfo.CropY;
	Uint32 w0 = pSrcInfo.CropW;
	Uint32 h0 = pSrcInfo.CropH;
	Uint32 pitch0 = pSrcData.Pitch;

	//pthread_mutex_lock(&info->lock);
	Uint32 x1Start= info->rtSclrInfo.dstX;
	Uint32 y1Start = info->rtSclrInfo.dstY;
	Uint32 w1 = info->rtSclrInfo.dstWidth;
	Uint32 h1 = info->rtSclrInfo.dstHeight;
	//pthread_mutex_unlock(&info->lock);

	Uint32 pitch1 = pDstData.Pitch;
	Uint8* pSrcY = pSrcData.Y;
	Uint8* pSrcY1 = pSrcData.Y;
	Uint8* pSrcUV = pSrcData.UV;
	Uint8* pSrcUV1 = pSrcData.UV;
	Uint8* pDstY = pDstData.Y;
	Uint8* pDstUV = pDstData.UV;
	Uint8* p1Y = NULL;
	Uint8* p1UV = NULL;
	float fw = float(w0-1) / (w1-1);
	float fh = float(h0-1) / (h1-1);
	float x0, y0;
	Uint32 y1, x1;

	if((x1Start + w1) > VIDEO_WIDTH || (y1Start + h1) > VIDEO_HEIGHT)
	{
//		nslog(NS_ERROR,"x:%d,y:%d,w:%d,h:%d,\n",x1Start,y1Start,w1,h1);
		return LINK_EFAIL;
	}

	Uint32* arr_Yx = new Uint32[VIDEO_WIDTH];
	Uint32* arr_Yy = new Uint32[VIDEO_HEIGHT];

	Uint32* arr_UVx = new Uint32[VIDEO_WIDTH];
	Uint32* arr_UVy = new Uint32[VIDEO_HEIGHT];

	if(arr_Yx == NULL || arr_Yy == NULL || arr_UVx == NULL || arr_UVy == NULL)
	{
		assert(0);
	}

	pDstInfo.CropX = x1Start;
	pDstInfo.CropY = y1Start;
	pDstInfo.CropW = w1;
	pDstInfo.CropH = h1;

	for(Uint32 x=0; x < (0 + w1); x++)
	{
		x0 = x*fw;
		arr_Yx[x] = int(x0 + 0.5);

		Uint32 tmpx = (x >> 1);
		x0 = tmpx*fw;
		arr_UVx[x] = int(x0 + 0.5);
	}

	for(Uint32 y=0; y< (0 + h1); y++)
	{
		y0 = y*fh;
		arr_Yy[y] = int(y0 + 0.5);
		if((y % 2) == 0)
		{
			Uint32 tmpY = (y >> 1);
			y0 = tmpY*fh;
			arr_UVy[tmpY] = int(y0 + 0.5);
		}
	}

	for(Uint32 y=y1Start; y< (y1Start + h1); y++)
	{
		y1 =  arr_Yy[y - y1Start] ;
		p1Y = pDstY + y*pitch1 + x1Start;
		pSrcY1 = pSrcY + pitch0*(y1 + y0Start) + x0Start;

		if(((y - y1Start) % 2) == 0)
		{
			Uint32 tmpY = ((y - y1Start) >> 1);
			y1 =  arr_UVy[tmpY] ;
			p1UV = pDstUV + (y >> 1)*pitch1 + x1Start;
			pSrcUV1 = pSrcUV + pitch0*(y1 + (y0Start >> 1)) + x0Start;
		}

		for(Uint32 x=x1Start; x < (x1Start + w1); x++)
		{
			x1 = arr_Yx[x - x1Start];

			Uint8* p11 = pSrcY1 + x1;
			*p1Y = (Uint8)(*p11);
			p1Y++;

			if(((y - y1Start)% 2) == 0)
			{
				x1 = arr_UVx[x - x1Start];

				p11 = pSrcUV1 + (x1 << 1);
				if(((x - x1Start)%2) != 0)
				{
					p11 = p11 + 1;
				}

				*p1UV = (Uint8)(*p11);
				p1UV++;
			}
		}

	}

	delete []arr_Yx;
	delete []arr_Yy;

	delete []arr_UVx;
	delete []arr_UVy;

	return status;
}
#endif
#else
static Int32 sclrLink_sclrProcess(FRAME_Buf *src, FRAME_Buf *dst,SCLR_CREATE_INFO *createInfo)
{
	if(NULL == createInfo || NULL == src || NULL == dst)
	{
		nslog(NS_ERROR,"para is NULL\n");
		return LINK_EFAIL;
	}
	Int32 status = LINK_SOK;
	int B,N,x,y;
	int i=0,j=0;
	int val;
	unsigned char *pA,*pB,*pC,*pD;
	unsigned char *pLinePrev,*pLineNext;
	unsigned char *pDest;

	if(yFlag==1)
	{
		for(i=0; i<dh; i++)
		{
			pDest=pDstBuf+i*ds;
			y=i*sh/dh;
			pLinePrev=pSrcBuf+y*ss;
			for(j=0; j<dw; j++)
			{
				x=j*sw/dw;
				pA=pLinePrev+x;
				*(pDest+j)=(unsigned char)*pA;
			}
		}
	}
	else
	{
		//uv缩放
		for(i=0; i<dh; i++)
		{
			pDest=pDstBuf+i*ds;
			y=i*sh/dh;
			pLinePrev=pSrcBuf+y*ss;
			for(j=0; j<dw; j++)
			{
				x=j*sw/dw;
				pA=pLinePrev+2*x;
				*(pDest+2*j)=(unsigned char)*pA;

				pA=pLinePrev+2*x+1;
				*(pDest+2*j+1)=(unsigned char)*pA;
			}
		}
	}

	return status;
}
#endif
static void* sclrLink_Process(void* arg)
{
	if(NULL == arg)
	{
//		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}
	
	Int32 status = LINK_SOK;
	FRAME_Buf *InFrame = NULL;
	FRAME_Buf *OutFrame = NULL;
	SCLR_LINK_INFO *info = (SCLR_LINK_INFO *)arg;
	
	//FILE *fpwrite = NULL;
	
	Uint32 framNum = 0;
	Uint32 startTime = 0;
	Uint32 CurrenTime = 0;
	
//	prctl(PR_SET_NAME, "sclrLink_Process", NULL, NULL, NULL);
#if 0
	if(info->createInfo.chNo == 0)
		fpwrite = fopen("sclr0.yuv","w");
#endif
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
		
		
		if(LINK_FALSE == info->EmpFullQueHndl.isConnect)
		{
			usleep(30);
			continue;
		}
		
		/*获取输入buff*/
		status = LINK_queGet(&(info->PreEmpFullQueHndl->pFullQue),(void **)(&InFrame),LINK_TIMEOUT_FOREVER);
		if(LINK_EFAIL == status)
		{
			usleep(10);
			continue;
		}
#if 0
		if(InFrame->channelNum != info->createInfo.chNo)
		{
			LINK_quePut(&(info->PreEmpFullQueHndl->pEmpQue),(void *)InFrame,LINK_TIMEOUT_NONE);
			usleep(2*1000);
			continue;
		}
#endif
		/*获取输出buff*/
		status = LINK_queGet(&(info->EmpFullQueHndl.pEmpQue),(void **)(&OutFrame),LINK_TIMEOUT_FOREVER);
		if(LINK_EFAIL == status)
		{
			usleep(10);
			continue;
		}
		
		if(LINK_TRUE == info->rtSclrInfoUpdate)
		{
			info->rtSclrInfo.dstX = info->createInfo.dstX;
			info->rtSclrInfo.dstY = info->createInfo.dstY;
			info->rtSclrInfo.dstWidth = info->createInfo.dstWidth;
			info->rtSclrInfo.dstHeight = info->createInfo.dstHeight;
		}
#if 0
		if(NULL != fpwrite)
		{
			printf("========%d,%d\n",InFrame->pSurfaces->Info.CropW,InFrame->pSurfaces->Info.CropH);
			fwrite(InFrame->pSurfaces->Data.Y,InFrame->pSurfaces->Info.CropW*InFrame->pSurfaces->Info.CropH,1,fpwrite);
			fwrite(InFrame->pSurfaces->Data.UV,InFrame->pSurfaces->Info.CropW*InFrame->pSurfaces->Info.CropH  / 2,1,fpwrite);
		}
#endif
		CurrenTime = link_get_run_time();
		status = sclrLink_sclrProcess(info,InFrame, OutFrame);
		if(LINK_EFAIL == status)
		{
//			nslog(NS_INFO,"=========sclrLink_sclrProcess is error!\n");
		}

		int d = link_get_run_time() - CurrenTime;
		printf("===========sclr process time:%d\n", link_get_run_time() - CurrenTime);

		/*还输出满包*/
		status = LINK_quePut(&(info->EmpFullQueHndl.pFullQue),(void *)OutFrame,LINK_TIMEOUT_NONE);
		if(LINK_EFAIL == status)
		{
			usleep(10);
			continue;
		}
		
		/*还输入空包*/
		status = LINK_quePut(&(info->PreEmpFullQueHndl->pEmpQue),(void *)InFrame,LINK_TIMEOUT_NONE);
		if(LINK_EFAIL == status)
		{
			usleep(10);
			continue;
		}
		
		framNum++;
		CurrenTime = link_get_run_time();
		if((CurrenTime - startTime) >= 5000)
		{
//			nslog(NS_INFO,"=========sclrLink_sclrProcess:fps:%d\n",framNum / 5);
			framNum = 0;
			startTime = link_get_run_time();
			
		}
		
	}
	
	LINK_tskDetach();
	LINK_tskExit(0);
	
	return NULL;
}


Int32 sclrLink_create(SCLR_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}
	
	info->exitTsk = LINK_FALSE;
	info->tskEnable = LINK_FALSE;
	info->rtSclrInfoUpdate = LINK_TRUE;
	info->PreEmpFullQueHndl = NULL;
	
	status = pthread_mutex_init(&info->lock, NULL);
//	if(status!=LINK_SOK)
//		nslog(NS_ERROR,"pthread_mutex_init is error\n");
		
	status = sclrLink_createQue(info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create que is error!\n");
		return LINK_EFAIL;
	}
	
	/*创建启动任务*/
	status = LINK_tskCreate(&info->tskHandle,sclrLink_Process,SCLRLINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create task is error!\n");
	}
	
	return status;
}

Int32 sclrLink_release(SCLR_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}
	
	info->exitTsk = LINK_TRUE;
	
	LINK_tskJoin(&info->tskHandle);
	
	return status;
}

Int32 sclrLink_enable(SCLR_LINK_INFO *info)
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

Int32 sclrLink_disable(SCLR_LINK_INFO *info)
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

void sclrLink_printf(SCLR_LINK_INFO *info)
{
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return;
	}
	
//	nslog(NS_INFO,"==============sclrLink param %d:==============\n",info->createInfo.chNo);
//	
// 	nslog(NS_INFO,
// 	      "tskEnable:%d\n"
// 	      "exitTsk:%d\n"
// 	      ,info->tskEnable,
// 	      info->exitTsk
// 	     );
// 	     
// 	nslog(NS_INFO,"=============sclrLink createInfo:===============\n");
// 	
// 	nslog(NS_INFO,
// 	      "dstX:%d\n"
// 	      "dstY:%d\n"
// 	      "dstWidth:%d\n"
// 	      "dstHeight:%d\n"
// 	      "queLen:%d\n"
// 	      ,info->createInfo.dstX,
// 	      info->createInfo.dstY,
// 	      info->createInfo.dstWidth,
// 	      info->createInfo.dstHeight,
// 	      info->createInfo.queLen
// 	     );
// 	     
// 	nslog(NS_INFO,"=============sclrLink AllocFramePara:===============\n");
// 	
// 	nslog(NS_INFO,
// 	      "nWidth:%d\n"
// 	      "nHeight:%d\n"
// 	      "nBuffNum:%d\n",
// 	      info->AllocFramePara.nSrcWidth,
// 	      info->AllocFramePara.nSrcHeight,
// 	      info->AllocFramePara.nBuffNum
// 	     );
// 	     
// 	nslog(NS_INFO,"\n=============sclrLink FrameQue:isConnect:%d============\n",info->EmpFullQueHndl.isConnect);
// 	
// 	nslog(NS_INFO,"pEmpQue:\n");
// 	nslog(NS_INFO,
// 	      "len:%d\n"
// 	      "count:%d\n",
// 	      info->EmpFullQueHndl.pEmpQue.len,
// 	      info->EmpFullQueHndl.pEmpQue.count
// 	     );
// 	     
// 	nslog(NS_INFO,"pFullQue:\n");
// 	nslog(NS_INFO,
// 	      "len:%d\n"
// 	      "count:%d\n",
// 	      info->EmpFullQueHndl.pFullQue.len,
// 	      info->EmpFullQueHndl.pFullQue.count
// 	     );
	     
	return;
}

Int32 sclrLink_setParam(SCLR_LINK_INFO *info,SCLR_SET_INFO *setPara)
{
	if(NULL == info || NULL == setPara)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}
	
	Int32 status = LINK_SOK;
	//pthread_mutex_lock(&info->lock);
	info->createInfo.dstX = setPara->dstX;
	info->createInfo.dstY = setPara->dstY;
	info->createInfo.dstWidth= setPara->dstWidth;
	info->createInfo.dstHeight= setPara->dstHeight;
	
	info->rtSclrInfoUpdate = LINK_TRUE;
	//pthread_mutex_unlock(&info->lock);
	return status;
}

Int32 sclrLink_connect(SCLR_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl)
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


