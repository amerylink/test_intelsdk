/*********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2005-2014 Intel Corporation. All Rights Reserved.

**********************************************************************************/

#include "mfx_samples_config.h"

#include <stdarg.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <memory>
#include "pipeline_encode.h"
#include "pipeline_user.h"
#include "pipeline_region_encode.h"
#include "pipeline_decode.h"
#include "SDK_codec.h"
#include "linkVideo.h"
#include "linkCommon.h"
#if 0
//Hardware GetBuff
mfxFrameSurface1* API_IntelMedia_SDK_HW_GetBuff(void *lp)
{
	int i = 0;
	if(lp==NULL)
	{
        	SDK_Codec_Debug();
	}

	sAPI_EncoderInputParams *pEncoderPar = (sAPI_EncoderInputParams *)lp;
	assert(pEncoderPar);
	assert(pEncoderPar->pHandle);

	CEncodingPipeline  *pPipeline = (CEncodingPipeline  *)pEncoderPar->pHandle;
	
	for ( i = 0; i < pPipeline->m_EncResponse.NumFrameActual; i++)
	{
		if(0 == pPipeline->m_pEncSurfaces[i].Data.Locked)
		{
			pPipeline->m_pEncSurfaces[i].Data.Locked = 2;
			break;
		}
	}

	if(i == pPipeline->m_EncResponse.NumFrameActual)
	{
		return NULL;
	}

	return &(pPipeline->m_pEncSurfaces[i]);
	
}

//Hardware Alloc buff
int API_IntelMedia_SDK_HW_AllocBuff(void *lp)
{
	if(lp==NULL)
	{
        	SDK_Codec_Debug();
	}
	sAPI_EncoderInputParams *pEncoderPar = (sAPI_EncoderInputParams *)lp;
	assert(pEncoderPar);

	sInputParams Params = {0};	// input parameters from command line
	//std::auto_ptr<CEncodingPipeline>	pPipeline;
	CEncodingPipeline  *pPipeline=NULL;
	pPipeline = new CEncodingPipeline;
	assert(pPipeline);

	mfxStatus sts = MFX_ERR_NONE; // return value check
	Params.numViews = 1;
	Params.nTargetUsage = MFX_TARGETUSAGE_BALANCED;
	
	//Params.memType = D3D9_MEMORY;
	Params.memType = SYSTEM_MEMORY;
	Params.bUseHWLib = 1;
	Params.nPicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	
	if (Params.nRateControlMethod == 0)
	{
		Params.nRateControlMethod = MFX_RATECONTROL_VBR;
	}

	if(pEncoderPar->nBuffNum == 0 || pEncoderPar->nBuffNum < 5)
		Params.nAsyncDepth = 5;
	else
		Params.nAsyncDepth = pEncoderPar->nBuffNum;

	Params.nEncNo=0;

	Params.CodecId = MFX_CODEC_AVC;
	//Params.ColorFormat = MFX_FOURCC_YV12;
	Params.ColorFormat = MFX_FOURCC_NV12;
	//Input YUV Width
	if(pEncoderPar->nSrcWidth>120&&pEncoderPar->nSrcWidth<=3840)
	{
		Params.nWidth	= pEncoderPar->nSrcWidth; //* å®½åº¦
	}
	else
	{
        	SDK_Codec_Debug();
		return -1;
	}

	//Output Encode Frame Width
	if(pEncoderPar->nDstWidth>120&&pEncoderPar->nDstWidth<=3840)
	{
		Params.nDstWidth = pEncoderPar->nDstWidth;
	}
	else
	{
       // SDK_Codec_Debug();
		//return -1;
		Params.nDstWidth= Params.nWidth;
	}
	
	//Input YUV Height
	if(pEncoderPar->nSrcHeight>120&&pEncoderPar->nSrcHeight<=2160)
	{
		Params.nHeight	= pEncoderPar->nSrcHeight; //* é«˜åº¦
	}
	else
	{
        SDK_Codec_Debug();
		return -1;
	}

	//Output Encode Frame Height
	if(pEncoderPar->nDstHeight>120&&pEncoderPar->nDstHeight<=2160)
	{
		Params.nDstHeight = pEncoderPar->nDstHeight;
	}
	else
	{
       // SDK_Codec_Debug();
		Params.nDstHeight = Params.nHeight;
		//return -1;
	}

	Params.nStartX= pEncoderPar->nDstX;
	Params.nStartY= pEncoderPar->nDstY;


	if(pEncoderPar->nEncodeQP >=10&&pEncoderPar->nEncodeQP<=50)
	{
		//In range [1,100]. 100 is the best quality
		Params.nQuality = pEncoderPar->nEncodeQP;
	}
	else
	{
		Params.nQuality = 10;
	}

	//Level
	if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels1)
	{
	   Params.nProfieLevel = ENCODER_Profile_Levels1;
	}
	else if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels2)
	{
	   Params.nProfieLevel = ENCODER_Profile_Levels2;
	}
	else if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels3)
	{
	   Params.nProfieLevel = ENCODER_Profile_Levels3;
	}
	else if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels4)
	{
	   Params.nProfieLevel = ENCODER_Profile_Levels4;
	}
	else
	{
	   Params.nProfieLevel = ENCODER_Profile_Levels4;
	}

	//* ÉèÖÃProfile
	if(pEncoderPar->nEncodeProfile==ENCODER_Baseline_Profile)
	{
	   //baseline
	  Params.nEncodeProfie = ENCODER_Baseline_Profile;
	}
	else if(pEncoderPar->nEncodeProfile==ENCODER_Main_Profile)
	{
	   //main
	  Params.nEncodeProfie = ENCODER_Main_Profile;
	}
	else if(pEncoderPar->nEncodeProfile==ENCODER_High_Profile)
	{
	   //high
	  Params.nEncodeProfie = ENCODER_High_Profile;
	}
	else
	{
	   Params.nEncodeProfie = ENCODER_High_Profile;
	}
	if(pEncoderPar->nEncodeFrameRate>=1.0&&pEncoderPar->nEncodeFrameRate<=30.0)
	{
	  Params.dFrameRate  = (mfxF64)pEncoderPar->nEncodeFrameRate;//
	}
	else
	{
	   Params.dFrameRate  = 25;
	}

	if(pEncoderPar->nEncodeStreambit>16&&pEncoderPar->nEncodeStreambit<100*1024)
	{
	  Params.nBitRate = pEncoderPar->nEncodeStreambit;//
	}
	else
	{
	  // Params.nBitRate = 4096;
	   Params.nBitRate = CalculateDefaultBitrate(Params.CodecId, Params.nTargetUsage, Params.nDstWidth,
		   Params.nDstHeight, Params.dFrameRate);
	}

	//Frame GOP
	if(pEncoderPar->nKeyframeIntelval>=5&&pEncoderPar->nKeyframeIntelval<=300)
	{
	   Params.nGopszie = pEncoderPar->nKeyframeIntelval;
	}
	else
	{
	   Params.nGopszie = 100;
	}
   

	printf("iiiiiiiiiiiiiiiiiiiiii before InitAlloc\n");
	sts = pPipeline->InitAlloc(&Params);
    printf("iiiiiiiiiiiiiiiiiiiiii after InitAlloc %d\n",sts);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);
	
	pPipeline->PrintInfo();


	//pHandle
	pEncoderPar->pHandle = (void *)pPipeline;

	msdk_printf(MSDK_STRING("Processing encode started\n"));

	return 0;
}
#else
void API_IntelMedia_SDK_HWAllocParaInit(void *lp)
{
	sAPI_FrameSurfaceAllocParams *pAllocPar = (sAPI_FrameSurfaceAllocParams *)lp;
	assert(pAllocPar);

	pAllocPar->nWidth		=VIDEO_WIDTH;	
	pAllocPar->nHeight	=VIDEO_HEIGHT;

}

