/*********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2008-2014 Intel Corporation. All Rights Reserved.

**********************************************************************************/

#include "linkCommon.h"

#include "mfx_samples_config.h"

#include "sample_vpp_utils.h"

#include "sample_vpp.h"


//获取毫秒级别
unsigned int vpp_get_run_time(void)
{
	unsigned int msec;
#ifndef WIN32
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	msec = tp.tv_sec;
	msec = msec * 1000 + tp.tv_nsec / 1000000;
#else
	return link_get_run_time();
#endif
	return msec;
}

void	vpp_set_run_time()
{
// 	SYSTEMTIME sys;
// 	GetLocalTime(&sys);
// 	iStartTime = sys.wDay * 24 * 3600 * 1000 + sys.wHour * 3600 * 1000 + sys.wMinute * 60 * 1000 + sys.wSecond * 1000 + sys.wMillisecond;
}


static int vppSetInputPara(sVppInputParams *Params,sVppUsrParams *usrParams)
{
	if(NULL == Params || NULL == usrParams)
	{
		printf("==========vppSetInputPara:params is NULL\n");
		return -1;
	}

	Params->inFrameInfo.CropX = usrParams->srcPos.CropX;
	Params->inFrameInfo.CropY = usrParams->srcPos.CropY;
	Params->inFrameInfo.CropW = usrParams->srcPos.CropW;
	Params->inFrameInfo.CropH= usrParams->srcPos.CropH;
	Params->inFrameInfo.nWidth= usrParams->srcPos.Width;
	Params->inFrameInfo.nHeight= usrParams->srcPos.Height;
	Params->inFrameInfo.FourCC = MFX_FOURCC_NV12;
	Params->inFrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	Params->inFrameInfo.dFrameRate = 30;

	Params->outFrameInfo.CropX = usrParams->dstPos.CropX;
	Params->outFrameInfo.CropY = usrParams->dstPos.CropY;
	Params->outFrameInfo.CropW = usrParams->dstPos.CropW;
	Params->outFrameInfo.CropH= usrParams->dstPos.CropH;
	Params->outFrameInfo.nWidth= usrParams->dstPos.Width;
	Params->outFrameInfo.nHeight= usrParams->dstPos.Height;
	Params->outFrameInfo.FourCC = MFX_FOURCC_NV12;
	Params->outFrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	Params->outFrameInfo.dFrameRate = 30;

	Params->memType = SYSTEM_MEMORY;
	Params->impLib = MFX_IMPL_HARDWARE;//MFX_IMPL_SOFTWARE;//
	Params->requestedFramesCount = 1;

	//printf("vppSetInputPara:%d,%d,%d,%d\n",Params->outFrameInfo.CropX,Params->outFrameInfo.CropY,Params->outFrameInfo.CropW,Params->outFrameInfo.CropH);
	
	return 0;
}

//Hardware vpp Initial
int API_IntelMedia_SDK_HW_VppInitial(void *lp)
{
	if(NULL == lp)
	{
		printf("==========API_IntelMedia_SDK_HW_VppInitial:lp is NULL\n");
		return -1;
	}

	int sts = 0;
	sVppInputParams        Params;
	sVppUsrParams *usrPara = (sVppUsrParams *)lp;

	sts  = vppSetInputPara(&Params,usrPara);
	if(-1 == sts )
	{
		printf("==========vppSetInputPara is error!\n");
		return -1;
	}

	//prepare mfxParams
	sts  = InitParamsVPP(&usrPara->appResources.VppParams, &Params);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);	

	  sts = ConfigVideoEnhancementFilters((void *)usrPara, &usrPara->appResources);
	  MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = InitResources(&usrPara->appResources, &usrPara->appResources.VppParams, &Params);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	PrintInfo(&Params,&usrPara->appResources.VppParams,&usrPara->appResources.Processor.mfxSession);

	return sts;
}

//Hardware vpp Release
int API_IntelMedia_SDK_HW_VppRelease(void *lp)
{
	if(NULL == lp)
	{
		printf("==========API_IntelMedia_SDK_HW_VppRelease:lp is NULL\n");
		return -1;
	}

	sVppUsrParams *usrPara = (sVppUsrParams *)lp;

	WipeResources(&usrPara->appResources);

	return 0;
}

