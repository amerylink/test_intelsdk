/*********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2008-2014 Intel Corporation. All Rights Reserved.

**********************************************************************************/

#include "mfx_samples_config.h"

#include <math.h>

#include "sample_vpp_utils.h"
#include "sysmem_allocator.h"
#include "mfxplugin.h"
#include "sample_vpp.h"

#ifdef D3D_SURFACES_SUPPORT
#include "d3d_device.h"
#include "d3d_allocator.h"
#endif
#ifdef MFX_D3D11_SUPPORT
#include "d3d11_device.h"
#include "d3d11_allocator.h"
#endif
#ifdef LIBVA_SUPPORT
#include "vaapi_device.h"
#include "vaapi_allocator.h"
#endif

/* ******************************************************************* */

static
void WipeFrameProcessor(sFrameProcessor* pProcessor);

static
void WipeMemoryAllocator(sMemoryAllocator* pAllocator);

/* ******************************************************************* */

static const msdk_char*
FourCC2Str( mfxU32 FourCC )
{
  switch ( FourCC )
  {
  case MFX_FOURCC_NV12:
    return MSDK_STRING("NV12");
    break;

  case MFX_FOURCC_YV12:
    return MSDK_STRING("YV12");
    break;

  case MFX_FOURCC_YUY2:
    return MSDK_STRING("YUY2");
    break;

  case MFX_FOURCC_RGB3:
    return MSDK_STRING("RGB3");
    break;

  case MFX_FOURCC_RGB4:
    return MSDK_STRING("RGB4");
    break;

  default:
    return MSDK_STRING("UNKN");
    break;
  }

} // msdk_char* FourCC2Str( mfxU32 FourCC )

/* ******************************************************************* */

static const msdk_char*
PicStruct2Str( mfxU16  PicStruct )
{
  if (PicStruct == MFX_PICSTRUCT_PROGRESSIVE)
  {
    return MSDK_STRING("progressive");
  }
  else
  {
    return MSDK_STRING("interleave");
  }

} // msdk_char* PicStruct2Str( mfxU16  PicStruct )

/* ******************************************************************* */

void PrintInfo(sVppInputParams* pParams, mfxVideoParam* pMfxParams, MFXVideoSession *pMfxSession)
{
  mfxFrameInfo Info;

  MSDK_CHECK_POINTER_NO_RET(pParams);
  MSDK_CHECK_POINTER_NO_RET(pMfxParams);

  msdk_printf(MSDK_STRING("VPP Sample Version %s\n\n"), MSDK_SAMPLE_VERSION);

  Info = pMfxParams->vpp.In;
  msdk_printf(MSDK_STRING("Input format\t%s\n"), FourCC2Str( Info.FourCC ));
  msdk_printf(MSDK_STRING("Resolution\t%dx%d\n"), Info.Width, Info.Height);
  msdk_printf(MSDK_STRING("Crop X,Y,W,H\t%d,%d,%d,%d\n"), Info.CropX, Info.CropY, Info.CropW, Info.CropH);
  msdk_printf(MSDK_STRING("Frame rate\t%.2f\n"), (mfxF64)Info.FrameRateExtN / Info.FrameRateExtD);
  msdk_printf(MSDK_STRING("PicStruct\t%s\n"), PicStruct2Str(Info.PicStruct));

  Info = pMfxParams->vpp.Out;
  msdk_printf(MSDK_STRING("Output format\t%s\n"), FourCC2Str( Info.FourCC ));
  msdk_printf(MSDK_STRING("Resolution\t%dx%d\n"), Info.Width, Info.Height);
  msdk_printf(MSDK_STRING("Crop X,Y,W,H\t%d,%d,%d,%d\n"), Info.CropX, Info.CropY, Info.CropW, Info.CropH);
  msdk_printf(MSDK_STRING("Frame rate\t%.2f\n"), (mfxF64)Info.FrameRateExtN / Info.FrameRateExtD);
  msdk_printf(MSDK_STRING("PicStruct\t%s\n"), PicStruct2Str(Info.PicStruct));

  msdk_printf(MSDK_STRING("\n"));
 #if 0
  msdk_printf(MSDK_STRING("Video Enhancement Algorithms\n"));
  msdk_printf(MSDK_STRING("Denoise\t\t%s\n"),     (VPP_FILTER_DISABLED != pParams->denoiseParam.mode) ? MSDK_STRING("ON"): MSDK_STRING("OFF"));
  msdk_printf(MSDK_STRING("VideoAnalysis\t%s\n"), (VPP_FILTER_DISABLED != pParams->vaParam.mode)      ? MSDK_STRING("ON"): MSDK_STRING("OFF"));
  msdk_printf(MSDK_STRING("ProcAmp\t\t%s\n"),     (VPP_FILTER_DISABLED != pParams->procampParam.mode) ? MSDK_STRING("ON"): MSDK_STRING("OFF"));
  msdk_printf(MSDK_STRING("Detail\t\t%s\n"),      (VPP_FILTER_DISABLED != pParams->detailParam.mode)  ? MSDK_STRING("ON"): MSDK_STRING("OFF"));
  msdk_printf(MSDK_STRING("ImgStab\t\t%s\n"),     (VPP_FILTER_DISABLED != pParams->istabParam.mode)   ? MSDK_STRING("ON"): MSDK_STRING("OFF"));
  msdk_printf(MSDK_STRING("\n"));
#endif
  const msdk_char* sMemType = NULL;
  switch (pParams->memType)
  {
  case D3D9_MEMORY:
      sMemType = MSDK_STRING("d3d9");
      break;
  case D3D11_MEMORY:
      sMemType = MSDK_STRING("d3d11");
      break;
#ifdef LIBVA_SUPPORT
  case VAAPI_MEMORY:
      sMemType = MSDK_STRING("vaapi");
      break;
#endif
  default:
      sMemType = MSDK_STRING("system");
  }
  msdk_printf(MSDK_STRING("Memory type\t%s\n"), sMemType);
  msdk_printf(MSDK_STRING("\n"));

  mfxIMPL impl;
  pMfxSession->QueryIMPL(&impl);

  const msdk_char* sImpl = (MFX_IMPL_HARDWARE == MFX_IMPL_BASETYPE(impl)) ? MSDK_STRING("hw") : MSDK_STRING("sw");
  msdk_printf(MSDK_STRING("MediaSDK impl\t%s\n"), sImpl);

  mfxVersion ver;
  pMfxSession->QueryVersion(&ver);
  msdk_printf(MSDK_STRING("MediaSDK ver\t%d.%d\n"), ver.Major, ver.Minor);

  msdk_printf(MSDK_STRING("\n"));

  return;

} // void PrintInfo(...)