//Hardware GetBuff
mfxFrameSurface1* API_IntelMedia_SDK_HW_GetBuff(void *lp)
{
	if(lp==NULL)
	{
        	SDK_Codec_Debug();
	}

	sAPI_FrameSurfaceAllocParams *pAllocPar = (sAPI_FrameSurfaceAllocParams *)lp;
	assert(pAllocPar);

	mfxFrameSurface1 *pSurfaces = NULL;

	pSurfaces = new mfxFrameSurface1;

	memset(pSurfaces,0,sizeof(mfxFrameSurface1));

	mfxFrameInfo    *Info = &pSurfaces->Info;
	mfxFrameData    *Data = &pSurfaces->Data;	

	Info->FourCC = MFX_FOURCC_NV12;
	Info->CropW = pAllocPar->nWidth;
	Info->CropH = pAllocPar->nHeight;
	Info->Width = MSDK_ALIGN16( pAllocPar->nWidth);
	Info->Height = MSDK_ALIGN16(pAllocPar->nHeight);
	Info->FrameRateExtN = 30;
	Info->FrameRateExtD = 1;
	Info->PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	Info->ChromaFormat = MFX_CHROMAFORMAT_YUV420;

	Data->Locked = 0;
	Data->Pitch = Info->Width;
	Data->Y = (mfxU8  *)malloc(Info->Width * Info->Height * 3/2);
	Data->UV = Data->Y + Info->Width * Info->Height;

	return pSurfaces;
	
}
#endif
//Hardware Encode reset Param
int API_IntelMedia_SDK_HW_EncResetPara(void *lp,void *encInfo)
{
	if(lp==NULL)
	{
        	SDK_Codec_Debug();
	}
	sAPI_EncoderInputParams *pEncoderPar = (sAPI_EncoderInputParams *)lp;
	assert(pEncoderPar);

	assert(pEncoderPar->pHandle);

	video_info_t *penc_info = (video_info_t *)encInfo;

	CEncodingPipeline  *pPipeline = (CEncodingPipeline  *)pEncoderPar->pHandle;

	sInputParams Params = {0};	// input parameters from command line

	mfxStatus sts = MFX_ERR_NONE; // return value check
	Params.numViews = 1;
	Params.nTargetUsage = MFX_TARGETUSAGE_BALANCED;
	
	//Params.memType = D3D9_MEMORY;
	Params.memType = SYSTEM_MEMORY;
	Params.bUseHWLib = 1;
	Params.nPicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	if(pEncoderPar->nBuffNum == 0 || pEncoderPar->nBuffNum < 5)
		Params.nAsyncDepth = 5;
	else
		Params.nAsyncDepth = pEncoderPar->nBuffNum;
#if 0
	if (Params.nAsyncDepth == 0)
	{
		Params.nAsyncDepth = 4; //set by default;
	}
	
	// Ignoring user-defined Async Depth for LA
	if (Params.nMaxSliceSize)
	{
		Params.nAsyncDepth = 1;
	}
#endif	
	if (Params.nRateControlMethod == 0)
	{
		Params.nRateControlMethod = MFX_RATECONTROL_VBR;
	}

	Params.nEncNo=pEncoderPar->nEncNo;

	Params.CodecId = MFX_CODEC_AVC;
	//Params.ColorFormat = MFX_FOURCC_YV12;
	Params.ColorFormat = MFX_FOURCC_NV12;
	//Input YUV Width
	if(pEncoderPar->nSrcWidth>120&&pEncoderPar->nSrcWidth<=3840)
	{
		Params.nWidth	= pEncoderPar->nSrcWidth; //* å®½åº¦
	}
	else
	{
        	SDK_Codec_Debug();
		return -1;
	}
	//Output Encode Frame Width
	if(pEncoderPar->nDstWidth>120&&pEncoderPar->nDstWidth<=3840)
	{
		Params.nDstWidth = pEncoderPar->nDstWidth;
	}
	else
	{
       // SDK_Codec_Debug();
		//return -1;
		Params.nDstWidth= Params.nWidth;
	}

	//Input YUV Height
	if(pEncoderPar->nSrcHeight>120&&pEncoderPar->nSrcHeight<=2160)
	{
		Params.nHeight	= pEncoderPar->nSrcHeight; //* é«˜åº¦
	}
	else
	{
        	SDK_Codec_Debug();
		return -1;
	}

	//Output Encode Frame Height
	if(pEncoderPar->nDstHeight>120&&pEncoderPar->nDstHeight<=2160)
	{
		Params.nDstHeight = pEncoderPar->nDstHeight;
	}
	else
	{
       // SDK_Codec_Debug();
		Params.nDstHeight = Params.nHeight;
		//return -1;
	}

	Params.nStartX= pEncoderPar->nDstX;
	Params.nStartY= pEncoderPar->nDstY;



	if(pEncoderPar->nEncodeQP >=10&&pEncoderPar->nEncodeQP<=50)
	{
		//In range [1,100]. 100 is the best quality
		Params.nQuality = pEncoderPar->nEncodeQP;
	}
	else
	{
		Params.nQuality = 10;
	}

	//Level
	if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels1)
	{
	   Params.nProfieLevel = ENCODER_Profile_Levels1;
	}
	else if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels2)
	{
	   Params.nProfieLevel = ENCODER_Profile_Levels2;
	}
	else if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels3)
	{
	   Params.nProfieLevel = ENCODER_Profile_Levels3;
	}
	else if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels4)
	{
	   Params.nProfieLevel = ENCODER_Profile_Levels4;
	}
	else
	{
	   Params.nProfieLevel = ENCODER_Profile_Levels4;
	}

	//* ÉèÖÃProfile
	if(pEncoderPar->nEncodeProfile==ENCODER_Baseline_Profile)
	{
	   //baseline
	  Params.nEncodeProfie = ENCODER_Baseline_Profile;
	}
	else if(pEncoderPar->nEncodeProfile==ENCODER_Main_Profile)
	{
	   //main
	  Params.nEncodeProfie = ENCODER_Main_Profile;
	}
	else if(pEncoderPar->nEncodeProfile==ENCODER_High_Profile)
	{
	   //high
	  Params.nEncodeProfie = ENCODER_High_Profile;
	}
	else
	{
	   Params.nEncodeProfie = ENCODER_High_Profile;
	}
	if(pEncoderPar->nEncodeFrameRate>=1.0&&pEncoderPar->nEncodeFrameRate<=30.0)
	{
	  Params.dFrameRate  = (mfxF64)pEncoderPar->nEncodeFrameRate;//
	}
	else
	{
	   Params.dFrameRate  = 25;
	}

	if(pEncoderPar->nEncodeStreambit>16&&pEncoderPar->nEncodeStreambit<100*1024)
	{
	  Params.nBitRate = pEncoderPar->nEncodeStreambit;//
	}
	else
	{
	  // Params.nBitRate = 4096;
	   Params.nBitRate = CalculateDefaultBitrate(Params.CodecId, Params.nTargetUsage, Params.nDstWidth,
		   Params.nDstHeight, Params.dFrameRate);
	}

	//Frame GOP
	if(pEncoderPar->nKeyframeIntelval>=5&&pEncoderPar->nKeyframeIntelval<=300)
	{
	   Params.nGopszie = pEncoderPar->nKeyframeIntelval;
	}
	else
	{
	   Params.nGopszie = 100;
	}

	sts = pPipeline->ResetEncPara(&Params,penc_info);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

   return sts;
}

