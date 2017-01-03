/* ////////////////////////////////////////////////////////////////////////////// */
/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//        Copyright (c) 2008 - 2014 Intel Corporation. All Rights Reserved.
//
//
*/

#ifndef __SAMPLE_VPP_UTILS_H
#define __SAMPLE_VPP_UTILS_H

/* ************************************************************************* */

#include "sample_defs.h"
#include "hw_device.h"
#if D3D_SURFACES_SUPPORT
#pragma warning(disable : 4201)
#include <d3d9.h>
#include <dxva2api.h>
#include <windows.h>
#endif

#ifdef LIBVA_SUPPORT
#include "vaapi_utils.h"
#endif

#include <stdio.h>

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cctype>

#include "mfxvideo.h"
#include "mfxvideo++.h"
#include "sample_utils.h"
#include "sample_params.h"
#include "base_allocator.h"

#include "sample_vpp_config.h"
/* ************************************************************************* */

#define VPP_IN       (0)
#define VPP_OUT      (1)
#define VPP_WORK     (2)
#define VPP_IN_RGB   2

#define MFX_MAX_32U   ( 0xFFFFFFFF )

// we introduce new macros without error message (returned status only)
// it allows to remove final error message due to EOF
#define IOSTREAM_CHECK_NOT_EQUAL(P, X, ERR)          {if ((X) != (P)) {return ERR;}}

typedef struct _ownFrameInfo
{
  mfxU16  nWidth;
  mfxU16  nHeight;
  // ROI
  mfxU16  CropX;
  mfxU16  CropY;
  mfxU16  CropW;
  mfxU16  CropH;

  mfxU32 FourCC;
  mfxU8  PicStruct;
  mfxF64 dFrameRate;

} sOwnFrameInfo;

struct sVppInputParams
{
  /* smart filters defined by mismatch btw src & dst */
  sOwnFrameInfo inFrameInfo;
  sOwnFrameInfo outFrameInfo;

  // flag describes type of memory
  // true  - frames in video memory (d3d surfaces),
  // false - in system memory
  MemType memType;

  mfxU32   requestedFramesCount;

  // required implementation of MediaSDK library
  mfxIMPL  impLib;
};

struct sFrameProcessor
{
  MFXVideoSession     mfxSession;
  MFXVideoVPP*        pmfxVPP;
  mfxPluginUID        mfxGuid;
  bool                plugin;
  sFrameProcessor(void){ pmfxVPP = NULL; plugin = false; return; };
};

struct sMemoryAllocator
{
  MFXFrameAllocator*  pMfxAllocator;
  mfxAllocatorParams* pAllocatorParams;
  MemType             memType;
  bool                bUsedAsExternalAllocator;

  mfxFrameSurface1*     pSurfaces[3];
  mfxFrameAllocResponse response[3];

  CHWDevice* pDevice;
};

struct sAppResources
{

	sFrameProcessor    Processor;
	mfxVideoParam      VppParams;
	sMemoryAllocator   Allocator;

	/* VPP extension */
	mfxExtVPPDoUse      extDoUse;
	mfxU32              tabDoUseAlg[ENH_FILTERS_COUNT];
	mfxExtBuffer*       pExtBuf[1+ENH_FILTERS_COUNT];
	mfxExtVppAuxData    extVPPAuxData;

	/* config video enhancement algorithms */
	mfxExtVPPProcAmp       procampConfig;
	mfxExtVPPDetail        detailConfig;
	mfxExtVPPDenoise       denoiseConfig;
	mfxExtVPPImageStab     istabConfig;
	mfxExtVPPComposite     compositeConfig;
	mfxExtVPPDeinterlacing deinterlaceConfig;
	mfxExtVPPFrameRateConversion    frcConfig;
};

/* ******************************************************************* */
/*                        service functions                            */
/* ******************************************************************* */

mfxStatus vppParseInputString(msdk_char* strInput[], mfxU8 nArgNum, sVppInputParams* pParams);

void PrintInfo(sVppInputParams* pParams, mfxVideoParam* pMfxParams, MFXVideoSession *pMfxSession);

mfxStatus InitParamsVPP(mfxVideoParam* pMFXParams, sVppInputParams* pInParams);

mfxStatus InitResources(sAppResources* pResources, mfxVideoParam* pParams, sVppInputParams* pInParams);

mfxStatus ResetResources(sAppResources* pResources, mfxVideoParam* pParams, sVppInputParams* pInParams);

void WipeResources(sAppResources* pResources);

mfxStatus GetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize, mfxFrameSurface1** ppSurface);

mfxStatus ConfigVideoEnhancementFilters( void *arg, sAppResources* pResources );

#endif /* __SAMPLE_VPP_UTILS_H */