/* ******************************************************************* */

mfxStatus InitParamsVPP(mfxVideoParam* pParams, sVppInputParams* pInParams)
{
  mfxU16 maxWidth = 0, maxHeight = 0, i;
  MSDK_CHECK_POINTER(pParams,    MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pInParams,  MFX_ERR_NULL_PTR);

  if (pInParams->inFrameInfo.nWidth == 0 || pInParams->inFrameInfo.nHeight == 0 ){
    return MFX_ERR_UNSUPPORTED;
  }
  if (pInParams->outFrameInfo.nWidth == 0 || pInParams->outFrameInfo.nHeight == 0 ){
    return MFX_ERR_UNSUPPORTED;
  }

  memset(pParams, 0, sizeof(mfxVideoParam));

  /* input data */
  pParams->vpp.In.FourCC          = pInParams->inFrameInfo.FourCC;

  pParams->vpp.In.CropX = pInParams->inFrameInfo.CropX;
  pParams->vpp.In.CropY = pInParams->inFrameInfo.CropY;
  pParams->vpp.In.CropW = pInParams->inFrameInfo.CropW;
  pParams->vpp.In.CropH = pInParams->inFrameInfo.CropH;

  // width must be a multiple of 16
  // height must be a multiple of 16 in case of frame picture and
  // a multiple of 32 in case of field picture
    pInParams->inFrameInfo.nWidth = MSDK_ALIGN16(pInParams->inFrameInfo.nWidth);
    pInParams->inFrameInfo.nHeight = (MFX_PICSTRUCT_PROGRESSIVE == pInParams->inFrameInfo.PicStruct)?
                                         MSDK_ALIGN16(pInParams->inFrameInfo.nHeight) : MSDK_ALIGN32(pInParams->inFrameInfo.nHeight);
    if (pInParams->inFrameInfo.nWidth > maxWidth)
      maxWidth = pInParams->inFrameInfo.nWidth;
    if (pInParams->inFrameInfo.nHeight > maxHeight)
      maxHeight = pInParams->inFrameInfo.nHeight;

  pParams->vpp.In.Width = maxWidth;
  pParams->vpp.In.Height= maxHeight;

  pParams->vpp.In.PicStruct = pInParams->inFrameInfo.PicStruct;
  pParams->vpp.In.ChromaFormat = MFX_CHROMAFORMAT_YUV420;

  ConvertFrameRate(pInParams->inFrameInfo.dFrameRate,
                   &pParams->vpp.In.FrameRateExtN,
                   &pParams->vpp.In.FrameRateExtD);

  /* output data */
  pParams->vpp.Out.FourCC          = pInParams->outFrameInfo.FourCC;

  pParams->vpp.Out.CropX = pInParams->outFrameInfo.CropX;
  pParams->vpp.Out.CropY = pInParams->outFrameInfo.CropY;
  pParams->vpp.Out.CropW = pInParams->outFrameInfo.CropW;
  pParams->vpp.Out.CropH = pInParams->outFrameInfo.CropH;

  // width must be a multiple of 16
  // height must be a multiple of 16 in case of frame picture and
  // a multiple of 32 in case of field picture
  pParams->vpp.Out.Width = MSDK_ALIGN16(pInParams->outFrameInfo.nWidth);
  pParams->vpp.Out.Height= (MFX_PICSTRUCT_PROGRESSIVE == pInParams->outFrameInfo.PicStruct)?
                           MSDK_ALIGN16(pInParams->outFrameInfo.nHeight) : MSDK_ALIGN32(pInParams->outFrameInfo.nHeight);

  pParams->vpp.Out.PicStruct = pInParams->outFrameInfo.PicStruct;
  pParams->vpp.Out.ChromaFormat = MFX_CHROMAFORMAT_YUV420;

  ConvertFrameRate(pInParams->outFrameInfo.dFrameRate,
                   &pParams->vpp.Out.FrameRateExtN,
                   &pParams->vpp.Out.FrameRateExtD);

  // this pattern is checked by VPP
  if( pInParams->memType != SYSTEM_MEMORY )
  {
    pParams->IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
  }
  else
  {
    pParams->IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
  }

  return MFX_ERR_NONE;

} // mfxStatus InitParamsVPP(mfxVideoParam* pParams, sVppInputParams* pInParams)