//Hardware Encode Initial
int API_IntelMedia_SDK_HW_EncInitial(void *lp)
{
	if(lp==NULL)
	{
        SDK_Codec_Debug();
	}
	sAPI_EncoderInputParams *pEncoderPar = (sAPI_EncoderInputParams *)lp;
	assert(pEncoderPar);

	sInputParams Params = {0};	// input parameters from command line
	//std::auto_ptr<CEncodingPipeline>	pPipeline;
	CEncodingPipeline  *pPipeline=NULL;
	pPipeline = new CEncodingPipeline;
	assert(pPipeline);

	mfxStatus sts = MFX_ERR_NONE; // return value check
	Params.numViews = 1;
	Params.nTargetUsage = MFX_TARGETUSAGE_BALANCED;
	
	//Params.memType = D3D9_MEMORY;
	Params.memType = SYSTEM_MEMORY;
	Params.bUseHWLib = 1;
	Params.nPicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	if(pEncoderPar->nBuffNum == 0 || pEncoderPar->nBuffNum < 5)
		Params.nAsyncDepth = 5;
	else
		Params.nAsyncDepth = pEncoderPar->nBuffNum;
#if 0
	if (Params.nAsyncDepth == 0)
	{
		Params.nAsyncDepth = 4; //set by default;
	}
	
	// Ignoring user-defined Async Depth for LA
	if (Params.nMaxSliceSize)
	{
		Params.nAsyncDepth = 1;
	}

	if(Params.nAsyncDepth < 5)
	{
		Params.nAsyncDepth= 5;
	}
#endif
	
	if (Params.nRateControlMethod == 0)
	{
		Params.nRateControlMethod = MFX_RATECONTROL_VBR;
	}

	Params.nEncNo=pEncoderPar->nEncNo;

	Params.CodecId = MFX_CODEC_AVC;
	//Params.ColorFormat = MFX_FOURCC_YV12;
	Params.ColorFormat = MFX_FOURCC_NV12;
	//Input YUV Width
	if(pEncoderPar->nSrcWidth>120&&pEncoderPar->nSrcWidth<=3840)
	{
		Params.nWidth	= pEncoderPar->nSrcWidth; //* å®½åº¦
	}
	else
	{
        	SDK_Codec_Debug();
		return -1;
	}
	//Output Encode Frame Width
	if(pEncoderPar->nDstWidth>120&&pEncoderPar->nDstWidth<=3840)
	{
		Params.nDstWidth = pEncoderPar->nDstWidth;
	}
	else
	{
       // SDK_Codec_Debug();
		//return -1;
		Params.nDstWidth= Params.nWidth;
	}

	//Input YUV Height
	if(pEncoderPar->nSrcHeight>120&&pEncoderPar->nSrcHeight<=2160)
	{
		Params.nHeight	= pEncoderPar->nSrcHeight; //* é«˜åº¦
	}
	else
	{
        SDK_Codec_Debug();
		return -1;
	}

	//Output Encode Frame Height
	if(pEncoderPar->nDstHeight>120&&pEncoderPar->nDstHeight<=2160)
	{
		Params.nDstHeight = pEncoderPar->nDstHeight;
	}
	else
	{
       // SDK_Codec_Debug();
		Params.nDstHeight = Params.nHeight;
		//return -1;
	}

	Params.nStartX= pEncoderPar->nDstX;
	Params.nStartY= pEncoderPar->nDstY;

	if(pEncoderPar->nEncodeQP >=10&&pEncoderPar->nEncodeQP<=50)
	{
		//In range [1,100]. 100 is the best quality
		Params.nQuality = pEncoderPar->nEncodeQP;
	}
	else
	{
		Params.nQuality = 10;
	}

   //Level
	if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels1)
   {
	   Params.nProfieLevel = ENCODER_Profile_Levels1;
   }
   else if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels2)
   {
	   Params.nProfieLevel = ENCODER_Profile_Levels2;
   }
   else if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels3)
   {
	   Params.nProfieLevel = ENCODER_Profile_Levels3;
   }
	else if(pEncoderPar->nEncodeLevels==ENCODER_Profile_Levels4)
   {
	   Params.nProfieLevel = ENCODER_Profile_Levels4;
   }
   else
   {
	   Params.nProfieLevel = ENCODER_Profile_Levels4;
   }

