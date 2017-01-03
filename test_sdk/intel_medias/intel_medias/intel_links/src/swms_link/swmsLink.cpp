#include "inc/enc_link/encLink.h"
#include "inc/swms_link/swmsLink.h"
#include "mfx_samples_config.h"
#include "pipeline_encode.h"
#include "pipeline_user.h"
#include "pipeline_region_encode.h"
#include "pipeline_decode.h"
#include "SDK_codec.h"

#define usleep(x)	Sleep(x/1000 == 0?1:x/1000)


Int32 swmsLink_setOsdWinParam(SWMS_LINK_INFO *info,AlgLink_OsdWindowPrm *Para,Int32 winId)
{
	if(NULL == info || NULL == Para)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	if(winId >= ALG_LINK_OSD_MAX_WINDOWS)
	{
//		nslog(NS_ERROR,"winId(%d) is error!\n",winId);
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;

	memcpy(&info->osdParams.osdCreateWinParams.winPrm[winId],Para,sizeof(AlgLink_OsdWindowPrm));
#if 0
	printf("====winId:%d,%p,%p,%p,%p,startX:%d,startY:%d,width:%d,height:%d,lineOffset:%d,globalAlpha:%d,transperencyEnable:%d,enableWin:%d,updateParm:%d\n",
		winId,
		info->osdParams.osdCreateWinParams.winPrm[winId].addr[0][0],
		info->osdParams.osdCreateWinParams.winPrm[winId].addr[0][1],
		info->osdParams.osdCreateWinParams.winPrm[winId].AlpBuff[0][0],
		info->osdParams.osdCreateWinParams.winPrm[winId].AlpBuff[0][1],
		info->osdParams.osdCreateWinParams.winPrm[winId].startX,
		info->osdParams.osdCreateWinParams.winPrm[winId].startY,
		info->osdParams.osdCreateWinParams.winPrm[winId].width,
		info->osdParams.osdCreateWinParams.winPrm[winId].height,
		info->osdParams.osdCreateWinParams.winPrm[winId].lineOffset,
		info->osdParams.osdCreateWinParams.winPrm[winId].globalAlpha,
		info->osdParams.osdCreateWinParams.winPrm[winId].transperencyEnable,
		info->osdParams.osdCreateWinParams.winPrm[winId].enableWin,
		info->osdParams.osdCreateWinParams.winPrm[winId].updateParm
		);
#endif
	return status;
}

#define SWOSD_ALPHA_QSHIFT         (7)
static inline void SWOSD_TI_algBlendLine
(
	Int64 *pInA,
	Int64 *pInB,
	Int64 *pAlp,
	Uint16 width,
	Bool transperencyEnable,
	Uint32 transperencyColor32,
	Uint32 globalAlpha32
)
{
	Int32 len;
	Uint8 *inA8, *inB8, *gain;
	Uint32 *pTmpB;
	Uint32 *pTmpAlp;

	len = (width)>>3;

	/*通过keycolor设置背景alpha值*/
	if(transperencyEnable)
	{
		Uint32 i;

		pTmpB   = (Uint32*)pInB;
		pTmpAlp = (Uint32*)pAlp;

		for(i=0; i<width/4; i++)
		{
			if(( *pTmpB ) == transperencyColor32)
				*pTmpAlp = ((globalAlpha32 << 24) + (globalAlpha32 << 16) + (globalAlpha32 << 8) + globalAlpha32);
			else
				*pTmpAlp = (((Uint32)0x7F << 24) + ((Uint32)0x7F << 16) + ((Uint32)0x7F << 8) + (Uint32)0x7F);

			pTmpAlp++;
			pTmpB++;
		}
	}

	while(len--)
	{
		inA8 = (Uint8*)pInA;
		gain = (Uint8*)pAlp;
		inB8 = (Uint8*)pInB;

		inA8[7] = inA8[7] + (Uint8)( ((inB8[7]-inA8[7])*gain[7]) >> SWOSD_ALPHA_QSHIFT );
		inA8[5] = inA8[5] + (Uint8)( ((inB8[5]-inA8[5])*gain[5]) >> SWOSD_ALPHA_QSHIFT );
		inA8[3] = inA8[3] + (Uint8)( ((inB8[3]-inA8[3])*gain[3]) >> SWOSD_ALPHA_QSHIFT );
		inA8[1] = inA8[1] + (Uint8)( ((inB8[1]-inA8[1])*gain[1]) >> SWOSD_ALPHA_QSHIFT );
		inA8[6] = inA8[6] + (Uint8)( ((inB8[6]-inA8[6])*gain[6]) >> SWOSD_ALPHA_QSHIFT );
		inA8[4] = inA8[4] + (Uint8)( ((inB8[4]-inA8[4])*gain[4]) >> SWOSD_ALPHA_QSHIFT );
		inA8[2] = inA8[2] + (Uint8)( ((inB8[2]-inA8[2])*gain[2]) >> SWOSD_ALPHA_QSHIFT );
		inA8[0] = inA8[0] + (Uint8)( ((inB8[0]-inA8[0])*gain[0]) >> SWOSD_ALPHA_QSHIFT );

		pInA++;
		pAlp++;
		pInB++;
	}
}

static Int32 AlgLink_OsdalgGetColorKey(Uint32 *colorKey, Uint32 dataFormat, Uint32 plane)
{
	Uint32 colorKeyY;
	Uint32 colorKeyU;
	Uint32 colorKeyV;
	Uint32 value;

	colorKeyY = (Uint8)colorKey[0];
	colorKeyU = (Uint8)colorKey[1];
	colorKeyV = (Uint8)colorKey[2];

	if(plane==0)
	{
	    value =
	         (colorKeyY <<0)
	        |(colorKeyY <<8)
	        |(colorKeyY <<16)
	        |(colorKeyY <<24)
	        ;
	}
	else
	{
	    value =
	         (colorKeyU <<0)
	        |(colorKeyV <<8)
	        |(colorKeyU <<16)
	        |(colorKeyV <<24)
	        ;
	}

	return value;
}

static Int32 swmsLink_osdProcess(SWMS_LINK_INFO *info, FRAME_Buf *frame)
{
	if(NULL == info || NULL == frame)
	{
//		nslog(NS_ERROR,"para is NULL\n");
		return LINK_EFAIL;
	}

	if(info->osdParams.osdCreateWinParams.numWindows > ALG_LINK_OSD_MAX_WINDOWS)
	{
//		nslog(NS_ERROR,"numWindows(%d) is error!\n",info->osdParams.osdCreateWinParams.numWindows);
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;
	mfxFrameInfo& pInfo = frame->pSurfaces->Info;
	mfxFrameData& pData = frame->pSurfaces->Data;
	Int32 winId = 0;

	Uint8 *pInA =  NULL;
	Uint8 *pInB =  NULL;
	Uint8 *pAlp =  NULL;
	Uint32 x,y,xStartA,yStartA,pitchA,pitchB,width,height;
	Uint32 algColorKey[2];


	Uint8* AlpData = new Uint8[VIDEO_WIDTH];
	pitchA = pData.Pitch;
	for(winId = 0; winId < info->osdParams.osdCreateWinParams.numWindows; winId++)
	{
		/*更新win param*/
		if(LINK_TRUE == info->osdParams.osdCreateWinParams.winPrm[winId].updateParm)
		{
			memcpy(&info->osdParams.winRtPrm[winId],&info->osdParams.osdCreateWinParams.winPrm[winId],sizeof(AlgLink_OsdWindowPrm));
			info->osdParams.osdCreateWinParams.winPrm[winId].updateParm = LINK_FALSE;
		}

		if(LINK_FALSE == info->osdParams.winRtPrm[winId].enableWin)
		{
			continue;
		}

		xStartA = info->osdParams.winRtPrm[winId].startX;
		yStartA = info->osdParams.winRtPrm[winId].startY;
		width = info->osdParams.winRtPrm[winId].width;
		height = info->osdParams.winRtPrm[winId].height;
		pitchB = info->osdParams.winRtPrm[winId].lineOffset;

		if((xStartA + width) > pInfo.CropW || (yStartA + height) > pInfo.CropH)
		{
//			nslog(NS_ERROR,"x(%d),y(%d),width(%d),height(%d),CropW(%d),CropH(%d)\n",xStartA,yStartA,width,height,pInfo.CropW,pInfo.CropH);
			continue;
		}

		algColorKey[0] = AlgLink_OsdalgGetColorKey(
		                    info->osdParams.winRtPrm[winId].colorKey,
		                    0,
		                    0
		                 );

		algColorKey[1] = AlgLink_OsdalgGetColorKey(
		                    info->osdParams.winRtPrm[winId].colorKey,
		                    0,
		                    1
		                 );

		/*处理y分量*/
		for(y = 0; y < height; y++)
		{
			pInA = (Uint8 *)(pData.Y + (y + yStartA)*pitchA + xStartA);
			pInB = (Uint8 *)(info->osdParams.winRtPrm[winId].addr[0][0] + y*pitchB);
			if(info->osdParams.winRtPrm[winId].AlpBuff[0][0] != NULL)
				pAlp = (Uint8 *)(info->osdParams.winRtPrm[winId].AlpBuff[0][0] + y*pitchB);
			else
				pAlp = AlpData;

			SWOSD_TI_algBlendLine
			(
				(Int64 *)pInA,
				(Int64 *)pInB,
				(Int64 *)pAlp,
				width,
				info->osdParams.winRtPrm[winId].transperencyEnable,
				algColorKey[0],
				info->osdParams.winRtPrm[winId].globalAlpha
			);
		}

		/*处理uv分量*/
		for(y = 0; y < (height >> 1); y++)
		{
			pInA = (Uint8 *)(pData.UV + (y + (yStartA >> 1))*pitchA + xStartA);
			pInB = (Uint8 *)(info->osdParams.winRtPrm[winId].addr[0][1] + y*pitchB);
			if(info->osdParams.winRtPrm[winId].AlpBuff[0][1] != NULL)
				pAlp = (Uint8 *)(info->osdParams.winRtPrm[winId].AlpBuff[0][1] + y*pitchB);
			else
				pAlp = AlpData;

			SWOSD_TI_algBlendLine
			(
				(Int64 *)pInA,
				(Int64 *)pInB,
				(Int64 *)pAlp,
				width,
				info->osdParams.winRtPrm[winId].transperencyEnable,
				algColorKey[1],
				info->osdParams.winRtPrm[winId].globalAlpha
			);
		}

	}

	delete []AlpData;

	return status;
}

static Int32 swmsLink_fillBlueFrame(FRAME_Buf *pFrame)
{
	if(NULL == pFrame)
	{
//		nslog(NS_ERROR,"pFrame is NULL\n");
		return LINK_EFAIL;
	}
	Int32 status = LINK_SOK;
	Uint32 size[2];
	Uint8 *pY, *pU, *pV;
	Int32 index;
	mfxFrameInfo& pInfo = pFrame->pSurfaces->Info;
	mfxFrameData& pData = pFrame->pSurfaces->Data;

	size[0] = (pData.Pitch) * (pInfo.Height);
	size[1] = (pData.Pitch) * ((pInfo.Height));

	pY = (Uint8 *)pData.Y;
	pU = (Uint8 *)pData.UV;
	pV = pU + 1;
	if(pY && size[0]){
		memset(pY, 0x0, size[0]);
	}


	for(index = 0; index < size[1] / 4; index++){
		*pU = 0x80;
		pU++;
		pU++;
	}
	for(index = 0; index < size[1] / 4; index++){
		*pV = 0x80;
		pV++;
		pV++;
	}

	pY = (Uint8 *)pData.Y;
	pU = (Uint8 *)pData.UV;
	pV = pU + 1;
	return status;
}

Int32 swmsLink_osdParaInit(AlgLink_OsdParams *Info)
{
	Int32 status = LINK_SOK;

	if(NULL == Info)
	{
//		nslog(NS_ERROR,"Info is NULL\n");
		return LINK_EFAIL;
	}

	Int32 winId = 0;
	Info->enableOsd = LINK_FALSE;
	Info->osdCreateWinParams.numWindows = 0;

	for(winId = 0; winId < ALG_LINK_OSD_MAX_WINDOWS; winId++)
	{
		memset(&Info->osdCreateWinParams.winPrm[winId],0,sizeof(AlgLink_OsdWindowPrm));
		Info->osdCreateWinParams.winPrm[winId].colorKey[0] = 0xFF;
		Info->osdCreateWinParams.winPrm[winId].colorKey[1] = 0xFF;
		Info->osdCreateWinParams.winPrm[winId].colorKey[2] = 0xFF;

		memset(&Info->winRtPrm[winId],0,sizeof(AlgLink_OsdWindowPrm));
		Info->winRtPrm[winId].colorKey[0] = 0xFF;
		Info->winRtPrm[winId].colorKey[1] = 0xFF;
		Info->winRtPrm[winId].colorKey[2] = 0xFF;

	}


	return status;
}

Int32 swmsLink_createParaInit(SWMS_CREATE_INFO *createInfo)
{
	Int32 status = LINK_SOK;

	if(NULL == createInfo)
	{
//		nslog(NS_ERROR,"createInfo is NULL\n");
		return LINK_EFAIL;
	}

    createInfo->queLen = SWMS_LINK_DEFAULT_ALLOC_BUFF_NUM;
    createInfo->maxOutRes = VSYS_STD_1080P_30;
    createInfo->layoutPrm.numWin = 1;
    createInfo->layoutPrm.outputFPS = 30;
    createInfo->layoutPrm.onlyCh2WinMapChanged = false;
    Int32 i = 0,j = 0;;
    for(i = 0; i < SYSTEM_SW_MS_MAX_WIN; i++)
    {
		createInfo->layoutPrm.winInfo[i].startX = 0;
		createInfo->layoutPrm.winInfo[i].startY = 0;
		createInfo->layoutPrm.winInfo[i].width = 1920;
		createInfo->layoutPrm.winInfo[i].height= 1080;
		createInfo->layoutPrm.winInfo[i].channelNum = 0;
		createInfo->layoutPrm.ch2WinMap[i] = -1;

		createInfo->vppUsrParams[i].srcPos.CropX= 0;
		createInfo->vppUsrParams[i].srcPos.CropY= 0;
		createInfo->vppUsrParams[i].srcPos.CropW= VIDEO_WIDTH;
		createInfo->vppUsrParams[i].srcPos.CropH= VIDEO_HEIGHT;
		createInfo->vppUsrParams[i].srcPos.Width= VIDEO_WIDTH;
		createInfo->vppUsrParams[i].srcPos.Height= (VIDEO_HEIGHT/* * 2*/);

		createInfo->vppUsrParams[i].dstPos.CropX= 0;
		createInfo->vppUsrParams[i].dstPos.CropY= 0;
		createInfo->vppUsrParams[i].dstPos.CropW= VIDEO_WIDTH;
		createInfo->vppUsrParams[i].dstPos.CropH= VIDEO_HEIGHT;
		createInfo->vppUsrParams[i].dstPos.Width= VIDEO_WIDTH;
		createInfo->vppUsrParams[i].dstPos.Height= VIDEO_HEIGHT;

		createInfo->vppUsrParams[i].compositionParam.mode = VPP_FILTER_DISABLED;
		createInfo->vppUsrParams[i].deinterlaceParam.mode = VPP_FILTER_DISABLED;
		createInfo->vppUsrParams[i].denoiseParam.mode = VPP_FILTER_DISABLED;
		createInfo->vppUsrParams[i].detailParam.mode = VPP_FILTER_DISABLED;
		createInfo->vppUsrParams[i].procampParam.mode = VPP_FILTER_DISABLED;
		createInfo->vppUsrParams[i].vaParam.mode = VPP_FILTER_DISABLED;
		createInfo->vppUsrParams[i].istabParam.mode = VPP_FILTER_DISABLED;
		createInfo->vppUsrParams[i].frcParam.mode = VPP_FILTER_DISABLED;
    }

    createInfo->timerPeriod = (1000/(createInfo->layoutPrm.outputFPS+(createInfo->layoutPrm.outputFPS/50)));


	return status;
}

static Int32 swmsLink_createQue(SWMS_LINK_INFO *info)
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

	if(info->createInfo.queLen > SWMS_LINK_MAX_ALLOC_BUFF_NUM || 0 == info->createInfo.queLen)
	{
		info->createInfo.queLen = SWMS_LINK_MAX_ALLOC_BUFF_NUM;
	}

	info->EmpFullQueHndl.isConnect = LINK_FALSE;

	/*创建空包队列*/
	status = LINK_queCreate(&(info->EmpFullQueHndl.pEmpQue),info->createInfo.queLen);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create emp que is error!\n");
		return LINK_EFAIL;
	}

    for(i = 0; i < SYSTEM_SW_MS_MAX_CH_ID; i++)
    {
        status = LINK_queCreate(&(info->chObj[i].inQue.pEmpQue),info->createInfo.queLen);
        if(LINK_EFAIL == status)
        {
//            nslog(NS_ERROR,"create emp que is error!\n");
            return LINK_EFAIL;
        }

        status = LINK_queCreate(&(info->chObj[i].inQue.pFullQue),info->createInfo.queLen);
        if(LINK_EFAIL == status)
        {
//            nslog(NS_ERROR,"create emp que is error!\n");
            return LINK_EFAIL;
        }
    }


	/*创建满包队列*/
	status = LINK_queCreate(&(info->EmpFullQueHndl.pFullQue),info->createInfo.queLen);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create full que is error!\n");
		return LINK_EFAIL;
	}

	API_IntelMedia_SDK_HWAllocParaInit((void *)&info->AllocFramePara);

	info->midFrameBuff.pSurfaces = NULL;
	info->midFrameBuff.pSurfaces = API_IntelMedia_SDK_HW_GetBuff((void *)&info->AllocFramePara);

	for(i = 0; i < info->createInfo.queLen; i++)
	{
		info->frameBuff[i].pSurfaces = NULL;

		info->frameBuff[i].pSurfaces = API_IntelMedia_SDK_HW_GetBuff((void *)&info->AllocFramePara);

		if(NULL == info->frameBuff[i].pSurfaces)
		{
//			nslog(NS_ERROR,"LINK_GetBuff[%d] is error!\n",i);
			return LINK_EFAIL;
		}

	    //info->frameBuff[i].channelNum = info->createInfo.chNo;
         info->frameBuff[i].channelNum = 0;


#if 0
		if(NULL != fSrcWrite)
		{
			fwrite(info->frameBuff[i].pSurfaces->Data.Y,info->frameBuff[i].pSurfaces->Data.Pitch * info->frameBuff[i].pSurfaces->Info.Height * 3 / 2,1, fSrcWrite);
		}
#endif
		//printf("=============%d,%p,%p\n",i,&(info->frameBuff[i]),info->frameBuff[i].pSurfaces);

         	swmsLink_fillBlueFrame(&info->frameBuff[i]);
		status = LINK_quePut(&(info->EmpFullQueHndl.pEmpQue),(void *)(&(info->frameBuff[i])),LINK_TIMEOUT_NONE);
		if(status != LINK_SOK)
		{
//			nslog(NS_ERROR,"LINK_quePut[%d] is error!\n",i);
			return LINK_EFAIL;
		}
	}

	return status;
}