/* ******************************************************************* */

mfxStatus CreateFrameProcessor(sFrameProcessor* pProcessor, mfxVideoParam* pParams, sVppInputParams* pInParams)
{
  mfxStatus  sts = MFX_ERR_NONE;

  mfxVersion version = {{3, 1}}; // as this version of sample demonstrates the new DOUSE structure used to turn on VPP filters

  MSDK_CHECK_POINTER(pProcessor, MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pParams,    MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pInParams,  MFX_ERR_NULL_PTR);
  mfxIMPL    impl = pInParams->impLib;

  WipeFrameProcessor(pProcessor);

  //MFX session
  if (MFX_IMPL_HARDWARE == impl)
  {
    // try searching on all display adapters
    sts = pProcessor->mfxSession.Init(MFX_IMPL_HARDWARE_ANY, &version);
  }
  else
    sts = pProcessor->mfxSession.Init(MFX_IMPL_SOFTWARE, &version);

  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeFrameProcessor(pProcessor));

  // VPP
  pProcessor->pmfxVPP = new MFXVideoVPP(pProcessor->mfxSession);

  return MFX_ERR_NONE;

} // mfxStatus CreateFrameProcessor(sFrameProcessor* pProcessor, mfxVideoParam* pParams, sVppInputParams* pInParams)

/* ******************************************************************* */

#ifdef D3D_SURFACES_SUPPORT
mfxStatus CreateDeviceManager(IDirect3DDeviceManager9** ppManager, mfxU32 nAdapterNum)
{
  MSDK_CHECK_POINTER(ppManager, MFX_ERR_NULL_PTR);

  IDirect3D9Ex* d3d;
  Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d);

  if (!d3d)
  {
    return MFX_ERR_NULL_PTR;
  }

  POINT point = {0, 0};
  HWND window = WindowFromPoint(point);

  D3DPRESENT_PARAMETERS d3dParams;
  memset(&d3dParams, 0, sizeof(d3dParams));
  d3dParams.Windowed = TRUE;
  d3dParams.hDeviceWindow = window;
  d3dParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
  d3dParams.Flags = D3DPRESENTFLAG_VIDEO;
  d3dParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
  d3dParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
  d3dParams.BackBufferCount = 1;
  d3dParams.BackBufferFormat = D3DFMT_X8R8G8B8;
  d3dParams.BackBufferWidth = 0;
  d3dParams.BackBufferHeight = 0;

  CComPtr<IDirect3DDevice9Ex> d3dDevice = 0;
  HRESULT hr = d3d->CreateDeviceEx(
                                nAdapterNum,
                                D3DDEVTYPE_HAL,
                                window,
                                D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
                                &d3dParams,
                                NULL,
                                &d3dDevice);

  if (FAILED(hr) || !d3dDevice)
  {
    return MFX_ERR_NULL_PTR;
  }

  UINT resetToken = 0;
  CComPtr<IDirect3DDeviceManager9> d3dDeviceManager = 0;
  hr = DXVA2CreateDirect3DDeviceManager9(&resetToken, &d3dDeviceManager);

  if (FAILED(hr) || !d3dDeviceManager)
  {
    return MFX_ERR_NULL_PTR;
  }

  hr = d3dDeviceManager->ResetDevice(d3dDevice, resetToken);
  if (FAILED(hr))
  {
    return MFX_ERR_UNDEFINED_BEHAVIOR;
  }

  *ppManager = d3dDeviceManager.Detach();

  if (NULL == *ppManager)
  {
    return MFX_ERR_NULL_PTR;
  }

  return MFX_ERR_NONE;

} // mfxStatus CreateDeviceManager(IDirect3DDeviceManager9** ppManager)
#endif

mfxStatus InitFrameProcessor(sFrameProcessor* pProcessor, mfxVideoParam* pParams)
{
  mfxStatus sts = MFX_ERR_NONE;

  MSDK_CHECK_POINTER(pProcessor,          MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pParams,             MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pProcessor->pmfxVPP, MFX_ERR_NULL_PTR);

  // close VPP in case it was initialized
  sts = pProcessor->pmfxVPP->Close();
  MSDK_IGNORE_MFX_STS(sts, MFX_ERR_NOT_INITIALIZED);
  MSDK_CHECK_RESULT(sts,   MFX_ERR_NONE, sts);

  // init VPP
  sts = pProcessor->pmfxVPP->Init(pParams);
  MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
  MSDK_CHECK_RESULT(sts,   MFX_ERR_NONE, sts);

  return MFX_ERR_NONE;

} // mfxStatus InitFrameProcessor(sFrameProcessor* pProcessor, mfxVideoParam* pParams)