//* ÉèÖÃProfile
   if(pEncoderPar->nEncodeProfile==ENCODER_Baseline_Profile)
   {
	   //baseline
	  Params.nEncodeProfie = ENCODER_Baseline_Profile;
   }
   else if(pEncoderPar->nEncodeProfile==ENCODER_Main_Profile)
   {
	   //main
	  Params.nEncodeProfie = ENCODER_Main_Profile;
   }
   else if(pEncoderPar->nEncodeProfile==ENCODER_High_Profile)
   {
	   //high
	  Params.nEncodeProfie = ENCODER_High_Profile;
   }
   else
   {
	   Params.nEncodeProfie = ENCODER_High_Profile;
   }
	if(pEncoderPar->nEncodeFrameRate>=1.0&&pEncoderPar->nEncodeFrameRate<=30.0)
   {
	  Params.dFrameRate  = (mfxF64)pEncoderPar->nEncodeFrameRate;//
   }
   else
   {
	   Params.dFrameRate  = 25;
   }
	
   if(pEncoderPar->nEncodeStreambit>16&&pEncoderPar->nEncodeStreambit<100*1024)
   {
	  Params.nBitRate = pEncoderPar->nEncodeStreambit;//
   }
   else
   {
	  // Params.nBitRate = 4096;
	   Params.nBitRate = CalculateDefaultBitrate(Params.CodecId, Params.nTargetUsage, Params.nDstWidth,
		   Params.nDstHeight, Params.dFrameRate);
   }

   //Frame GOP
   if(pEncoderPar->nKeyframeIntelval>=5&&pEncoderPar->nKeyframeIntelval<=300)
   {
	   Params.nGopszie = pEncoderPar->nKeyframeIntelval;
   }
   else
   {
	   Params.nGopszie = 100;
   }
   


	//pPipeline.reset((Params.nRotationAngle) ? new CUserPipeline : new CEncodingPipeline);
	//MSDK_CHECK_POINTER(pPipeline.get(), MFX_ERR_MEMORY_ALLOC);

	if (MVC_ENABLED & Params.MVC_flags)
	{
		pPipeline->SetMultiView();
		pPipeline->SetNumView(Params.numViews);
	}
	printf("iiiiiiiiiiiiiiiiiiiiii before init\n");
	sts = pPipeline->Init(&Params);
	printf("iiiiiiiiiiiiiiiiiiiiii after init\n");
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);

	pPipeline->PrintInfo();


	//pHandle
	pEncoderPar->pHandle = (void *)pPipeline;

	msdk_printf(MSDK_STRING("Processing encode started\n"));
	return 0;
}