static Int32 swmsLink_sclrProcessEx1(SWMS_LINK_INFO *info,Uint32 WinId,FRAME_Buf *src, FRAME_Buf *dst)
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

    Uint32 x1Start=  info->createInfo.layoutPrm.winInfo[WinId].startX;
	Uint32 y1Start = info->createInfo.layoutPrm.winInfo[WinId].startY;
	Uint32 w =      info->createInfo.layoutPrm.winInfo[WinId].width;
	Uint32 h =      info->createInfo.layoutPrm.winInfo[WinId].height;
	Uint32 pitch1 = pDstData.Pitch;


	int i = 0;
	int j = 0;
	unsigned char *Y =   pDstData.Y;
	unsigned char *UV =  pDstData.UV;
	unsigned char *srcY = pSrcData.Y;
	unsigned char *srcUV = pSrcData.UV;

   // w = 640;
  //  h = 360;

	int stepW = pSrcInfo.CropW*100/w;
	int stepH = pSrcInfo.CropH*100/h;
 //   nslog(NS_ERROR,"x:%d,y:%d,w:%d,h:%d,%d %d %d %d %d %d\n",x1Start,y1Start,w,h,pitch1,pitch0,stepW,stepH,w0,h0);
   // printf("%d %d %d %d\n",stepW,stepH,pSrcInfo.CropW,pSrcInfo.CropH);


   #if 0
   y1Start = 0;
   y1Start = 0;
   w= 1920;
   h = 1080;
   stepW = 100;
   stepH = 100;
   #endif
    for(i = 0; i< h; i++)
    {
        for(j = 0; j < w; j++)
        {
           Y [(i + y1Start)*pitch1 + (j+ x1Start)]            = srcY[i*stepH/100*pitch0 + stepW*j/100];
           UV[(i + y1Start)/2*pitch1 + (j+ x1Start)/2*2]      = srcUV[i/2*stepH/100*pitch0 + j/2*stepW/100*2];
           UV[(i + y1Start)/2*pitch1 + (j+ x1Start)/2*2 + 1]  = srcUV[i/2*stepH/100*pitch0 + j/2*stepW/100*2 + 1];


        //   UV[(i + y1Start)/2*pitch1 + (j+ x1Start)/2*2]      = srcUV[(stepH/100*i/2)*pitch0 + (stepW/100*j)/2*2];
        //   UV[(i + y1Start)/2*pitch1 + (j+ x1Start)/2*2 + 1]  = srcUV[(stepH/100*i/2)*pitch0 + (stepW/100*j)/2*2 + 1];

         //  UV[(i + y1Start)/2*pitch1 + (j+ x1Start)/2*2]      = srcUV[i/2*pitch1 + j/2*2];
        //   UV[(i + y1Start)/2*pitch1 + (j+ x1Start)/2*2 + 1]  = srcUV[i/2*pitch1 + j/2*2 + 1];
           //  Y [(i + y1Start)*pitch1 + (j+ x1Start)]          = 0x36;
           // UV[(i + y1Start)/2*pitch1 + (j+ x1Start)/2*2]     = 0xc0;
           // UV[(i + y1Start)/2*pitch1 + (j+ x1Start)/2*2 + 1] = 0x70;
        }
    }

    return  status;
}