/* ******************************************************************* */

mfxStatus InitSurfaces(sMemoryAllocator* pAllocator, mfxFrameAllocRequest* pRequest, mfxFrameInfo* pInfo, mfxU32 indx)
{
  mfxStatus sts = MFX_ERR_NONE;
  mfxU16    nFrames, i;

  sts = pAllocator->pMfxAllocator->Alloc(pAllocator->pMfxAllocator->pthis, pRequest, &(pAllocator->response[indx]));
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

  nFrames = pAllocator->response[indx].NumFrameActual;

  pAllocator->pSurfaces[indx] = new mfxFrameSurface1 [nFrames];

  for (i = 0; i < nFrames; i++)
  {
    memset(&(pAllocator->pSurfaces[indx][i]), 0, sizeof(mfxFrameSurface1));
    pAllocator->pSurfaces[indx][i].Info = *pInfo;

    if( pAllocator->bUsedAsExternalAllocator )
    {
      pAllocator->pSurfaces[indx][i].Data.MemId = pAllocator->response[indx].mids[i];
    }
    else
    {
      sts = pAllocator->pMfxAllocator->Lock(pAllocator->pMfxAllocator->pthis,
                                            pAllocator->response[indx].mids[i],
                                            &(pAllocator->pSurfaces[indx][i].Data));
      MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));
    }
  }

  return sts;

} // mfxStatus InitSurfaces(...)

/* ******************************************************************* */

mfxStatus InitMemoryAllocator(sFrameProcessor* pProcessor, sMemoryAllocator* pAllocator, mfxVideoParam* pParams, sVppInputParams* pInParams)
{
  mfxStatus sts = MFX_ERR_NONE;
  mfxFrameAllocRequest request[2];// [0] - in, [1] - out
  //mfxFrameAllocRequest request_RGB;
  mfxFrameInfo requestFrameInfoRGB;

  MSDK_CHECK_POINTER(pProcessor,          MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pAllocator,          MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pParams,             MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pInParams,             MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pProcessor->pmfxVPP, MFX_ERR_NULL_PTR);

  MSDK_ZERO_MEMORY(request[VPP_IN]);
  MSDK_ZERO_MEMORY(request[VPP_OUT]);
  //MSDK_ZERO_MEMORY(request_RGB);

  // VppRequest[0] for input frames request, VppRequest[1] for output frames request
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

   if( pInParams->memType == D3D9_MEMORY )
  {
#ifdef D3D_SURFACES_SUPPORT
    // prepare device manager
    pAllocator->pDevice = new CD3D9Device();
    sts = pAllocator->pDevice->Init(0, 1, MSDKAdapter::GetNumber(pProcessor->mfxSession));
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

    mfxHDL hdl = 0;
    sts = pAllocator->pDevice->GetHandle(MFX_HANDLE_D3D9_DEVICE_MANAGER, &hdl);
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));
    sts = pProcessor->mfxSession.SetHandle(MFX_HANDLE_D3D9_DEVICE_MANAGER, hdl);
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

    // prepare allocator
    pAllocator->pMfxAllocator = new D3DFrameAllocator;

    D3DAllocatorParams *pd3dAllocParams = new D3DAllocatorParams;

    pd3dAllocParams->pManager = (IDirect3DDeviceManager9*)hdl;
    pAllocator->pAllocatorParams = pd3dAllocParams;

    /* In case of video memory we must provide mediasdk with external allocator
    thus we demonstrate "external allocator" usage model.
    Call SetAllocator to pass allocator to mediasdk */
    sts = pProcessor->mfxSession.SetFrameAllocator(pAllocator->pMfxAllocator);
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

    pAllocator->bUsedAsExternalAllocator = true;
#endif
  }
  else if( pInParams->memType == D3D11_MEMORY )
  {
#ifdef MFX_D3D11_SUPPORT
    pAllocator->pDevice = new CD3D11Device();

    sts = pAllocator->pDevice->Init(0, 1, MSDKAdapter::GetNumber(pProcessor->mfxSession));
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

    mfxHDL hdl = 0;
    sts = pAllocator->pDevice->GetHandle(MFX_HANDLE_D3D11_DEVICE, &hdl);
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));
    sts = pProcessor->mfxSession.SetHandle(MFX_HANDLE_D3D11_DEVICE, hdl);
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

    // prepare allocator
    pAllocator->pMfxAllocator = new D3D11FrameAllocator;

    D3D11AllocatorParams *pd3d11AllocParams = new D3D11AllocatorParams;

    pd3d11AllocParams->pDevice = (ID3D11Device*)hdl;
    pAllocator->pAllocatorParams = pd3d11AllocParams;

    sts = pProcessor->mfxSession.SetFrameAllocator(pAllocator->pMfxAllocator);
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

    pAllocator->bUsedAsExternalAllocator = true;