//Hardware Encode Release
int API_IntelMedia_SDK_HW_EncRelease(void *lp)
{
    if(lp==NULL)
    {
        SDK_Codec_Debug();
        return -1;
    }
	
    sAPI_EncoderInputParams *pEncoderPar = (sAPI_EncoderInputParams *)lp;
    assert(pEncoderPar);

    CEncodingPipeline  *pPipeline = (CEncodingPipeline *)pEncoderPar->pHandle;
    assert(pPipeline);

    pPipeline->Close();
    if(pPipeline)
    {
        delete pPipeline;
        pPipeline = NULL;
        pEncoderPar->pHandle = NULL;
    }

    return 0;
}


//Hardware Encode Frame
int API_IntelMedia_SDK_HW_EncFrame(void *lp,void *encInfo)
{
    int iResult = 0;
    if(lp==NULL || NULL == encInfo)
    {
        SDK_Codec_Debug();
        return -1;
    }
    sAPI_EncoderInputParams *pEncoderPar = (sAPI_EncoderInputParams *)lp;
    if(!pEncoderPar)
	{
		SDK_Codec_Debug();
		return -1;
	}
    video_info_t *penc_info = (video_info_t *)encInfo;
    CEncodingPipeline  *pPipeline=NULL;
    pPipeline = (CEncodingPipeline *)pEncoderPar->pHandle;
    if(!pPipeline)
	{
		SDK_Codec_Debug();
		return -1;
	}
	//printf("EncodeRun\n");
    iResult = pPipeline->EncodeRun(penc_info);
    return iResult;
}
#if 0
void API_IntelMedia_SDK_HW_DEC_GetBuff(void *lp,FRAME_BufList *framBuffList)
{
	int i = 0;
	if(lp==NULL || NULL == framBuffList)
	{
        	SDK_Codec_Debug();
	}

	sAPI_DecoderInputParams *pDecoderPar = (sAPI_DecoderInputParams *)lp;
	assert(pDecoderPar);
	assert(pDecoderPar->pHandle);

	CDecodingPipeline  *pPipeline = (CDecodingPipeline  *)pDecoderPar->pHandle;

	framBuffList->numBufs = 0;
	
	for ( i = 0; i < pPipeline->m_SurfacesNumber; i++)
	{

	}

	if(i == pPipeline->m_EncResponse.NumFrameActual)
	{
		return NULL;
	}

	return;
	
}
#endif
//Hardware Decode Initial
int API_IntelMedia_SDK_HW_DecInitial_CreateSession(void *lp)
{
	if(lp==NULL)
	{
        SDK_Codec_Debug();
	}
	sAPI_DecoderInputParams *pDecoderPar = (sAPI_DecoderInputParams *)lp;
	assert(pDecoderPar);

	sInputParams_dec Params;	// input parameters from command line
	CDecodingPipeline  *pPipeline=NULL;
	pPipeline = new CDecodingPipeline;
	assert(pPipeline);

	MSDK_ZERO_MEMORY(Params);

	mfxStatus sts = MFX_ERR_NONE; // return value check

	Params.mode = MODE_FILE_DUMP/*MODE_RENDERING*/;
	//Params.fourcc = MFX_FOURCC_NV12;

	Params.bUseHWLib = true;
	Params.nDecNo=pDecoderPar->nDecNo;
	Params.memType = /*SYSTEM_MEMORY*/D3D11_MEMORY;
	Params.videoType=MFX_CODEC_AVC;
	if (Params.videoType == CODEC_MVC)
	{
		Params.videoType = MFX_CODEC_AVC;
		Params.bIsMVC = true;
	}

	if(pDecoderPar->nBuffNum == 0 || pDecoderPar->nBuffNum < 5)
		Params.nAsyncDepth = 5;
	else
		Params.nAsyncDepth = pDecoderPar->nBuffNum;	

	sts = pPipeline->Init_CreateSession(&Params);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);

	//pHandle
	pDecoderPar->pHandle = (void *)pPipeline;

	msdk_printf(MSDK_STRING("API_IntelMedia_SDK_HW_DecInitial_CreateSession finish\n"));
	return 0;
}