static Int32 swmsLink_sclrProcessEx(SWMS_LINK_INFO *info,Uint32 WinId,FRAME_Buf *src, FRAME_Buf *dst)
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

	Uint32 x1Start=  info->createInfo.layoutPrm.winInfo[WinId].startX;
	Uint32 y1Start = info->createInfo.layoutPrm.winInfo[WinId].startY;
	Uint32 w1 =      info->createInfo.layoutPrm.winInfo[WinId].width;
	Uint32 h1 =      info->createInfo.layoutPrm.winInfo[WinId].height;
	Uint32 pitch1 = pDstData.Pitch;

	Uint8* pSrcY = pSrcData.Y;
	Uint8* pSrcY1 = pSrcData.Y;
	Uint8* pSrcUV = pSrcData.UV;//
	Uint8* pSrcUV1 = pSrcData.UV;//
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

static Int32 swmsLink_sclrProcess(SWMS_LINK_INFO *info,Uint32 WinId,FRAME_Buf *src, FRAME_Buf *dst)

{

	if(NULL == info || NULL == dst || NULL == src || WinId < 0 || WinId >= SYSTEM_SW_MS_MAX_WIN)
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

	Uint32 x1Start=  info->createInfo.layoutPrm.winInfo[WinId].startX;
	Uint32 y1Start = info->createInfo.layoutPrm.winInfo[WinId].startY;
	Uint32 w1 =      info->createInfo.layoutPrm.winInfo[WinId].width;
	Uint32 h1 =      info->createInfo.layoutPrm.winInfo[WinId].height;


	Uint32 pitch1 = pDstData.Pitch;
	Uint8* pSrcY = pSrcData.Y;
	Uint8* pSrcUV = pSrcData.UV;
	Uint8* pDstY = pDstData.Y;
	Uint8* pDstUV = pDstData.UV;
	Uint8* p1Y = NULL;
	Uint8* p1UV = NULL;
	float fw = float(w0-1) / (w1-1);
	float fh = float(h0-1) / (h1-1);
	float x0, y0;
	Uint32 y1, y2, x1, x2;
	float fx1, fx2, fy1, fy2;
	Uint32* arr_x1 = new Uint32[w1];
	Uint32* arr_x2 = new Uint32[w1];
	float* arr_fx1 = new float[w1];
	Uint32* arr_y1 = new Uint32[h1];
	Uint32* arr_y2 = new Uint32[h1];
	float* arr_fy1 = new float[h1];

	if((x1Start + w1) > VIDEO_WIDTH || (y1Start + h1) > VIDEO_HEIGHT)
	{
//		nslog(NS_ERROR,"x:%d,y:%d,w:%d,h:%d,\n",x1Start,y1Start,w1,h1);
		return LINK_EFAIL;
	}

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

			Uint8* p11 = pSrcY + pitch0*y1 + x1;
			Uint8* p12 = pSrcY + pitch0*y1 + x2;
			Uint8* p21 = pSrcY + pitch0*y2 + x1;
			Uint8* p22 = pSrcY + pitch0*y2 + x2;
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
			p11 = pSrcUV + pitch0*y1 + (x1 << 1);
			p21 = pSrcUV + pitch0*y2 + (x1 << 1);
			if(((x1 << 1) + 2) >= (x1Start + w1 -1))
			{
				p12 = p11;
				p22 = p21;
			}
			else
			{
				p12 = p11 + 2;
				p22 = p21 + 2;
			}

			*p1UV = (Uint8)((*p11)*s1 + (*p12)*s2 + (*p21)*s4 + (*p22)*s3);
			p1UV++;
          		/*处理V分量*/
			p11 = pSrcUV + pitch0*y1 + (x1 << 1) + 1;
			p21 = pSrcUV + pitch0*y2 + (x1 << 1) + 1;
			if(((x1 << 1) + 3) >= (x1Start + w1 -1))
			{
				p12 = p11;
				p22 = p21;
			}
			else
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
/*src和dst的buff必须是相同大小的*/
static Int32 swmsLink_copy(mfxFrameSurface1 *src,mfxFrameSurface1 *dst)
{
	if(NULL == src || NULL == dst)
	{
//		nslog(NS_ERROR,"para is NULL\n");
		return LINK_EFAIL;
	}
	mfxFrameInfo& srcInfo = src->Info;
	mfxFrameData& srcData = src->Data;
	mfxFrameInfo& dstInfo = dst->Info;
	mfxFrameData& dstData = dst->Data;
	mfxU8 *srcY,*srcUV,*dstY,*dstUV;
	Int32 i,j;

	for(i = 0; i < ((srcInfo.CropH) >> 1); i++)
	{
		srcY = srcData.Y + (srcInfo.CropY + (i << 1)) * (srcData.Pitch) + srcInfo.CropX;
		dstY = dstData.Y + (srcInfo.CropY + (i << 1)) * (dstData.Pitch) + srcInfo.CropX;
		memcpy(dstY,srcY,srcInfo.CropW);
		srcY = srcData.Y + (srcInfo.CropY + (i << 1) + 1) * (srcData.Pitch) + srcInfo.CropX;
		dstY = dstData.Y + (srcInfo.CropY + (i << 1) + 1) * (dstData.Pitch) + srcInfo.CropX;
		memcpy(dstY,srcY,srcInfo.CropW);

		srcUV = srcData.UV + (((srcInfo.CropY) >> 1) + i) * (srcData.Pitch) + srcInfo.CropX;
		dstUV = dstData.UV + (((srcInfo.CropY) >> 1) + i) * (dstData.Pitch) + srcInfo.CropX;
		memcpy(dstUV,srcUV,srcInfo.CropW);
	}

	return LINK_SOK;
}

static int s_count = 0;
static FILE *writefile = NULL;
#if 1
static Int32 swmsLink_swmsProcess(SWMS_LINK_INFO *info, FRAME_Buf *dst)
{

	if(NULL == info || NULL == dst)
	{
//		nslog(NS_ERROR,"para is NULL\n");
		return LINK_EFAIL;
	}

    Int32 status;
    Uint32 WinId = 0;
    Uint32 Chid = 0;
    sVppUsrBuffParams buffParams;
    mfxFrameInfo    *pFrameInfo = NULL;
    Bool needResetVpp = LINK_FALSE;
    Uint32 CurrenTime = 0;

    if(info->createInfo.rtlayoutPrm.onlyCh2WinMapChanged == true)
    {
      //  memcpy(&info->createInfo.layoutPrm, &info->createInfo.rtlayoutPrm,sizeof(SwMsLink_LayoutPrm));
        info->createInfo.rtlayoutPrm.numWin = info->createInfo.layoutPrm.numWin;

        //初始化映射
        for(Chid = 0; Chid < SYSTEM_SW_MS_MAX_CH_ID; Chid++)
        {
            info->createInfo.rtlayoutPrm.ch2WinMap[Chid] = -1;
        }

        //更新布局信息
        for(WinId = 0; WinId < SYSTEM_SW_MS_MAX_WIN;WinId++)
        {

			if (WinId < info->createInfo.rtlayoutPrm.numWin)
            {
                info->createInfo.rtlayoutPrm.winInfo[WinId] = info->createInfo.layoutPrm.winInfo[WinId];
            }
            else
            {
                info->createInfo.rtlayoutPrm.winInfo[WinId].channelNum = -1;
            }

            Chid = info->createInfo.rtlayoutPrm.winInfo[WinId].channelNum;
            if(Chid < SYSTEM_SW_MS_MAX_CH_ID)
            {
                info->createInfo.rtlayoutPrm.ch2WinMap[Chid] = WinId;
            }

        }

        for(Int32 i = 0; i < info->createInfo.queLen; i++)
        {
              swmsLink_fillBlueFrame(&info->frameBuff[i]);
        }


        info->createInfo.rtlayoutPrm.onlyCh2WinMapChanged = false;
    }

    for(Chid = 0; Chid < SYSTEM_SW_MS_MAX_CH_ID; Chid++)
    {
        WinId = info->createInfo.rtlayoutPrm.ch2WinMap[Chid];
        FRAME_Buf *InFrame = NULL;
		if (WinId >= info->createInfo.rtlayoutPrm.numWin )
        {
            LINK_queGet(&(info->chObj[Chid].inQue.pFullQue),(void **)(&InFrame),LINK_TIMEOUT_NONE);
            if(InFrame)
            {
                if(info->chObj[Chid].pCurInFrame != NULL)
                {
                       LINK_quePut(&(info->PreEmpFullQueHndl->pEmpQue),(void *)info->chObj[Chid].pCurInFrame,LINK_TIMEOUT_FOREVER);
                }
                info->chObj[Chid].pCurInFrame = InFrame;
            }

           continue;

        }

        LINK_queGet(&(info->chObj[Chid].inQue.pFullQue),(void **)(&InFrame),LINK_TIMEOUT_NONE);
        if(InFrame)
        {

          // swmsLink_sclrProcessEx(info, WinId,InFrame, dst);

            if(info->chObj[Chid].pCurInFrame != NULL)
            {
                   LINK_quePut(&(info->PreEmpFullQueHndl->pEmpQue),(void *)info->chObj[Chid].pCurInFrame,LINK_TIMEOUT_FOREVER);
            }

            info->chObj[Chid].pCurInFrame = InFrame;
        }
        else
        {
            if( info->chObj[Chid].pCurInFrame != NULL)
            {
            	//swmsLink_sclrProcessEx(info, WinId, info->chObj[Chid].pCurInFrame, dst);
            }
        }

    }
	//CurrenTime = link_get_run_time();
	#if 1
    for(WinId= 0; WinId < info->createInfo.rtlayoutPrm.numWin; WinId++)
    {
        Chid  =  info->createInfo.rtlayoutPrm.winInfo[WinId].channelNum;
		if (info->chObj[Chid].pCurInFrame != NULL && info->createInfo.layoutPrm.winInfo[WinId].width != 0)
        {
	            #if 1
			buffParams.pInSurfaces = info->chObj[Chid].pCurInFrame->pSurfaces;
			buffParams.pOutSurfaces = info->midFrameBuff.pSurfaces;//dst->pSurfaces;
			pFrameInfo = &buffParams.pInSurfaces->Info;

			needResetVpp = LINK_FALSE;

			if((info->createInfo.rtlayoutPrm.winInfo[WinId].startX != info->createInfo.vppUsrParams[0].dstPos.CropX)
				||(info->createInfo.rtlayoutPrm.winInfo[WinId].startY != info->createInfo.vppUsrParams[0].dstPos.CropY)
				||(info->createInfo.rtlayoutPrm.winInfo[WinId].width != info->createInfo.vppUsrParams[0].dstPos.CropW)
				||(info->createInfo.rtlayoutPrm.winInfo[WinId].height != info->createInfo.vppUsrParams[0].dstPos.CropH))
			{
				info->createInfo.vppUsrParams[0].dstPos.CropX =  info->createInfo.rtlayoutPrm.winInfo[WinId].startX;
				info->createInfo.vppUsrParams[0].dstPos.CropY =  info->createInfo.rtlayoutPrm.winInfo[WinId].startY;
				info->createInfo.vppUsrParams[0].dstPos.CropW =  info->createInfo.rtlayoutPrm.winInfo[WinId].width;
				info->createInfo.vppUsrParams[0].dstPos.CropH =  info->createInfo.rtlayoutPrm.winInfo[WinId].height;

				needResetVpp = LINK_TRUE;
			}

			if((pFrameInfo->CropX != info->createInfo.vppUsrParams[0].srcPos.CropX)
				||(pFrameInfo->CropY != info->createInfo.vppUsrParams[0].srcPos.CropY)
				||(pFrameInfo->CropW != info->createInfo.vppUsrParams[0].srcPos.CropW)
				||(pFrameInfo->CropH != info->createInfo.vppUsrParams[0].srcPos.CropH))
			{
				info->createInfo.vppUsrParams[0].srcPos.CropX= pFrameInfo->CropX;
				info->createInfo.vppUsrParams[0].srcPos.CropY= pFrameInfo->CropY;
				info->createInfo.vppUsrParams[0].srcPos.CropW= pFrameInfo->CropW;
				info->createInfo.vppUsrParams[0].srcPos.CropH= pFrameInfo->CropH;

				needResetVpp = LINK_TRUE;
			}
#if 0
			/*参数有变化需要reset vpp*/
			if(LINK_TRUE == needResetVpp)
			{

				status = API_IntelMedia_SDK_HW_VppReset((void *)&info->createInfo.vppUsrParams[0]);
				if(status != 0)
				{
//					nslog(NS_INFO,"=========winId:%d,API_IntelMedia_SDK_HW_VppReset is error!\n",WinId);
					printf("=========winId:%d,API_IntelMedia_SDK_HW_VppReset is error!\n",WinId);
				}

			}

			status = API_IntelMedia_SDK_HW_VppProcessFrame((void *)&info->createInfo.vppUsrParams[0],(void *)&buffParams);
			if(0 != status)
			{
//				nslog(NS_INFO,"=========API_IntelMedia_SDK_HW_VppProcessFrame is error!\n");
				printf("=========API_IntelMedia_SDK_HW_VppProcessFrame[%d] is error!\n",WinId);
			}

			if(buffParams.pOutSurfaces->Data.Locked > 0)
				printf("=================lock:%d\n",buffParams.pOutSurfaces->Data.Locked);

			/*由于vpp缩放出来会把前面缩放好的使用黑屏覆盖，所以这里需要拷贝到目的输出buff*/
			swmsLink_copy(buffParams.pOutSurfaces,dst->pSurfaces);
#else
			if (WinId > 4)
			{
				int a = 0;
			}
		swmsLink_sclrProcessEx(info, WinId, info->chObj[Chid].pCurInFrame, dst);
		if(300 < s_count && s_count< 310)
		{
			writefile = fopen("comp.yuv","a");
			if(NULL != writefile)
			{
				fwrite(buffParams.pOutSurfaces->Data.Y,1,1920*1080,writefile);
				fwrite(buffParams.pOutSurfaces->Data.UV,1,1920*1080/2,writefile);
				fclose(writefile);
				writefile = NULL;
			}
		}
		s_count++;
#endif
			#else
			swmsLink_sclrProcessEx(info, WinId,info->chObj[Chid].pCurInFrame, dst);
			#endif
        }

    }
	//printf("===========API_IntelMedia_SDK_HW_VppProcessFrame time:%d,numWin:%d\n",link_get_run_time() - CurrenTime,info->createInfo.rtlayoutPrm.numWin);
	#endif
    return LINK_SOK;
}


#else
static Int32 swmsLink_sclrProcess(FRAME_Buf *src, FRAME_Buf *dst,SWMS_CREATE_INFO *createInfo)
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
		for(i=0;i<dh;i++)
		{
			pDest=pDstBuf+i*ds;
			y=i*sh/dh;
			pLinePrev=pSrcBuf+y*ss;
			for(j=0;j<dw;j++)
			{
				x=j*sw/dw;
				pA=pLinePrev+x;
				*(pDest+j)=(unsigned char)*pA;
			}
		}
	}
	else
	{//uv缩放
		for(i=0;i<dh;i++)
		{
			pDest=pDstBuf+i*ds;
			y=i*sh/dh;
			pLinePrev=pSrcBuf+y*ss;
			for(j=0;j<dw;j++)
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
static void* swmsLinkLink_GetProcessData(void* arg)
{
	if(NULL == arg)
	{
//		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}

	Int32 status = LINK_SOK;
	FRAME_Buf *InFrame = NULL;
	FRAME_Buf *OutFrame = NULL;
	SWMS_LINK_INFO *info = (SWMS_LINK_INFO *)arg;

	Uint32 framNum = 0;
	Uint32 startTime = 0;
	Uint32 CurrenTime = 0;
//	prctl(PR_SET_NAME, "swmsLink_GPD", NULL, NULL, NULL);
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
         if(InFrame->channelNum >= SYSTEM_SW_MS_MAX_CH_ID)
         {
              LINK_quePut(&(info->PreEmpFullQueHndl->pEmpQue),(void *)InFrame,LINK_TIMEOUT_FOREVER);
              continue;
         }

         #if 0
         //通道映射窗口
         Int32 WinId = info->createInfo.layoutPrm.ch2WinMap[InFrame->channelNum];
         if(WinId >= SYSTEM_SW_MS_MAX_WIN || WinId < 0)
         {
              LINK_quePut(&(info->PreEmpFullQueHndl->pEmpQue),(void *)InFrame,LINK_TIMEOUT_FOREVER);
              continue;
         }
         #endif

        // printf("InFrame->channelNum = %d %d\n",InFrame->channelNum,WinId);
         //LINK_quePut(&(info->PreEmpFullQueHndl->pEmpQue),(void *)InFrame,LINK_TIMEOUT_FOREVER);
         //continue;
    		/*还输出满包*/
    		status = LINK_quePut(&(info->chObj[InFrame->channelNum].inQue.pFullQue),(void *)InFrame,LINK_TIMEOUT_FOREVER);
	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}


static void* SwMsLink_tskRun(void* arg)
{
	if(NULL == arg)
	{
//		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}
//	prctl(PR_SET_NAME, "SwmsLink_Process", NULL, NULL, NULL);
	Int32 status = LINK_SOK;
	FRAME_Buf *InFrame = NULL;
	FRAME_Buf *OutFrame = NULL;
	SWMS_LINK_INFO *info = (SWMS_LINK_INFO *)arg;
     Uint32 *msg = NULL;

	Uint32 framNum = 0;
	Uint32 startTime = 0;
	Uint32 CurrenTime = 0;

	while(!(info->exitTsk))
	{
		if(LINK_FALSE == info->tskEnable)
		{
			usleep(30*1000);
			continue;
		}

        	/*获取处理消息*/
		status = LINK_queGet(&(info->MsgQueHandle.pFullQue),(void **)(&msg),LINK_TIMEOUT_FOREVER);
		if(LINK_EFAIL == status)
		{
			continue;
		}

		/*还处理消息*/
		status = LINK_quePut(&(info->MsgQueHandle.pEmpQue),(void *)msg,LINK_TIMEOUT_FOREVER);
		if(LINK_EFAIL == status)
		{

			continue;
		}

		if(LINK_FALSE == info->EmpFullQueHndl.isConnect)
		{
			usleep(30*1000);
			continue;
		}

		if((NULL == info->PreEmpFullQueHndl) || (LINK_FALSE == info->PreEmpFullQueHndl->isConnect))
		{
			usleep(30*1000);
			continue;
		}

		if(LINK_FALSE == info->EmpFullQueHndl.isConnect)
		{
			usleep(30*1000);
			continue;
		}

		/*获取输出buff*/
		status = LINK_queGet(&(info->EmpFullQueHndl.pEmpQue),(void **)(&OutFrame),LINK_TIMEOUT_FOREVER);
		if(LINK_EFAIL == status)
		{
			continue;
		}
		//CurrenTime = link_get_run_time();
		status = swmsLink_swmsProcess(info, OutFrame);

		if(LINK_EFAIL == status)
		{
//			nslog(NS_WARN,"=========swmsLink_sclrProcess is error!\n");
		}
		//printf("===========sclr process time:%d\n",link_get_run_time() - CurrenTime);
		if(LINK_TRUE == info->osdParams.enableOsd)
		{
			status = swmsLink_osdProcess(info, OutFrame);
			if(LINK_EFAIL == status)
			{
//				nslog(NS_INFO,"=========swmsLink_osdProcess is error!\n");
			}
		}

		/*还输出满包*/
		status = LINK_quePut(&(info->EmpFullQueHndl.pFullQue),(void *)OutFrame,LINK_TIMEOUT_FOREVER);
         	framNum++;
		CurrenTime = link_get_run_time();
		if((CurrenTime - startTime) >= 5000)
		{
//			nslog(NS_INFO,"=========swmsLink_sclrProcess:fps:%d\n",framNum / 5);
			framNum = 0;
			startTime = link_get_run_time();

		}
	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}

static void swmsLink_ms_delay(Uint32 timerPeriod)
{

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = timerPeriod * 1000;

	if(-1 == select(0, NULL, NULL, NULL, &tv)) {
//		nslog(NS_ERROR,"[ms_timer] select : %s", strerror(errno));
	}
}

static void* swmsLink_periodFun(void* arg)
{
	SWMS_LINK_INFO *pPara = (SWMS_LINK_INFO *)arg;
	Int32 *Msg = NULL;
//	prctl(PR_SET_NAME, "swmsLink_periodFun", NULL, NULL, NULL);
	while(1)
	{
		swmsLink_ms_delay(pPara->createInfo.timerPeriod);

		if(LINK_TRUE == pPara->tskEnable)
		{
			LINK_queGet(&(pPara->MsgQueHandle.pEmpQue),(void **)&Msg,LINK_TIMEOUT_FOREVER);

			LINK_quePut(&(pPara->MsgQueHandle.pFullQue),(void *)Msg,LINK_TIMEOUT_NONE);
		}
	}
}


static Int32 swmsLink_createTimer(SWMS_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	Int32 i = 0;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	/*创建空包队列*/
	status = LINK_queCreate(&(info->MsgQueHandle.pEmpQue),SWMS_LINK_MSG_NUM);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create emp que is error!\n");
		return LINK_EFAIL;
	}
	/*创建满包队列*/
	status = LINK_queCreate(&(info->MsgQueHandle.pFullQue),SWMS_LINK_MSG_NUM);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create full que is error!\n");
		return LINK_EFAIL;
	}

	for(i = 0; i < SWMS_LINK_MSG_NUM; i++)
	{
		status = LINK_quePut(&(info->MsgQueHandle.pEmpQue),(void *)(&(info->Msg[i])),LINK_TIMEOUT_NONE);
		if(status != LINK_SOK)
		{
//			nslog(NS_ERROR,"LINK_quePut[%d] is error!\n",i);
			return LINK_EFAIL;
		}
	}

	/*创建任务*/
	status = LINK_tskCreate(&info->tskHandle_msg,swmsLink_periodFun,SWMSLINK_TSK_TIMER_PRI,0,(void *)info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create task is error!\n");
	}

	return status;
}

Int32 SwMsLink_drvSwitchLayout(SWMS_LINK_INFO *info, SwMsLink_LayoutPrm *pram)
{
    if(NULL == info  || NULL == pram)
    {
        return LINK_EFAIL;
    }

    memcpy(&info->createInfo.layoutPrm, pram, sizeof(SwMsLink_LayoutPrm));
    info->createInfo.rtlayoutPrm.onlyCh2WinMapChanged = true;
	return LINK_SOK;
}

Int32 swmsLink_create(SWMS_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->exitTsk = LINK_FALSE;
	info->tskEnable = LINK_FALSE;
	info->PreEmpFullQueHndl = NULL;

	status = pthread_mutex_init(&info->lock, NULL);
// 	if(status!=LINK_SOK)
// 		nslog(NS_ERROR,"pthread_mutex_init is error\n");

    //创建队列
	status = swmsLink_createQue(info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create que is error!\n");
		return LINK_EFAIL;
	}

	/*初始化vpp算法*/
	Int32 i = 0;
	for(i = 0; i < 1/*SYSTEM_SW_MS_MAX_WIN*/; i++)
	{
		status = API_IntelMedia_SDK_HW_VppInitial((void *)&info->createInfo.vppUsrParams[i]);
		if(status!=LINK_SOK)
		{
//			nslog(NS_ERROR,"swms winId[%d] API_IntelMedia_SDK_HW_VppInitial is error\n",i);
			return LINK_EFAIL;
		}
	}

    /*创建buf分配任务*/
	status = LINK_tskCreate(&info->tskHandle,SwMsLink_tskRun,SWMSLINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create task is error!\n");
	}

	/*创建处理任务*/
	status = LINK_tskCreate(&info->tskHandle,swmsLinkLink_GetProcessData,SWMSLINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create task is error!\n");
	}


     swmsLink_createTimer(info);

	return status;
}

Int32 swmsLink_release(SWMS_LINK_INFO *info)
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
	API_IntelMedia_SDK_HW_VppRelease((void *)&info->createInfo.vppUsrParams[0]);
	return LINK_TRUE;
}

Int32 swmsLink_enable(SWMS_LINK_INFO *info)
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

Int32 swmsLink_disable(SWMS_LINK_INFO *info)
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

void swmsLink_printf(SWMS_LINK_INFO *info)
{
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return;
	}

	//nslog(NS_INFO,"==============swmsLink param %d:==============\n",info->createInfo.chNo);

	return;
}

Int32 swmsLink_setParam(SWMS_LINK_INFO *info,SWMS_SET_INFO *setPara)
{
	if(NULL == info || NULL == setPara)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;
#if 0
	pthread_mutex_lock(&info->lock);
	info->createInfo.dstX = setPara->dstX;
	info->createInfo.dstY = setPara->dstY;
	info->createInfo.dstWidth= setPara->dstWidth;
	info->createInfo.dstHeight= setPara->dstHeight;
	pthread_mutex_unlock(&info->lock);
#endif
	return status;
}

Int32 swmsLink_connect(SWMS_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl)
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