#endif
  }
  else if (pInParams->memType == VAAPI_MEMORY)
  {
#ifdef LIBVA_SUPPORT
    pAllocator->pDevice = CreateVAAPIDevice();
    MSDK_CHECK_POINTER(pAllocator->pDevice, MFX_ERR_NULL_PTR);

    sts = pAllocator->pDevice->Init(0, 1, MSDKAdapter::GetNumber(pProcessor->mfxSession));
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

    mfxHDL hdl = 0;
    sts = pAllocator->pDevice->GetHandle(MFX_HANDLE_VA_DISPLAY, &hdl);
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));
    sts = pProcessor->mfxSession.SetHandle(MFX_HANDLE_VA_DISPLAY, hdl);
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

    // prepare allocator
    pAllocator->pMfxAllocator = new vaapiFrameAllocator;

    vaapiAllocatorParams *pVaapiAllocParams = new vaapiAllocatorParams;

    pVaapiAllocParams->m_dpy = (VADisplay)hdl;
    pAllocator->pAllocatorParams = pVaapiAllocParams;

    sts = pProcessor->mfxSession.SetFrameAllocator(pAllocator->pMfxAllocator);
    MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

    pAllocator->bUsedAsExternalAllocator = true;
#endif
  }
  else
  {
#ifdef LIBVA_SUPPORT
    //in case of system memory allocator we also have to pass MFX_HANDLE_VA_DISPLAY to HW library
    mfxIMPL impl;
    pProcessor->mfxSession.QueryIMPL(&impl);

    if(MFX_IMPL_HARDWARE == MFX_IMPL_BASETYPE(impl))
    {
      pAllocator->pDevice = CreateVAAPIDevice();
      if (!pAllocator->pDevice) sts = MFX_ERR_MEMORY_ALLOC;
      MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

      mfxHDL hdl = 0;
      sts = pAllocator->pDevice->GetHandle(MFX_HANDLE_VA_DISPLAY, &hdl);
      MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

      sts = pProcessor->mfxSession.SetHandle(MFX_HANDLE_VA_DISPLAY, hdl);
      MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));
    }
#endif

    // prepare allocator
    pAllocator->pMfxAllocator = new SysMemFrameAllocator;

    /* In case of system memory we demonstrate "no external allocator" usage model.
    We don't call SetAllocator, mediasdk uses internal allocator.
    We use software allocator object only as a memory manager for application */
  }

  sts = pAllocator->pMfxAllocator->Init(pAllocator->pAllocatorParams);
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

  sts = pProcessor->pmfxVPP->QueryIOSurf(pParams, request);
  MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));

  //MSDK_MEMCPY(&request_RGB,&(request[VPP_IN]),sizeof(mfxFrameAllocRequest) );
  // alloc frames for vpp
  // [IN]
#if 0
  sts = InitSurfaces(pAllocator, &(request[VPP_IN]), &(pParams->vpp.In), VPP_IN);
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));
 #endif
  /**/
#if 0
  request_RGB.Info.FourCC = MFX_FOURCC_RGB4;
  request_RGB.Info.ChromaFormat = 0;
  MSDK_MEMCPY(&requestFrameInfoRGB, &(pParams->vpp.In), sizeof(mfxFrameInfo));
  requestFrameInfoRGB.ChromaFormat = 0;
  requestFrameInfoRGB.FourCC = MFX_FOURCC_RGB4;

  sts = InitSurfaces(pAllocator, &request_RGB, &requestFrameInfoRGB, VPP_IN_RGB);
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));
#endif
  // [OUT]
 #if 0
  sts = InitSurfaces(pAllocator, &(request[VPP_OUT]), &(pParams->vpp.Out), VPP_OUT);
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMemoryAllocator(pAllocator));
#endif
  return MFX_ERR_NONE;

} // mfxStatus InitMemoryAllocator(...)

/* ******************************************************************* */

mfxStatus InitResources(sAppResources* pResources, mfxVideoParam* pParams, sVppInputParams* pInParams)
{
  mfxStatus sts = MFX_ERR_NONE;
  //mfxVideoParam videoParams;

  MSDK_CHECK_POINTER(pResources, MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pParams,    MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pInParams, MFX_ERR_NULL_PTR);

  sts = CreateFrameProcessor(&pResources->Processor, pParams, pInParams);
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeResources(pResources));

  //memcpy(&videoParams,pParams,sizeof(mfxVideoParam));

  sts = InitMemoryAllocator(&pResources->Processor, &pResources->Allocator, /*&videoParams*/pParams,pInParams);
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeResources(pResources));

  sts = InitFrameProcessor(&pResources->Processor, pParams);
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeResources(pResources));

  return sts;

} // mfxStatus InitResources(sAppResources* pResources, sVppInputParams* pInParams)

/* ******************************************************************* */

void WipeFrameProcessor(sFrameProcessor* pProcessor)
{
  MSDK_CHECK_POINTER_NO_RET(pProcessor);
  MSDK_SAFE_DELETE(pProcessor->pmfxVPP);
  if (pProcessor->mfxSession.operator mfxSession())
  {
      pProcessor->mfxSession.Close();
  }

} // void WipeFrameProcessor(sFrameProcessor* pProcessor)