//Hardware Decode Initial
int API_IntelMedia_SDK_HW_DecInitial_DecHeader(void *lp,Bitstream_Buf* pBitStrameBuff)
{
	if(lp==NULL || NULL == pBitStrameBuff)
	{
        	SDK_Codec_Debug();
	}

	sAPI_DecoderInputParams *pDecoderPar = (sAPI_DecoderInputParams *)lp;
	assert(pDecoderPar);

	sInputParams_dec Params;	// input parameters from command line
       CDecodingPipeline  *pPipeline=NULL;
       pPipeline = (CDecodingPipeline *)pDecoderPar->pHandle;
	assert(pPipeline);

	MSDK_ZERO_MEMORY(Params);

	mfxStatus sts = MFX_ERR_NONE; // return value check

	Params.mode = MODE_FILE_DUMP;
	//Params.fourcc = MFX_FOURCC_NV12;

	Params.bUseHWLib = true;
	Params.nDecNo=pDecoderPar->nDecNo;
	Params.memType = SYSTEM_MEMORY;
	Params.videoType=MFX_CODEC_AVC;
	if (Params.videoType == CODEC_MVC)
	{
		Params.videoType = MFX_CODEC_AVC;
		Params.bIsMVC = true;
	}

	if(pDecoderPar->nBuffNum == 0 || pDecoderPar->nBuffNum < 5)
		Params.nAsyncDepth = 5;
	else
		Params.nAsyncDepth = pDecoderPar->nBuffNum;

	Params.pBitStrameBuff = pBitStrameBuff;

	sts = pPipeline->Init_DecHeader(&Params);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);

	msdk_printf(MSDK_STRING("API_IntelMedia_SDK_HW_DecInitial_DecHeader finish\n"));
	return sts;
}

//Hardware Decode Initial
int API_IntelMedia_SDK_HW_DecInitial_CreateDec(void *lp)
{
	if(lp==NULL)
	{
        	SDK_Codec_Debug();
	}
	sAPI_DecoderInputParams *pDecoderPar = (sAPI_DecoderInputParams *)lp;
	assert(pDecoderPar);

	sInputParams_dec Params;	// input parameters from command line
       CDecodingPipeline  *pPipeline=NULL;
       pPipeline = (CDecodingPipeline *)pDecoderPar->pHandle;
	assert(pPipeline);

	MSDK_ZERO_MEMORY(Params);

	mfxStatus sts = MFX_ERR_NONE; // return value check

	Params.mode = MODE_FILE_DUMP/*MODE_RENDERING*/;
	//Params.fourcc = MFX_FOURCC_NV12;

	Params.bUseHWLib = true;
	Params.nDecNo=pDecoderPar->nDecNo;
	Params.memType = SYSTEM_MEMORY;
	Params.videoType=MFX_CODEC_AVC;
	if (Params.videoType == CODEC_MVC)
	{
		Params.videoType = MFX_CODEC_AVC;
		Params.bIsMVC = true;
	}

	if(pDecoderPar->nBuffNum == 0 || pDecoderPar->nBuffNum < 5)
		Params.nAsyncDepth = 5;
	else
		Params.nAsyncDepth = pDecoderPar->nBuffNum;	

	sts = pPipeline->Init_CreateDec(&Params);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);

	pPipeline->PrintInfo();

	msdk_printf(MSDK_STRING("API_IntelMedia_SDK_HW_DecInitial_CreateDec finish\n"));
	return 0;
}