//Hardware vpp Reset
int API_IntelMedia_SDK_HW_VppReset(void *lp)
{
	if(NULL == lp)
	{
		printf("==========API_IntelMedia_SDK_HW_VppReset:lp is NULL\n");
		return -1;
	}

	
#if 0
	int sts = 0;
	sts = API_IntelMedia_SDK_HW_VppRelease(lp);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);	
	sts = API_IntelMedia_SDK_HW_VppInitial(lp);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
#else
	int sts = 0;
	sVppInputParams        Params;
	sVppUsrParams *usrPara = (sVppUsrParams *)lp;

	sts  = vppSetInputPara(&Params,usrPara);
	if(-1 == sts )
	{
		printf("==========vppSetInputPara is error!\n");
		return -1;
	}

	//prepare mfxParams
	sts  = InitParamsVPP(&usrPara->appResources.VppParams, &Params);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);	

	//sts = ResetResources(&usrPara->appResources, &usrPara->appResources.VppParams, &Params);
	//MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	//PrintInfo(&Params,&usrPara->appResources.VppParams,&usrPara->appResources.Processor.mfxSession);
#endif
	return sts;
}

//Hardware vpp Frame
int API_IntelMedia_SDK_HW_VppProcessFrame(void *lp,void *buffInfo)
{
	if(NULL == lp || NULL == buffInfo)
	{
		printf("==========API_IntelMedia_SDK_HW_VppReset:lp is NULL\n");
		return -1;
	}

	int sts = 0;
	sVppUsrParams *usrPara = (sVppUsrParams *)lp;
	sVppUsrBuffParams *pBuffInfo = (sVppUsrBuffParams *)buffInfo;
	mfxSyncPoint        syncPoint;
	 mfxFrameSurface1 *pSurface = NULL;
	 int i;
	 mfxU8   *srcY,*srcUV;
	 mfxU8   *dstY,*dstUV;
	 mfxU16  srcCropX,srcCropY,srcPitch;
	 mfxU16  dstCropX,dstCropY,dstPitch;
	 mfxU16 CropW,CropH;
	 mfxU16 dstCropW,dstCropH;
	 unsigned int currentTime = 0;
	 

	if(NULL == pBuffInfo->pInSurfaces || NULL == pBuffInfo->pOutSurfaces)
	{
		printf("==========API_IntelMedia_SDK_HW_VppReset:Surfaces is NULL\n");
		return -1;
	}
	CropW = usrPara->appResources.VppParams.vpp.In.CropW;
	CropH = usrPara->appResources.VppParams.vpp.In.CropH;
	dstCropW = usrPara->appResources.VppParams.vpp.Out.CropW;
	dstCropH = usrPara->appResources.VppParams.vpp.Out.CropH;
#if 0	
	sts = GetFreeSurface(usrPara->appResources.Allocator.pSurfaces[VPP_IN],usrPara->appResources.Allocator.response[VPP_IN].NumFrameActual,&pSurface);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	//sts = GetFreeSurface(usrPara->appResources.Allocator.pSurfaces[VPP_OUT],usrPara->appResources.Allocator.response[VPP_OUT].NumFrameActual,&pOutSurface);
	//MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	srcCropX = pBuffInfo->pInSurfaces->Info.CropX;
	srcCropY = pBuffInfo->pInSurfaces->Info.CropY;
	srcPitch = pBuffInfo->pInSurfaces->Data.Pitch;

	dstCropX = pSurface->Info.CropX;
	dstCropY = pSurface->Info.CropY;
	dstPitch = pSurface->Data.Pitch;
	CropW = usrPara->appResources.VppParams.vpp.In.CropW;
	CropH = usrPara->appResources.VppParams.vpp.In.CropH;
	
	/*拷贝输入数据,注意，x，y，CropH必须是偶数*/
	for(i = 0; i < CropH; i++)
	{
		srcY = pBuffInfo->pInSurfaces->Data.Y + (i + srcCropY)*srcPitch + srcCropX;
		dstY =pSurface->Data.Y + (i + dstCropY)*dstPitch + dstCropX;
		memcpy(dstY,srcY,CropW);
	}
	
	for(i = 0; i < CropH / 2; i++)
	{
		/*注意这里不能使用Data.UV*/
		srcUV = pBuffInfo->pInSurfaces->Data.Y + srcPitch*1080 + (i + (srcCropY / 2))*srcPitch + srcCropX;
		dstUV =pSurface->Data.UV + (i + (dstCropY / 2 ))*dstPitch + dstCropX;
		memcpy(dstUV,srcUV,CropW);
	}
#endif
	
#if 0
	/*备份输入buff信息，以便vpp处理之后复原*/
	sVppDataInfo dataInfo;
	dataInfo.x = pBuffInfo->pInSurfaces->Info.CropX;
	dataInfo.y = pBuffInfo->pInSurfaces->Info.CropY;
	dataInfo.cropw= pBuffInfo->pInSurfaces->Info.CropW;
	dataInfo.croph = pBuffInfo->pInSurfaces->Info.CropH;
	dataInfo.w = pBuffInfo->pInSurfaces->Info.Width;
	dataInfo.h = pBuffInfo->pInSurfaces->Info.Height;
	dataInfo.pitch= pBuffInfo->pInSurfaces->Data.Pitch;
	
	/*把输入输出buff的分辨率信息修改为vpp设置的信息，不然vpp处理出错*/
	pBuffInfo->pInSurfaces->Data.Pitch = usrPara->appResources.VppParams.vpp.In.Width;
	pBuffInfo->pInSurfaces->Data.PitchLow = usrPara->appResources.VppParams.vpp.In.Width;
	pBuffInfo->pInSurfaces->Info.Width = usrPara->appResources.VppParams.vpp.In.Width;
	pBuffInfo->pInSurfaces->Info.Height = usrPara->appResources.VppParams.vpp.In.Height;
	pBuffInfo->pInSurfaces->Info.CropX= usrPara->appResources.VppParams.vpp.In.CropX;
	pBuffInfo->pInSurfaces->Info.CropY= usrPara->appResources.VppParams.vpp.In.CropY;
	pBuffInfo->pInSurfaces->Info.CropW= usrPara->appResources.VppParams.vpp.In.CropW;
	pBuffInfo->pInSurfaces->Info.CropH= usrPara->appResources.VppParams.vpp.In.CropH;
#endif

	pBuffInfo->pOutSurfaces->Info.CropX= usrPara->appResources.VppParams.vpp.Out.CropX;
	pBuffInfo->pOutSurfaces->Info.CropY= usrPara->appResources.VppParams.vpp.Out.CropY;
	pBuffInfo->pOutSurfaces->Info.CropW= usrPara->appResources.VppParams.vpp.Out.CropW;
	pBuffInfo->pOutSurfaces->Info.CropH= usrPara->appResources.VppParams.vpp.Out.CropH;

	pBuffInfo->pOutSurfaces->Data.Locked = 0;
	
	//currentTime = vpp_get_run_time();
	sts = usrPara->appResources.Processor.pmfxVPP->RunFrameVPPAsync( pBuffInfo->pInSurfaces, pBuffInfo->pOutSurfaces,
	                                            NULL,&syncPoint );
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = usrPara->appResources.Processor.mfxSession.SyncOperation(syncPoint, MSDK_VPP_WAIT_INTERVAL);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);	
	//printf("=========RunFrameVPPAsync time:%d,%dx%d,%dx%d\n",vpp_get_run_time() - currentTime,CropW,CropH,dstCropW,dstCropH);

	//printf("=========pOutSurfaces:x:%d,y:%d,w:%d,h:%d,locked:%d\n",pBuffInfo->pOutSurfaces->Info.CropX,pBuffInfo->pOutSurfaces->Info.CropY,
	//	pBuffInfo->pOutSurfaces->Info.CropW,pBuffInfo->pOutSurfaces->Info.CropH,pBuffInfo->pOutSurfaces->Data.Locked);

	return sts;
	
}