void WipeMemoryAllocator(sMemoryAllocator* pAllocator)
{
  MSDK_CHECK_POINTER_NO_RET(pAllocator);

  MSDK_SAFE_DELETE_ARRAY(pAllocator->pSurfaces[VPP_IN]);
  MSDK_SAFE_DELETE_ARRAY(pAllocator->pSurfaces[VPP_IN_RGB]);
  MSDK_SAFE_DELETE_ARRAY(pAllocator->pSurfaces[VPP_OUT]);

  // delete frames
  if (pAllocator->pMfxAllocator)
  {
    pAllocator->pMfxAllocator->Free(pAllocator->pMfxAllocator->pthis, &pAllocator->response[VPP_IN]);
    pAllocator->pMfxAllocator->Free(pAllocator->pMfxAllocator->pthis, &pAllocator->response[VPP_OUT]);
  }

  // delete allocator
  MSDK_SAFE_DELETE(pAllocator->pMfxAllocator);
  MSDK_SAFE_DELETE(pAllocator->pDevice);

  // delete allocator parameters
  MSDK_SAFE_DELETE(pAllocator->pAllocatorParams);


} // void WipeMemoryAllocator(sMemoryAllocator* pAllocator)

/* ******************************************************************* */
//获取毫秒级别
static unsigned int vpp_utils_get_run_time(void)
{	
	unsigned int msec;
#ifndef WIN32
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	msec = tp.tv_sec;
	msec = msec * 1000 + tp.tv_nsec / 1000000;
#else
	return vpp_get_run_time();
#endif
	return msec;
}
mfxStatus ResetResources(sAppResources* pResources, mfxVideoParam* pParams, sVppInputParams* pInParams)
{
	mfxStatus sts = MFX_ERR_NONE;
	unsigned int currentTime = 0;
	//mfxVideoParam tmpParams;

  MSDK_CHECK_POINTER(pResources,MFX_ERR_NULL_PTR);
  //MSDK_SAFE_DELETE(pResources->Processor.pmfxVPP);
#if 0
  WipeMemoryAllocator(&pResources->Allocator);
  mfxVideoParam videoParams;

  MSDK_CHECK_POINTER(pResources, MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pParams,    MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(pInParams, MFX_ERR_NULL_PTR);

  // VPP
  //pResources->Processor.pmfxVPP = new MFXVideoVPP(pResources->Processor.mfxSession);
  
  memcpy(&videoParams,pParams,sizeof(mfxVideoParam));

  sts = InitMemoryAllocator(&pResources->Processor, &pResources->Allocator, &videoParams);
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeResources(pResources));

  //sts = InitFrameProcessor(&pResources->Processor, pParams);
#endif

   //memset(&tmpParams,0,sizeof(tmpParams));
#if 0
  // close VPP in case it was initialized
  currentTime = vpp_utils_get_run_time();
  sts = pResources->Processor.pmfxVPP->Close();
  MSDK_IGNORE_MFX_STS(sts, MFX_ERR_NOT_INITIALIZED);
  MSDK_CHECK_RESULT(sts,   MFX_ERR_NONE, sts);
  printf("============.pmfxVPP->Close time:%d\n",vpp_utils_get_run_time() - currentTime);
  currentTime = vpp_utils_get_run_time();
  sts = pResources->Processor.pmfxVPP->Init(pParams);
  MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeResources(pResources));
  printf("============.pmfxVPP->Init time:%d\n",vpp_utils_get_run_time() - currentTime);
  //sts = pResources->Processor.pmfxVPP->GetVideoParam(&tmpParams);
  //MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeResources(pResources)); 
  //printf("=========ResetResources:src:%d,%d,dst:%d,%d\n",pParams->vpp.Out.CropW,pParams->vpp.Out.CropH,tmpParams.vpp.Out.CropW,tmpParams.vpp.Out.CropH);
#else
  sts = pResources->Processor.pmfxVPP->Reset(pParams);
  MSDK_CHECK_RESULT(sts,   MFX_ERR_NONE, sts);
#endif
  return sts;
  
} // void WipeResources(sAppResources* pResources)

/* ******************************************************************* */

/* ******************************************************************* */

void WipeResources(sAppResources* pResources)
{
  MSDK_CHECK_POINTER_NO_RET(pResources);
  WipeFrameProcessor(&pResources->Processor);
  WipeMemoryAllocator(&pResources->Allocator);
} // void WipeResources(sAppResources* pResources)

/* ******************************************************************* */


mfxU16 GetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize)
{
    if (pSurfacesPool)
    {
        for (mfxU16 i = 0; i < nPoolSize; i++)
        {
            if (0 == pSurfacesPool[i].Data.Locked)
            {
                return i;
            }
        }
    }

    return MSDK_INVALID_SURF_IDX;
}