//Hardware Decode Initial
int API_IntelMedia_SDK_HW_DecInitial(void *lp)
{
	if(lp==NULL)
	{
        SDK_Codec_Debug();
	}
	sAPI_DecoderInputParams *pDecoderPar = (sAPI_DecoderInputParams *)lp;
	assert(pDecoderPar);

	sInputParams_dec Params;	// input parameters from command line
	CDecodingPipeline  *pPipeline=NULL;
	pPipeline = new CDecodingPipeline;
	assert(pPipeline);

	MSDK_ZERO_MEMORY(Params);

	mfxStatus sts = MFX_ERR_NONE; // return value check

/****************************************/
	char srcpath[256];
	char dstpath[256];
	//sprintf(srcpath , "%p%s" , pPipeline ,"temp.264");
	//sprintf(dstpath , "%p%s" , pPipeline ,"temp.yuv");
	sprintf(srcpath , "%s%s" , "/home/wuzc/x86/intelsdk/edukit_app/reach_app/" ,"test.264");
	sprintf(dstpath , "%s%s" , "/home/wuzc/x86/intelsdk/edukit_app/reach_app/" ,"temp.yuv");
	for (int i=0;i<strlen(srcpath);i++)
	{
		Params.strSrcFile[i] = srcpath[i];
	}
	for (int i=0;i<strlen(dstpath);i++)
	{
		Params.strDstFile[i] = dstpath[i];
	}
	printf("iiiiiiiii src=%s,dst=%s\n",Params.strSrcFile,Params.strDstFile);
/********************************************/

	Params.mode = MODE_FILE_DUMP;
	//Params.fourcc = MFX_FOURCC_NV12;

	Params.bUseHWLib = true;
	Params.nDecNo=pDecoderPar->nDecNo;
	Params.memType = SYSTEM_MEMORY;
	Params.videoType=MFX_CODEC_AVC;
	if (Params.videoType == CODEC_MVC)
	{
		Params.videoType = MFX_CODEC_AVC;
		Params.bIsMVC = true;
	}

	if(pDecoderPar->nBuffNum == 0 || pDecoderPar->nBuffNum < 5)
		Params.nAsyncDepth = 5;
	else
		Params.nAsyncDepth = pDecoderPar->nBuffNum;	
	printf("=============Init\n");
	sts = pPipeline->Init(&Params);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);

	pPipeline->PrintInfo();


	//pHandle
	pDecoderPar->pHandle = (void *)pPipeline;

	msdk_printf(MSDK_STRING("Processing decode started\n"));
	return 0;
}

//Hardware Encode Initial
int API_IntelMedia_SDK_HW_DecInitial_reach(void *lp)
{
	if(lp==NULL)
	{
        SDK_Codec_Debug();
	}
	sAPI_EncoderInputParams *pDecoderPar = (sAPI_EncoderInputParams *)lp;
	assert(pDecoderPar);

	sInputParams_reach Params = {0};	// input parameters from command line
	//std::auto_ptr<CEncodingPipeline>	pPipeline;
	CDecodingPipeline  *pPipeline=NULL;
	pPipeline = new CDecodingPipeline;
	assert(pPipeline);

	mfxStatus sts = MFX_ERR_NONE; // return value check
	Params.numViews = 1;
	Params.nTargetUsage = MFX_TARGETUSAGE_BALANCED;
	
	//Params.memType = D3D9_MEMORY;
	Params.memType = SYSTEM_MEMORY;
	Params.bUseHWLib = 1;
	Params.nPicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	if(pDecoderPar->nBuffNum == 0 || pDecoderPar->nBuffNum < 5)
		Params.nAsyncDepth = 5;
	else
		Params.nAsyncDepth = pDecoderPar->nBuffNum;
	
	if (Params.nRateControlMethod == 0)
	{
		Params.nRateControlMethod = MFX_RATECONTROL_VBR;
	}

	Params.nDecNo=pDecoderPar->nEncNo;

	Params.CodecId = MFX_CODEC_AVC;
	//Params.ColorFormat = MFX_FOURCC_YV12;
	Params.ColorFormat = MFX_FOURCC_NV12;
	//Input YUV Width
	if(pDecoderPar->nSrcWidth>120&&pDecoderPar->nSrcWidth<=3840)
	{
		Params.nWidth	= pDecoderPar->nSrcWidth; //* å®½åº¦
	}
	else
	{
        	SDK_Codec_Debug();
		return -1;
	}
	//Output Encode Frame Width
	if(pDecoderPar->nDstWidth>120&&pDecoderPar->nDstWidth<=3840)
	{
		Params.nDstWidth = pDecoderPar->nDstWidth;
	}
	else
	{
       // SDK_Codec_Debug();
		//return -1;
		Params.nDstWidth= Params.nWidth;
	}

	//Input YUV Height
	if(pDecoderPar->nSrcHeight>120&&pDecoderPar->nSrcHeight<=2160)
	{
		Params.nHeight	= pDecoderPar->nSrcHeight; //* é«˜åº¦
	}
	else
	{
        SDK_Codec_Debug();
		return -1;
	}

	//Output Encode Frame Height
	if(pDecoderPar->nDstHeight>120&&pDecoderPar->nDstHeight<=2160)
	{
		Params.nDstHeight = pDecoderPar->nDstHeight;
	}
	else
	{
       // SDK_Codec_Debug();
		Params.nDstHeight = Params.nHeight;
		//return -1;
	}

	if(pDecoderPar->nEncodeQP >=10&&pDecoderPar->nEncodeQP<=50)
	{
		//In range [1,100]. 100 is the best quality
		Params.nQuality = pDecoderPar->nEncodeQP;
	}
	else
	{
		Params.nQuality = 10;
	}

   //Level
	if(pDecoderPar->nEncodeLevels==ENCODER_Profile_Levels1)
   {
	   Params.nProfieLevel = ENCODER_Profile_Levels1;
   }
   else if(pDecoderPar->nEncodeLevels==ENCODER_Profile_Levels2)
   {
	   Params.nProfieLevel = ENCODER_Profile_Levels2;
   }
   else if(pDecoderPar->nEncodeLevels==ENCODER_Profile_Levels3)
   {
	   Params.nProfieLevel = ENCODER_Profile_Levels3;
   }
	else if(pDecoderPar->nEncodeLevels==ENCODER_Profile_Levels4)
   {
	   Params.nProfieLevel = ENCODER_Profile_Levels4;
   }
   else
   {
	   Params.nProfieLevel = ENCODER_Profile_Levels4;
   }

//* ÉèÖÃProfile
   if(pDecoderPar->nEncodeProfile==ENCODER_Baseline_Profile)
   {
	   //baseline
	  Params.nEncodeProfie = ENCODER_Baseline_Profile;
   }
   else if(pDecoderPar->nEncodeProfile==ENCODER_Main_Profile)
   {
	   //main
	  Params.nEncodeProfie = ENCODER_Main_Profile;
   }
   else if(pDecoderPar->nEncodeProfile==ENCODER_High_Profile)
   {
	   //high
	  Params.nEncodeProfie = ENCODER_High_Profile;
   }
   else
   {
	   Params.nEncodeProfie = ENCODER_High_Profile;
   }
	if(pDecoderPar->nEncodeFrameRate>=1.0&&pDecoderPar->nEncodeFrameRate<=30.0)
   {
	  Params.dFrameRate  = (mfxF64)pDecoderPar->nEncodeFrameRate;//
   }
   else
   {
	   Params.dFrameRate  = 25;
   }
	
   if(pDecoderPar->nEncodeStreambit>16&&pDecoderPar->nEncodeStreambit<100*1024)
   {
	  Params.nBitRate = pDecoderPar->nEncodeStreambit;//
   }
   else
   {
	  // Params.nBitRate = 4096;
	   Params.nBitRate = CalculateDefaultBitrate(Params.CodecId, Params.nTargetUsage, Params.nDstWidth,
		   Params.nDstHeight, Params.dFrameRate);
   }

   //Frame GOP
   if(pDecoderPar->nKeyframeIntelval>=5&&pDecoderPar->nKeyframeIntelval<=300)
   {
	   Params.nGopszie = pDecoderPar->nKeyframeIntelval;
   }
   else
   {
	   Params.nGopszie = 100;
   }
   

	printf("iiiiiiiiiiiiiiiiiiiiii before init\n");
	sts = pPipeline->Init_reach(&Params);
	printf("iiiiiiiiiiiiiiiiiiiiii after init\n");
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);

	pPipeline->PrintInfo();


	//pHandle
	pDecoderPar->pHandle = (void *)pPipeline;

	msdk_printf(MSDK_STRING("Processing decode started\n"));
	return 0;
}


//Hardware Encode Frame
int API_IntelMedia_SDK_HW_DecFrame(void *lp,void *videoInfo)
{
    int iResult = 0;
    if(lp==NULL || NULL == videoInfo)
    {
        SDK_Codec_Debug();
        return -1;
    }
    sAPI_DecoderInputParams *pDecoderPar = (sAPI_DecoderInputParams *)lp;
    if(!pDecoderPar)
	{
		SDK_Codec_Debug();
		return -1;
	}

    video_info_t *pvideo_info = (video_info_t *)videoInfo;
    CDecodingPipeline  *pPipeline=NULL;
    pPipeline = (CDecodingPipeline *)pDecoderPar->pHandle;
    if(!pPipeline)
	{
		SDK_Codec_Debug();
		return -1;
	}
	
    iResult = pPipeline->RunDecodingBits(pvideo_info);
    //iResult = pPipeline->RunDecoding();
    return iResult;
}



//Hardware Encode Release
int API_IntelMedia_SDK_HW_DecRelease(void *lp)
{
    if(lp==NULL)
    {
        SDK_Codec_Debug();
        return -1;
    }
	
    sAPI_DecoderInputParams *pDecoderPar = (sAPI_DecoderInputParams *)lp;
    assert(pDecoderPar);

    CDecodingPipeline  *pPipeline = (CDecodingPipeline *)pDecoderPar->pHandle;
    assert(pPipeline);

    pPipeline->Close();
    if(pPipeline)
    {
        delete pPipeline;
        pPipeline = NULL;
        pDecoderPar->pHandle = NULL;
    }

    return 0;
}