mfxStatus GetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize, mfxFrameSurface1** ppSurface)
{
  MSDK_CHECK_POINTER(pSurfacesPool, MFX_ERR_NULL_PTR);
  MSDK_CHECK_POINTER(ppSurface,     MFX_ERR_NULL_PTR);

  mfxU32 timeToSleep = 10; // milliseconds
  mfxU32 numSleeps = MSDK_VPP_WAIT_INTERVAL / timeToSleep + 1; // at least 1

  mfxU32 i = 0;

  //wait if there's no free surface
  while ((MSDK_INVALID_SURF_IDX == GetFreeSurfaceIndex(pSurfacesPool, nPoolSize)) && (i < numSleeps))
  {
    MSDK_SLEEP(timeToSleep);
    i++;
  }

  mfxU16 index = GetFreeSurfaceIndex(pSurfacesPool, nPoolSize);

  if (index < nPoolSize)
  {
    *ppSurface = &(pSurfacesPool[index]);
    return MFX_ERR_NONE;
  }

  return MFX_ERR_NOT_ENOUGH_BUFFER;

} // mfxStatus GetFreeSurface(...)

/* ******************************************************************* */
mfxStatus ConfigVideoEnhancementFilters( void *arg, sAppResources* pResources )
{
	sVppUsrParams* pParams = (sVppUsrParams* )arg;
    mfxVideoParam*   pVppParam = &pResources->VppParams;
    mfxU32  enabledFilterCount = 0;

    // [0] common tuning params
    pVppParam->NumExtParam = 0;
    // to simplify logic
    pVppParam->ExtParam    = (mfxExtBuffer**)pResources->pExtBuf;

    pResources->extDoUse.Header.BufferId = MFX_EXTBUFF_VPP_DOUSE;
    pResources->extDoUse.Header.BufferSz = sizeof(mfxExtVPPDoUse);
    pResources->extDoUse.NumAlg  = 0;
    pResources->extDoUse.AlgList = NULL;

    // [1] video enhancement algorithms can be enabled with default parameters
    if( VPP_FILTER_DISABLED != pParams->denoiseParam.mode )
    {
        pResources->tabDoUseAlg[enabledFilterCount++] = MFX_EXTBUFF_VPP_DENOISE;
    }
    if( VPP_FILTER_DISABLED != pParams->vaParam.mode )
    {
        pResources->tabDoUseAlg[enabledFilterCount++] = MFX_EXTBUFF_VPP_SCENE_ANALYSIS;
    }
    if( VPP_FILTER_DISABLED != pParams->procampParam.mode )
    {
        pResources->tabDoUseAlg[enabledFilterCount++] = MFX_EXTBUFF_VPP_PROCAMP;
    }
    if( VPP_FILTER_DISABLED != pParams->detailParam.mode )
    {
        pResources->tabDoUseAlg[enabledFilterCount++] = MFX_EXTBUFF_VPP_DETAIL;
    }
    if( VPP_FILTER_DISABLED != pParams->istabParam.mode )
    {
        pResources->tabDoUseAlg[enabledFilterCount++] = MFX_EXTBUFF_VPP_IMAGE_STABILIZATION;
    }

    if( enabledFilterCount > 0 )
    {
        pResources->extDoUse.NumAlg  = enabledFilterCount;
        pResources->extDoUse.AlgList = pResources->tabDoUseAlg;
        pVppParam->ExtParam[pVppParam->NumExtParam++] = (mfxExtBuffer*)&(pResources->extDoUse);
    }

    // [2] video enhancement algorithms can be configured
    if( VPP_FILTER_ENABLED_CONFIGURED == pParams->denoiseParam.mode )
    {
        pResources->denoiseConfig.Header.BufferId = MFX_EXTBUFF_VPP_DENOISE;
        pResources->denoiseConfig.Header.BufferSz = sizeof(mfxExtVPPDenoise);

        pResources->denoiseConfig.DenoiseFactor   = pParams->denoiseParam.factor;

        pVppParam->ExtParam[pVppParam->NumExtParam++] = (mfxExtBuffer*)&(pResources->denoiseConfig);
    }
    if( VPP_FILTER_ENABLED_CONFIGURED == pParams->vaParam.mode )
    {
        // video analysis filters isn't configured
    }
    if( VPP_FILTER_ENABLED_CONFIGURED == pParams->procampParam.mode )
    {
        pResources->procampConfig.Header.BufferId = MFX_EXTBUFF_VPP_PROCAMP;
        pResources->procampConfig.Header.BufferSz = sizeof(mfxExtVPPProcAmp);

        pResources->procampConfig.Hue        = pParams->procampParam.hue;
        pResources->procampConfig.Saturation = pParams->procampParam.saturation;
        pResources->procampConfig.Contrast   = pParams->procampParam.contrast;
        pResources->procampConfig.Brightness = pParams->procampParam.brightness;

        pVppParam->ExtParam[pVppParam->NumExtParam++] = (mfxExtBuffer*)&(pResources->procampConfig);
    }
    if( VPP_FILTER_ENABLED_CONFIGURED == pParams->detailParam.mode )
    {
        pResources->detailConfig.Header.BufferId = MFX_EXTBUFF_VPP_DETAIL;
        pResources->detailConfig.Header.BufferSz = sizeof(mfxExtVPPDetail);

        pResources->detailConfig.DetailFactor   = pParams->detailParam.factor;

        pVppParam->ExtParam[pVppParam->NumExtParam++] = (mfxExtBuffer*)&(pResources->detailConfig);
    }
    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->deinterlaceParam.mode)
    {
        pResources->deinterlaceConfig.Header.BufferId = MFX_EXTBUFF_VPP_DEINTERLACING;
        pResources->deinterlaceConfig.Header.BufferSz = sizeof(mfxExtVPPDeinterlacing);
        pResources->deinterlaceConfig.Mode = pParams->deinterlaceParam.algorithm;
        pResources->deinterlaceConfig.TelecinePattern = pParams->deinterlaceParam.tc_pattern;
        pResources->deinterlaceConfig.TelecineLocation = pParams->deinterlaceParam.tc_pos;

        pVppParam->ExtParam[pVppParam->NumExtParam++] = (mfxExtBuffer*)&(pResources->deinterlaceConfig);
    }
    if( VPP_FILTER_ENABLED_CONFIGURED == pParams->istabParam.mode )
    {
        pResources->istabConfig.Header.BufferId = MFX_EXTBUFF_VPP_IMAGE_STABILIZATION;
        pResources->istabConfig.Header.BufferSz = sizeof(mfxExtVPPImageStab);
        pResources->istabConfig.Mode            = pParams->istabParam.istabMode;

        pVppParam->ExtParam[pVppParam->NumExtParam++] = (mfxExtBuffer*)&(pResources->istabConfig);
    }
    if( VPP_FILTER_ENABLED_CONFIGURED == pParams->compositionParam.mode )
    {
        pResources->compositeConfig.Header.BufferId = MFX_EXTBUFF_VPP_COMPOSITE;
        pResources->compositeConfig.Header.BufferSz = sizeof(mfxExtVPPComposite);
        pResources->compositeConfig.NumInputStream  = pParams->numStreams;
        pResources->compositeConfig.InputStream     = new mfxVPPCompInputStream[pResources->compositeConfig.NumInputStream];
        memset(pResources->compositeConfig.InputStream, 0, sizeof(mfxVPPCompInputStream) * pResources->compositeConfig.NumInputStream);

        for (int i = 0; i < pResources->compositeConfig.NumInputStream; i++)
        {
            pResources->compositeConfig.InputStream[i].DstX = pParams->compositionParam.streamInfo[i].compStream.DstX;
            pResources->compositeConfig.InputStream[i].DstY = pParams->compositionParam.streamInfo[i].compStream.DstY;
            pResources->compositeConfig.InputStream[i].DstW = pParams->compositionParam.streamInfo[i].compStream.DstW;
            pResources->compositeConfig.InputStream[i].DstH = pParams->compositionParam.streamInfo[i].compStream.DstH;
            if (pParams->compositionParam.streamInfo[i].compStream.GlobalAlphaEnable != 0 )
            {
                pResources->compositeConfig.InputStream[i].GlobalAlphaEnable = pParams->compositionParam.streamInfo[i].compStream.GlobalAlphaEnable;
                pResources->compositeConfig.InputStream[i].GlobalAlpha = pParams->compositionParam.streamInfo[i].compStream.GlobalAlpha;
            }
            if (pParams->compositionParam.streamInfo[i].compStream.LumaKeyEnable != 0 )
            {
                pResources->compositeConfig.InputStream[i].LumaKeyEnable = pParams->compositionParam.streamInfo[i].compStream.LumaKeyEnable;
                pResources->compositeConfig.InputStream[i].LumaKeyMin = pParams->compositionParam.streamInfo[i].compStream.LumaKeyMin;
                pResources->compositeConfig.InputStream[i].LumaKeyMax = pParams->compositionParam.streamInfo[i].compStream.LumaKeyMax;
            }
            if (pParams->compositionParam.streamInfo[i].compStream.PixelAlphaEnable != 0 )
            {
                pResources->compositeConfig.InputStream[i].PixelAlphaEnable = pParams->compositionParam.streamInfo[i].compStream.PixelAlphaEnable;
            }
        } // for (int i = 0; i < pResources->compositeConfig.NumInputStream; i++)

        pVppParam->ExtParam[pVppParam->NumExtParam++] = (mfxExtBuffer*)&(pResources->compositeConfig);
    }
    if( VPP_FILTER_ENABLED_CONFIGURED == pParams->frcParam.mode )
    {
        pResources->frcConfig.Header.BufferId = MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION;
        pResources->frcConfig.Header.BufferSz = sizeof(mfxExtVPPFrameRateConversion);

        pResources->frcConfig.Algorithm   = (mfxU16)pParams->frcParam.algorithm;//MFX_FRCALGM_DISTRIBUTED_TIMESTAMP;

        pVppParam->ExtParam[pVppParam->NumExtParam++] = (mfxExtBuffer*)&(pResources->frcConfig);
    }

    // confirm configuration
    if( 0 == pVppParam->NumExtParam )
    {
        pVppParam->ExtParam = NULL;
    }

    return MFX_ERR_NONE;

} // mfxStatus ConfigVideoEnhancementFilters( sAppResources* pResources, mfxVideoParam* pParams )

/* EOF */
