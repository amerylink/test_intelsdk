/*********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2005-2015 Intel Corporation. All Rights Reserved.

**********************************************************************************/

#include "mfx_samples_config.h"
#include "sample_defs.h"

#if defined(_WIN32) || defined(_WIN64)
#include <tchar.h>
#include <windows.h>
#endif

#include <ctime>
#include <algorithm>
#include "pipeline_decode.h"
#include "sysmem_allocator.h"

#if defined(_WIN32) || defined(_WIN64)
#include "d3d_allocator.h"
#include "d3d11_allocator.h"
#include "d3d_device.h"
#include "d3d11_device.h"
#endif

#if defined LIBVA_SUPPORT
#include "vaapi_allocator.h"
#include "vaapi_device.h"
#endif
#include "SDK_codec.h"

#pragma comment(lib,"sample_common.lib")
#pragma warning(disable : 4100)

#define __SYNC_WA // avoid sync issue on Media SDK side

CDecodingPipeline::CDecodingPipeline()
{
    MSDK_ZERO_MEMORY(m_mfxBS);

    m_pmfxDEC = NULL;
    MSDK_ZERO_MEMORY(m_mfxVideoParams);

    m_pMFXAllocator = NULL;
    m_pmfxAllocatorParams = NULL;
    m_memType = SYSTEM_MEMORY;
    m_bExternalAlloc = false;
    MSDK_ZERO_MEMORY(m_mfxResponse);

    m_pCurrentFreeSurface = NULL;
    m_pCurrentFreeOutputSurface = NULL;
    m_pCurrentOutputSurface = NULL;

    m_pDeliverOutputSemaphore = NULL;
    m_pDeliveredEvent = NULL;
    m_error = MFX_ERR_NONE;
    m_bStopDeliverLoop = false;

    m_eWorkMode = MODE_PERFORMANCE;
    m_bIsMVC = false;
    m_bIsExtBuffers = false;
    m_bIsVideoWall = false;
    m_bIsCompleteFrame = false;
    m_bPrintLatency = false;

    m_nTimeout = 1;
    m_nMaxFps = 0;

    m_vLatency.reserve(1000); // reserve some space to reduce dynamic reallocation impact on pipeline execution

    m_hwdev = NULL;

#if D3D_SURFACES_SUPPORT
    m_pS3DControl = NULL;
#endif
}

CDecodingPipeline::~CDecodingPipeline()
{
    Close();
}

mfxStatus CDecodingPipeline::Init_CreateSession(sInputParams_dec *pParams)
{
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    mfxStatus sts = MFX_ERR_NONE;
    m_nDecNo=pParams->nDecNo;

    m_memType = pParams->memType;
    m_nMaxFps = pParams->nMaxFPS;
    m_nFrames = pParams->nFrames ? pParams->nFrames : MFX_INFINITE;

    mfxInitParam initPar;
    mfxExtThreadsParam threadsPar;
    mfxExtBuffer* extBufs[1];
    mfxVersion version;     // real API version with which library is initialized

    MSDK_ZERO_MEMORY(initPar);
    MSDK_ZERO_MEMORY(threadsPar);

    // we set version to 1.0 and later we will query actual version of the library which will got leaded
    initPar.Version.Major = 1;
    initPar.Version.Minor = 0;

    initPar.GPUCopy = pParams->gpuCopy;

    init_ext_buffer(threadsPar);

    bool needInitExtPar = false;

    if (pParams->nThreadsNum) {
        threadsPar.NumThread = pParams->nThreadsNum;
        needInitExtPar = true;
    }
    if (pParams->SchedulingType) {
        threadsPar.SchedulingType = pParams->SchedulingType;
        needInitExtPar = true;
    }
    if (pParams->Priority) {
        threadsPar.Priority = pParams->Priority;
        needInitExtPar = true;
    }
    if (needInitExtPar) {
        extBufs[0] = (mfxExtBuffer*)&threadsPar;
        initPar.ExtParam = extBufs;
        initPar.NumExtParam = 1;
    }

    // Init session
    if (pParams->bUseHWLib) {
        // try searching on all display adapters
        initPar.Implementation = MFX_IMPL_HARDWARE_ANY;

        // if d3d11 surfaces are used ask the library to run acceleration through D3D11
        // feature may be unsupported due to OS or MSDK API version
        if (D3D11_MEMORY == pParams->memType)
            initPar.Implementation |= MFX_IMPL_VIA_D3D11;

        sts = m_mfxSession.InitEx(initPar);

        // MSDK API version may not support multiple adapters - then try initialize on the default
        if (MFX_ERR_NONE != sts) {
            initPar.Implementation = (initPar.Implementation & !MFX_IMPL_HARDWARE_ANY) | MFX_IMPL_HARDWARE;
            sts = m_mfxSession.InitEx(initPar);
        }
    } else {
        initPar.Implementation = MFX_IMPL_SOFTWARE;
        sts = m_mfxSession.InitEx(initPar);
    }

    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    sts = MFXQueryVersion(m_mfxSession , &version); // get real API version of the loaded library
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // create decoder
    m_pmfxDEC = new MFXVideoDECODE(m_mfxSession);
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_MEMORY_ALLOC);

    // set video type in parameters
    m_mfxVideoParams.mfx.CodecId = pParams->videoType;
#if 0
    sts = InitMfxBitstream(&m_mfxBS, 1920 * 1080);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    m_mfxBS.DataOffset = 0;
    m_mfxBS.DataLength = 0;
#endif
    if (CheckVersion(&version, MSDK_FEATURE_PLUGIN_API)) {
        /* Here we actually define the following codec initialization scheme:
        *  1. If plugin path or guid is specified: we load user-defined plugin (example: VP8 sample decoder plugin)
        *  2. If plugin path not specified:
        *    2.a) we check if codec is distributed as a mediasdk plugin and load it if yes
        *    2.b) if codec is not in the list of mediasdk plugins, we assume, that it is supported inside mediasdk library
        */
        // Load user plug-in, should go after CreateAllocator function (when all callbacks were initialized)
        if (pParams->pluginParams.type == MFX_PLUGINLOAD_TYPE_FILE && strlen(pParams->pluginParams.strPluginPath))
        {
            m_pUserModule.reset(new MFXVideoUSER(m_mfxSession));
            if (pParams->videoType == CODEC_VP8 || pParams->videoType == MFX_CODEC_HEVC)
            {
                m_pPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, m_mfxSession, pParams->pluginParams.pluginGuid, 1, pParams->pluginParams.strPluginPath, (mfxU32)strlen(pParams->pluginParams.strPluginPath)));
            }
            if (m_pPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
        }
        else
        {
            if (AreGuidsEqual(pParams->pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
            {
                mfxIMPL impl = pParams->bUseHWLib ? MFX_IMPL_HARDWARE : MFX_IMPL_SOFTWARE;
                pParams->pluginParams.pluginGuid = msdkGetPluginUID(impl, MSDK_VDECODE, pParams->videoType);
            }
            if (!AreGuidsEqual(pParams->pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
            {
                m_pPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, m_mfxSession, pParams->pluginParams.pluginGuid, 1));
                if (m_pPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
            }
        }
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }

    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    return sts;
}

mfxStatus CDecodingPipeline::Init_DecHeader(sInputParams_dec *pParams)
{
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    mfxStatus sts = MFX_ERR_NONE;

    // Populate parameters. Involves DecodeHeader call
    sts = InitMfxParams(pParams);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    return sts;
}

mfxStatus CDecodingPipeline::Init_CreateDec(sInputParams_dec *pParams)
{
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    mfxStatus sts = MFX_ERR_NONE;

    m_eWorkMode = pParams->mode;
    if (m_eWorkMode == MODE_FILE_DUMP) {
        // prepare YUV file writer
        //sts = m_FileWriter.Init(pParams->strDstFile, pParams->numViews);
    } else if ((m_eWorkMode != MODE_PERFORMANCE) && (m_eWorkMode != MODE_RENDERING)) {
        msdk_printf(MSDK_STRING("error: unsupported work mode\n"));
        sts = MFX_ERR_UNSUPPORTED;
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // create device and allocator
    sts = CreateAllocator();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // in case of HW accelerated decode frames must be allocated prior to decoder initialization
    sts = AllocFrames();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    sts = m_pmfxDEC->Init(&m_mfxVideoParams);
    if (MFX_WRN_PARTIAL_ACCELERATION == sts)
    {
        msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    if (m_eWorkMode == MODE_RENDERING)
    {
        sts = CreateRenderingWindow(pParams, m_bIsMVC && (m_memType == D3D9_MEMORY));
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }

    return sts;
}

mfxStatus CDecodingPipeline::Init_reach(sInputParams_reach *pParams)
{
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    mfxStatus sts = MFX_ERR_NONE;
    m_nDecNo=pParams->nDecNo;

    m_memType = pParams->memType;
    //m_nMaxFps = pParams->nMaxFPS;
    //m_nFrames = pParams->nFrames ? pParams->nFrames : MFX_INFINITE;

    mfxInitParam initPar;
    mfxExtThreadsParam threadsPar;
    mfxExtBuffer* extBufs[1];
    mfxVersion version;     // real API version with which library is initialized

    MSDK_ZERO_MEMORY(initPar);
    MSDK_ZERO_MEMORY(threadsPar);

    // we set version to 1.0 and later we will query actual version of the library which will got leaded
    initPar.Version.Major = 1;
    initPar.Version.Minor = 0;

    initPar.GPUCopy = pParams->gpuCopy;

    //init_ext_buffer(threadsPar);

    bool needInitExtPar = false;

    if (pParams->nThreadsNum) {
        threadsPar.NumThread = pParams->nThreadsNum;
        needInitExtPar = true;
    }
    if (pParams->SchedulingType) {
        threadsPar.SchedulingType = pParams->SchedulingType;
        needInitExtPar = true;
    }
    if (pParams->Priority) {
        threadsPar.Priority = pParams->Priority;
        needInitExtPar = true;
    }
    if (needInitExtPar) {
        extBufs[0] = (mfxExtBuffer*)&threadsPar;
        initPar.ExtParam = extBufs;
        initPar.NumExtParam = 1;
    }

    // Init session
    if (pParams->bUseHWLib) {
        // try searching on all display adapters
        initPar.Implementation = MFX_IMPL_HARDWARE_ANY;

        // if d3d11 surfaces are used ask the library to run acceleration through D3D11
        // feature may be unsupported due to OS or MSDK API version
        if (D3D11_MEMORY == pParams->memType)
            initPar.Implementation |= MFX_IMPL_VIA_D3D11;

        sts = m_mfxSession.InitEx(initPar);

        // MSDK API version may not support multiple adapters - then try initialize on the default
        if (MFX_ERR_NONE != sts) {
            initPar.Implementation = (initPar.Implementation & !MFX_IMPL_HARDWARE_ANY) | MFX_IMPL_HARDWARE;
            sts = m_mfxSession.InitEx(initPar);
        }
    } else {
        initPar.Implementation = MFX_IMPL_SOFTWARE;
        sts = m_mfxSession.InitEx(initPar);
    }

    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    sts = MFXQueryVersion(m_mfxSession , &version); // get real API version of the loaded library
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // create decoder
    m_pmfxDEC = new MFXVideoDECODE(m_mfxSession);
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_MEMORY_ALLOC);

    // set video type in parameters
    m_mfxVideoParams.mfx.CodecId = pParams->CodecId;

    if (CheckVersion(&version, MSDK_FEATURE_PLUGIN_API)) {
        /* Here we actually define the following codec initialization scheme:
        *  1. If plugin path or guid is specified: we load user-defined plugin (example: VP8 sample decoder plugin)
        *  2. If plugin path not specified:
        *    2.a) we check if codec is distributed as a mediasdk plugin and load it if yes
        *    2.b) if codec is not in the list of mediasdk plugins, we assume, that it is supported inside mediasdk library
        */
        // Load user plug-in, should go after CreateAllocator function (when all callbacks were initialized)
        if (pParams->pluginParams.type == MFX_PLUGINLOAD_TYPE_FILE && strlen(pParams->pluginParams.strPluginPath))
        {
            m_pUserModule.reset(new MFXVideoUSER(m_mfxSession));
            if (pParams->CodecId == CODEC_VP8 || pParams->CodecId == MFX_CODEC_HEVC)
            {
                m_pPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, m_mfxSession, pParams->pluginParams.pluginGuid, 1, pParams->pluginParams.strPluginPath, (mfxU32)strlen(pParams->pluginParams.strPluginPath)));
            }
            if (m_pPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
        }
        else
        {
            if (AreGuidsEqual(pParams->pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
            {
                mfxIMPL impl = pParams->bUseHWLib ? MFX_IMPL_HARDWARE : MFX_IMPL_SOFTWARE;
                pParams->pluginParams.pluginGuid = msdkGetPluginUID(impl, MSDK_VDECODE, pParams->CodecId);
            }
            if (!AreGuidsEqual(pParams->pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
            {
                m_pPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, m_mfxSession, pParams->pluginParams.pluginGuid, 1));
                if (m_pPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
            }
        }
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }

    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // Populate parameters. Involves DecodeHeader call
    sts = InitMfxParams_reach(pParams);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // create device and allocator
    sts = CreateAllocator();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // in case of HW accelerated decode frames must be allocated prior to decoder initialization
    sts = AllocFrames();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    sts = m_pmfxDEC->Init(&m_mfxVideoParams);
    if (MFX_WRN_PARTIAL_ACCELERATION == sts)
    {
        msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    return sts;
}
#if 0
mfxStatus CDecodingPipeline::Init(sInputParams_dec *pParams)
{
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    mfxStatus sts = MFX_ERR_NONE;
    m_nDecNo=pParams->nDecNo;

    m_memType = pParams->memType;
    m_nMaxFps = pParams->nMaxFPS;
    m_nFrames = pParams->nFrames ? pParams->nFrames : MFX_INFINITE;

    mfxInitParam initPar;
    mfxExtThreadsParam threadsPar;
    mfxExtBuffer* extBufs[1];
    mfxVersion version;     // real API version with which library is initialized

    MSDK_ZERO_MEMORY(initPar);
    MSDK_ZERO_MEMORY(threadsPar);

    // we set version to 1.0 and later we will query actual version of the library which will got leaded
    initPar.Version.Major = 1;
    initPar.Version.Minor = 0;

    initPar.GPUCopy = pParams->gpuCopy;

    init_ext_buffer(threadsPar);

    bool needInitExtPar = false;

    if (pParams->nThreadsNum) {
        threadsPar.NumThread = pParams->nThreadsNum;
        needInitExtPar = true;
    }
    if (pParams->SchedulingType) {
        threadsPar.SchedulingType = pParams->SchedulingType;
        needInitExtPar = true;
    }
    if (pParams->Priority) {
        threadsPar.Priority = pParams->Priority;
        needInitExtPar = true;
    }
    if (needInitExtPar) {
        extBufs[0] = (mfxExtBuffer*)&threadsPar;
        initPar.ExtParam = extBufs;
        initPar.NumExtParam = 1;
    }

    // Init session
    if (pParams->bUseHWLib) {
        // try searching on all display adapters
        initPar.Implementation = MFX_IMPL_HARDWARE_ANY;

        // if d3d11 surfaces are used ask the library to run acceleration through D3D11
        // feature may be unsupported due to OS or MSDK API version
        if (D3D11_MEMORY == pParams->memType)
            initPar.Implementation |= MFX_IMPL_VIA_D3D11;

        sts = m_mfxSession.InitEx(initPar);

        // MSDK API version may not support multiple adapters - then try initialize on the default
        if (MFX_ERR_NONE != sts) {
            initPar.Implementation = (initPar.Implementation & !MFX_IMPL_HARDWARE_ANY) | MFX_IMPL_HARDWARE;
            sts = m_mfxSession.InitEx(initPar);
        }
    } else {
        initPar.Implementation = MFX_IMPL_SOFTWARE;
        sts = m_mfxSession.InitEx(initPar);
    }

    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    sts = MFXQueryVersion(m_mfxSession , &version); // get real API version of the loaded library
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // create decoder
    m_pmfxDEC = new MFXVideoDECODE(m_mfxSession);
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_MEMORY_ALLOC);

    // set video type in parameters
    m_mfxVideoParams.mfx.CodecId = pParams->videoType;

    if (CheckVersion(&version, MSDK_FEATURE_PLUGIN_API)) {
        /* Here we actually define the following codec initialization scheme:
        *  1. If plugin path or guid is specified: we load user-defined plugin (example: VP8 sample decoder plugin)
        *  2. If plugin path not specified:
        *    2.a) we check if codec is distributed as a mediasdk plugin and load it if yes
        *    2.b) if codec is not in the list of mediasdk plugins, we assume, that it is supported inside mediasdk library
        */
        // Load user plug-in, should go after CreateAllocator function (when all callbacks were initialized)
        if (pParams->pluginParams.type == MFX_PLUGINLOAD_TYPE_FILE && strlen(pParams->pluginParams.strPluginPath))
        {
            m_pUserModule.reset(new MFXVideoUSER(m_mfxSession));
            if (pParams->videoType == CODEC_VP8 || pParams->videoType == MFX_CODEC_HEVC)
            {
                m_pPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, m_mfxSession, pParams->pluginParams.pluginGuid, 1, pParams->pluginParams.strPluginPath, (mfxU32)strlen(pParams->pluginParams.strPluginPath)));
            }
            if (m_pPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
        }
        else
        {
            if (AreGuidsEqual(pParams->pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
            {
                mfxIMPL impl = pParams->bUseHWLib ? MFX_IMPL_HARDWARE : MFX_IMPL_SOFTWARE;
                pParams->pluginParams.pluginGuid = msdkGetPluginUID(impl, MSDK_VDECODE, pParams->videoType);
            }
            if (!AreGuidsEqual(pParams->pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
            {
                m_pPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, m_mfxSession, pParams->pluginParams.pluginGuid, 1));
                if (m_pPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
            }
        }
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }

    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // Populate parameters. Involves DecodeHeader call
    sts = InitMfxParams(pParams);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    m_eWorkMode = pParams->mode;
    if (m_eWorkMode == MODE_FILE_DUMP) {
        // prepare YUV file writer
        sts = m_FileWriter.Init(pParams->strDstFile, pParams->numViews);
    } else if ((m_eWorkMode != MODE_PERFORMANCE) && (m_eWorkMode != MODE_RENDERING)) {
        msdk_printf(MSDK_STRING("error: unsupported work mode\n"));
        sts = MFX_ERR_UNSUPPORTED;
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // create device and allocator
    sts = CreateAllocator();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // in case of HW accelerated decode frames must be allocated prior to decoder initialization
    sts = AllocFrames();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    sts = m_pmfxDEC->Init(&m_mfxVideoParams);
    if (MFX_WRN_PARTIAL_ACCELERATION == sts)
    {
        msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    if (m_eWorkMode == MODE_RENDERING)
    {
        sts = CreateRenderingWindow(pParams, m_bIsMVC && (m_memType == D3D9_MEMORY));
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }

    return sts;
}
#else
mfxStatus CDecodingPipeline::Init(sInputParams_dec *pParams)
{
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    mfxStatus sts = MFX_ERR_NONE;
	m_nDecNo=pParams->nDecNo;
    // prepare input stream file reader
    // for VP8 complete and single frame reader is a requirement
    // create reader that supports completeframe mode for latency oriented scenarios
    if (pParams->bLowLat || pParams->bCalLat)
    {
        switch (pParams->videoType)
        {
        case MFX_CODEC_HEVC:
        case MFX_CODEC_AVC:
            m_FileReader.reset(new CH264FrameReader());
            m_bIsCompleteFrame = true;
            m_bPrintLatency = pParams->bCalLat;
            break;
        case MFX_CODEC_JPEG:
            m_FileReader.reset(new CJPEGFrameReader());
            m_bIsCompleteFrame = true;
            m_bPrintLatency = pParams->bCalLat;
            break;
        case CODEC_VP8:
            m_FileReader.reset(new CIVFFrameReader());
            m_bIsCompleteFrame = true;
            m_bPrintLatency = pParams->bCalLat;
            break;
        default:
            return MFX_ERR_UNSUPPORTED; // latency mode is supported only for H.264 and JPEG codecs
        }
    }
    else
    {
        switch (pParams->videoType)
        {
        case CODEC_VP8:
            m_FileReader.reset(new CIVFFrameReader());
            break;
        default:
            m_FileReader.reset(new CSmplBitstreamReader());
            break;
        }
    }

	printf("============Init 1\n");

    m_memType = pParams->memType;
    m_nMaxFps = pParams->nMaxFPS;
    m_nFrames = pParams->nFrames ? pParams->nFrames : MFX_INFINITE;

    if (MFX_CODEC_CAPTURE != pParams->videoType)
    {
        sts = m_FileReader->Init(pParams->strSrcFile);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }
printf("============Init 2\n");
    mfxInitParam initPar;
    mfxExtThreadsParam threadsPar;
    mfxExtBuffer* extBufs[1];
    mfxVersion version;     // real API version with which library is initialized

    MSDK_ZERO_MEMORY(initPar);
    MSDK_ZERO_MEMORY(threadsPar);

    // we set version to 1.0 and later we will query actual version of the library which will got leaded
    initPar.Version.Major = 1;
    initPar.Version.Minor = 0;

    initPar.GPUCopy = pParams->gpuCopy;

    init_ext_buffer(threadsPar);

    bool needInitExtPar = false;

    if (pParams->nThreadsNum) {
        threadsPar.NumThread = pParams->nThreadsNum;
        needInitExtPar = true;
    }
    if (pParams->SchedulingType) {
        threadsPar.SchedulingType = pParams->SchedulingType;
        needInitExtPar = true;
    }
    if (pParams->Priority) {
        threadsPar.Priority = pParams->Priority;
        needInitExtPar = true;
    }
    if (needInitExtPar) {
        extBufs[0] = (mfxExtBuffer*)&threadsPar;
        initPar.ExtParam = extBufs;
        initPar.NumExtParam = 1;
    }

    // Init session
    if (pParams->bUseHWLib) {
        // try searching on all display adapters
        initPar.Implementation = MFX_IMPL_HARDWARE_ANY;

        // if d3d11 surfaces are used ask the library to run acceleration through D3D11
        // feature may be unsupported due to OS or MSDK API version
        if (D3D11_MEMORY == pParams->memType)
            initPar.Implementation |= MFX_IMPL_VIA_D3D11;

        sts = m_mfxSession.InitEx(initPar);

        // MSDK API version may not support multiple adapters - then try initialize on the default
        if (MFX_ERR_NONE != sts) {
            initPar.Implementation = (initPar.Implementation & !MFX_IMPL_HARDWARE_ANY) | MFX_IMPL_HARDWARE;
            sts = m_mfxSession.InitEx(initPar);
        }
    } else {
        initPar.Implementation = MFX_IMPL_SOFTWARE;
        sts = m_mfxSession.InitEx(initPar);
    }

    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    sts = MFXQueryVersion(m_mfxSession , &version); // get real API version of the loaded library
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    if (pParams->bIsMVC && !CheckVersion(&version, MSDK_FEATURE_MVC)) {
        msdk_printf(MSDK_STRING("error: MVC is not supported in the %d.%d API version\n"),
            version.Major, version.Minor);
        return MFX_ERR_UNSUPPORTED;

    }
    if ((pParams->videoType == MFX_CODEC_JPEG) && !CheckVersion(&version, MSDK_FEATURE_JPEG_DECODE)) {
        msdk_printf(MSDK_STRING("error: Jpeg is not supported in the %d.%d API version\n"),
            version.Major, version.Minor);
        return MFX_ERR_UNSUPPORTED;
    }
    if (pParams->bLowLat && !CheckVersion(&version, MSDK_FEATURE_LOW_LATENCY)) {
        msdk_printf(MSDK_STRING("error: Low Latency mode is not supported in the %d.%d API version\n"),
            version.Major, version.Minor);
        return MFX_ERR_UNSUPPORTED;
    }

    // create decoder
    m_pmfxDEC = new MFXVideoDECODE(m_mfxSession);
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_MEMORY_ALLOC);

    // set video type in parameters
    m_mfxVideoParams.mfx.CodecId = pParams->videoType;

    // prepare bit stream
    if (MFX_CODEC_CAPTURE != pParams->videoType)
    {
        sts = InitMfxBitstream(&m_mfxBS, 1024 * 1024);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }

    if (CheckVersion(&version, MSDK_FEATURE_PLUGIN_API)) {
        /* Here we actually define the following codec initialization scheme:
        *  1. If plugin path or guid is specified: we load user-defined plugin (example: VP8 sample decoder plugin)
        *  2. If plugin path not specified:
        *    2.a) we check if codec is distributed as a mediasdk plugin and load it if yes
        *    2.b) if codec is not in the list of mediasdk plugins, we assume, that it is supported inside mediasdk library
        */
        // Load user plug-in, should go after CreateAllocator function (when all callbacks were initialized)
        if (pParams->pluginParams.type == MFX_PLUGINLOAD_TYPE_FILE && strlen(pParams->pluginParams.strPluginPath))
        {
            m_pUserModule.reset(new MFXVideoUSER(m_mfxSession));
            if (pParams->videoType == CODEC_VP8 || pParams->videoType == MFX_CODEC_HEVC)
            {
                m_pPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, m_mfxSession, pParams->pluginParams.pluginGuid, 1, pParams->pluginParams.strPluginPath, (mfxU32)strlen(pParams->pluginParams.strPluginPath)));
            }
            if (m_pPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
        }
        else
        {
            if (AreGuidsEqual(pParams->pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
            {
                mfxIMPL impl = pParams->bUseHWLib ? MFX_IMPL_HARDWARE : MFX_IMPL_SOFTWARE;
                pParams->pluginParams.pluginGuid = msdkGetPluginUID(impl, MSDK_VDECODE, pParams->videoType);
            }
            if (!AreGuidsEqual(pParams->pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
            {
                m_pPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, m_mfxSession, pParams->pluginParams.pluginGuid, 1));
                if (m_pPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
            }
        }
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }
printf("============Init 3\n");
    // Populate parameters. Involves DecodeHeader call
    sts = InitMfxParams(pParams);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
printf("============Init 4\n");
    m_eWorkMode = pParams->mode;
    if (m_eWorkMode == MODE_FILE_DUMP) {
        // prepare YUV file writer
        sts = m_FileWriter.Init(pParams->strDstFile, pParams->numViews);
    } else if ((m_eWorkMode != MODE_PERFORMANCE) && (m_eWorkMode != MODE_RENDERING)) {
        msdk_printf(MSDK_STRING("error: unsupported work mode\n"));
        sts = MFX_ERR_UNSUPPORTED;
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // create device and allocator
    sts = CreateAllocator();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // in case of HW accelerated decode frames must be allocated prior to decoder initialization
    sts = AllocFrames();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
printf("============Init 5\n");
    sts = m_pmfxDEC->Init(&m_mfxVideoParams);
    if (MFX_WRN_PARTIAL_ACCELERATION == sts)
    {
        msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
printf("============Init 6\n");
    if (m_eWorkMode == MODE_RENDERING)
    {
        sts = CreateRenderingWindow(pParams, m_bIsMVC && (m_memType == D3D9_MEMORY));
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }
printf("============Init 7\n");
    return sts;
}
#endif

void CDecodingPipeline::Close()
{
#if D3D_SURFACES_SUPPORT
    if (NULL != m_pS3DControl)
    {
        m_pS3DControl->SwitchTo2D(NULL);
        MSDK_SAFE_DELETE(m_pS3DControl);
    }
#endif
    WipeMfxBitstream(&m_mfxBS);
    MSDK_SAFE_DELETE(m_pmfxDEC);

    DeleteFrames();

    if (m_bIsExtBuffers)
    {
        DeallocateExtMVCBuffers();
        DeleteExtBuffers();
    }

    m_pPlugin.reset();
    m_mfxSession.Close();
    m_FileWriter.Close();
    if (m_FileReader.get())
        m_FileReader->Close();

    // allocator if used as external for MediaSDK must be deleted after decoder
    DeleteAllocator();

    return;
}

#if D3D_SURFACES_SUPPORT
bool operator < (const IGFX_DISPLAY_MODE &l, const IGFX_DISPLAY_MODE& r)
{
    if (r.ulResWidth >= 0xFFFF || r.ulResHeight >= 0xFFFF || r.ulRefreshRate >= 0xFFFF)
        return false;

         if (l.ulResWidth < r.ulResWidth) return true;
    else if (l.ulResHeight < r.ulResHeight) return true;
    else if (l.ulRefreshRate < r.ulRefreshRate) return true;

    return false;
}
#endif

mfxStatus CDecodingPipeline::CreateRenderingWindow(sInputParams_dec *pParams, bool try_s3d)
{
    mfxStatus sts = MFX_ERR_NONE;

#if D3D_SURFACES_SUPPORT
    if (try_s3d) {

        m_pS3DControl = CreateIGFXS3DControl();
        MSDK_CHECK_POINTER(m_pS3DControl, MFX_ERR_DEVICE_FAILED);

        // check if s3d supported and get a list of supported display modes
        IGFX_S3DCAPS caps;
        MSDK_ZERO_MEMORY(caps);
        HRESULT hr = m_pS3DControl->GetS3DCaps(&caps);
        if (FAILED(hr) || 0 >= caps.ulNumEntries) {
            MSDK_SAFE_DELETE(m_pS3DControl);
            return MFX_ERR_DEVICE_FAILED;
        }

        // switch to 3D mode
        ULONG max = 0;
        MSDK_CHECK_POINTER(caps.S3DSupportedModes, MFX_ERR_NOT_INITIALIZED);
        for (ULONG i = 0; i < caps.ulNumEntries; i++) {
            if (caps.S3DSupportedModes[max] < caps.S3DSupportedModes[i])
                max = i;
        }

        if (0 == pParams->nWallCell) {
            hr = m_pS3DControl->SwitchTo3D(&caps.S3DSupportedModes[max]);
            if (FAILED(hr)) {
                MSDK_SAFE_DELETE(m_pS3DControl);
                return MFX_ERR_DEVICE_FAILED;
            }
        }
    }
    sWindowParams windowParams;

    windowParams.lpWindowName = pParams->bWallNoTitle ? NULL : MSDK_STRING("sample_decode");
    windowParams.nx           = pParams->nWallW;
    windowParams.ny           = pParams->nWallH;
    windowParams.nWidth       = m_mfxVideoParams.mfx.FrameInfo.Width;
    windowParams.nHeight      = m_mfxVideoParams.mfx.FrameInfo.Height;
    windowParams.ncell        = pParams->nWallCell;
    windowParams.nAdapter     = pParams->nWallMonitor;

    windowParams.lpClassName  = MSDK_STRING("Render Window Class");
    windowParams.dwStyle      = WS_OVERLAPPEDWINDOW;
    windowParams.hWndParent   = NULL;
    windowParams.hMenu        = NULL;
    windowParams.hInstance    = GetModuleHandle(NULL);
    windowParams.lpParam      = NULL;
    windowParams.bFullScreen  = FALSE;

    sts = m_d3dRender.Init(windowParams);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    //setting videowall flag
    m_bIsVideoWall = 0 != windowParams.nx;
    //setting timeout value
    if (m_bIsVideoWall && (pParams->nWallTimeout>0)) m_nTimeout = pParams->nWallTimeout;
#endif
    return sts;
}
#if 1
mfxStatus CDecodingPipeline::InitMfxParams(sInputParams_dec *pParams)
{
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_NULL_PTR);
    mfxStatus sts = MFX_ERR_NONE;
    mfxU32 &numViews = pParams->numViews;
    mfxBitstream *pbitstream = &(pParams->pBitStrameBuff->Bitstream);

    // trying to find PicStruct information in AVI headers
    if ( m_mfxVideoParams.mfx.CodecId == MFX_CODEC_JPEG )
        MJPEG_AVI_ParsePicStruct(pbitstream);

    // parse bit stream and fill mfx params
    sts = m_pmfxDEC->DecodeHeader(pbitstream, &m_mfxVideoParams);
    if (m_pPlugin.get() && pParams->videoType == CODEC_VP8 && !sts) {
        // force set format to nv12 as the vp8 plugin uses yv12
        m_mfxVideoParams.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
    }
    if (MFX_ERR_MORE_DATA == sts)
    {
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }
    else
    {
        // Enter MVC mode
        if (m_bIsMVC)
        {
            // Check for attached external parameters - if we have them already,
            // we don't need to attach them again
            if (NULL != m_mfxVideoParams.ExtParam)
                goto exit;

            // allocate and attach external parameters for MVC decoder
            sts = AllocateExtBuffer<mfxExtMVCSeqDesc>();
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

            AttachExtParam();
            sts = m_pmfxDEC->DecodeHeader(pbitstream, &m_mfxVideoParams);

            if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)
            {
                sts = AllocateExtMVCBuffers();
                SetExtBuffersFlag();

                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
                MSDK_CHECK_POINTER(m_mfxVideoParams.ExtParam, MFX_ERR_MEMORY_ALLOC);
            }
	     
        }

        // if input is interlaced JPEG stream
        if ( pbitstream->PicStruct == MFX_PICSTRUCT_FIELD_TFF || pbitstream->PicStruct == MFX_PICSTRUCT_FIELD_BFF)
        {
            m_mfxVideoParams.mfx.FrameInfo.CropH *= 2;
            m_mfxVideoParams.mfx.FrameInfo.Height = MSDK_ALIGN16(m_mfxVideoParams.mfx.FrameInfo.CropH);
            m_mfxVideoParams.mfx.FrameInfo.PicStruct = pbitstream->PicStruct;
        }

        switch(pParams->nRotation)
        {
	        case 0:
	            m_mfxVideoParams.mfx.Rotation = MFX_ROTATION_0;
	            break;
	        case 90:
	            m_mfxVideoParams.mfx.Rotation = MFX_ROTATION_90;
	            break;
	        case 180:
	            m_mfxVideoParams.mfx.Rotation = MFX_ROTATION_180;
	            break;
	        case 270:
	            m_mfxVideoParams.mfx.Rotation = MFX_ROTATION_270;
	            break;
	        default:
	            return MFX_ERR_UNSUPPORTED;
        }
    }
exit:
    // check DecodeHeader status
    if (MFX_WRN_PARTIAL_ACCELERATION == sts)
    {
        msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // If MVC mode we need to detect number of views in stream
    if (m_bIsMVC)
    {
        mfxExtMVCSeqDesc* pSequenceBuffer;
        pSequenceBuffer = (mfxExtMVCSeqDesc*) GetExtBuffer(m_mfxVideoParams.ExtParam, m_mfxVideoParams.NumExtParam, MFX_EXTBUFF_MVC_SEQ_DESC);
        MSDK_CHECK_POINTER(pSequenceBuffer, MFX_ERR_INVALID_VIDEO_PARAM);

        mfxU32 i = 0;
        numViews = 0;
        for (i = 0; i < pSequenceBuffer->NumView; ++i)
        {
            /* Some MVC streams can contain different information about
               number of views and view IDs, e.x. numVews = 2
               and ViewId[0, 1] = 0, 2 instead of ViewId[0, 1] = 0, 1.
               numViews should be equal (max(ViewId[i]) + 1)
               to prevent crashes during output files writing */
            if (pSequenceBuffer->View[i].ViewId >= numViews)
                numViews = pSequenceBuffer->View[i].ViewId + 1;
        }
    }
    else
    {
        numViews = 1;
    }

    // specify memory type
    m_mfxVideoParams.IOPattern = (mfxU16)(m_memType != SYSTEM_MEMORY ? MFX_IOPATTERN_OUT_VIDEO_MEMORY : MFX_IOPATTERN_OUT_SYSTEM_MEMORY);

    m_mfxVideoParams.AsyncDepth = pParams->nAsyncDepth;

    m_mfxVideoParams.mfx.FrameInfo.Width = 1920;
    m_mfxVideoParams.mfx.FrameInfo.Height = 1088;
    m_mfxVideoParams.mfx.FrameInfo.CropW = 1920;
    m_mfxVideoParams.mfx.FrameInfo.CropH = 1080;


	msdk_printf(_T("m_mfxVideoParams:\n"));
	msdk_printf(_T("AsyncDepth:%d\n")
		_T("GopPicSize:%d\n")
		_T("CodecId:%d\n")
		_T("TargetUsage:%d\n")
		_T("TargetKbps:%d\n")
		_T("RateControlMethod:%d\n")
		_T("NumSlice:%d\n")
		_T("IOPattern:%d\n")
		_T("PicStruct:%d\n")
		_T("Width:%d\n")
		_T("Height:%d\n")
		_T("CropX:%d\n")
		_T("CropY:%d\n")
		_T("CropW:%d\n")
		_T("CropH:%d\n")
		_T("FourCC:%d\n")
		_T("ChromaFormat:%d\n")
		,m_mfxVideoParams.AsyncDepth,m_mfxVideoParams.mfx.GopPicSize,m_mfxVideoParams.mfx.CodecId,m_mfxVideoParams.mfx.TargetUsage,
		m_mfxVideoParams.mfx.TargetKbps,m_mfxVideoParams.mfx.RateControlMethod,m_mfxVideoParams.mfx.NumSlice,
		m_mfxVideoParams.IOPattern,m_mfxVideoParams.mfx.FrameInfo.PicStruct,m_mfxVideoParams.mfx.FrameInfo.Width,
		m_mfxVideoParams.mfx.FrameInfo.Height,m_mfxVideoParams.mfx.FrameInfo.CropX,m_mfxVideoParams.mfx.FrameInfo.CropY,
		m_mfxVideoParams.mfx.FrameInfo.CropW,m_mfxVideoParams.mfx.FrameInfo.CropH,m_mfxVideoParams.mfx.FrameInfo.FourCC,
		m_mfxVideoParams.mfx.FrameInfo.ChromaFormat
		);

    return MFX_ERR_NONE;
}
#else
mfxStatus CDecodingPipeline::InitMfxParams(sInputParams_dec *pParams)
{
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_NULL_PTR);
    mfxStatus sts = MFX_ERR_NONE;
    mfxU32 &numViews = pParams->numViews;

    // try to find a sequence header in the stream
    // if header is not found this function exits with error (e.g. if device was lost and there's no header in the remaining stream)
    if (MFX_CODEC_CAPTURE == pParams->videoType)
    {
        m_mfxVideoParams.mfx.CodecId = MFX_CODEC_CAPTURE;
        m_mfxVideoParams.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
        m_mfxVideoParams.mfx.FrameInfo.Width = MSDK_ALIGN32(pParams->width);
        m_mfxVideoParams.mfx.FrameInfo.Height = MSDK_ALIGN32(pParams->height);
        m_mfxVideoParams.mfx.FrameInfo.CropW = pParams->width;
        m_mfxVideoParams.mfx.FrameInfo.CropH = pParams->height;
        m_mfxVideoParams.mfx.FrameInfo.FourCC = pParams->fourcc;
        if (!m_mfxVideoParams.mfx.FrameInfo.FourCC)
            m_mfxVideoParams.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
        if (!m_mfxVideoParams.mfx.FrameInfo.ChromaFormat)
        {
            if (MFX_FOURCC_NV12 == m_mfxVideoParams.mfx.FrameInfo.FourCC)
                m_mfxVideoParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
            else if (MFX_FOURCC_RGB4 == m_mfxVideoParams.mfx.FrameInfo.FourCC)
                m_mfxVideoParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV444;
        }
    }
printf("============InitMfxParams 1\n");
    for (; MFX_CODEC_CAPTURE != pParams->videoType;)
    {
        // trying to find PicStruct information in AVI headers
        if ( m_mfxVideoParams.mfx.CodecId == MFX_CODEC_JPEG )
            MJPEG_AVI_ParsePicStruct(&m_mfxBS);
printf("============InitMfxParams 2\n");
        // parse bit stream and fill mfx params
        sts = m_pmfxDEC->DecodeHeader(&m_mfxBS, &m_mfxVideoParams);
        if (m_pPlugin.get() && pParams->videoType == CODEC_VP8 && !sts) {
            // force set format to nv12 as the vp8 plugin uses yv12
            m_mfxVideoParams.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
        }
		printf("============InitMfxParams 3\n");
        if (MFX_ERR_MORE_DATA == sts)
        {
        	printf("============InitMfxParams 8\n");
            if (m_mfxBS.MaxLength == m_mfxBS.DataLength)
            {
                sts = ExtendMfxBitstream(&m_mfxBS, m_mfxBS.MaxLength * 2);
                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
            }
			printf("============InitMfxParams 9\n");
            // read a portion of data
          //  printf("iiiiiiiiiiiGetNetData\n");
            sts = m_FileReader->ReadNextFrame(&m_mfxBS);
          //  printf("iiiiiiiiiiiGetNetData,sts=%d\n");
            if (MFX_ERR_MORE_DATA == sts &&
                !(m_mfxBS.DataFlag & MFX_BITSTREAM_EOS))
            {
                m_mfxBS.DataFlag |= MFX_BITSTREAM_EOS;
                sts = MFX_ERR_NONE;
            }
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

            continue;
        }
        else
        {
            // Enter MVC mode
            if (m_bIsMVC)
            {
                // Check for attached external parameters - if we have them already,
                // we don't need to attach them again
                if (NULL != m_mfxVideoParams.ExtParam)
                    break;

                // allocate and attach external parameters for MVC decoder
                sts = AllocateExtBuffer<mfxExtMVCSeqDesc>();
                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

                AttachExtParam();
                sts = m_pmfxDEC->DecodeHeader(&m_mfxBS, &m_mfxVideoParams);

                if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)
                {
                    sts = AllocateExtMVCBuffers();
                    SetExtBuffersFlag();

                    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
                    MSDK_CHECK_POINTER(m_mfxVideoParams.ExtParam, MFX_ERR_MEMORY_ALLOC);
                    continue;
                }
            }
printf("============InitMfxParams 4\n");
            // if input is interlaced JPEG stream
            if ( m_mfxBS.PicStruct == MFX_PICSTRUCT_FIELD_TFF || m_mfxBS.PicStruct == MFX_PICSTRUCT_FIELD_BFF)
            {
                m_mfxVideoParams.mfx.FrameInfo.CropH *= 2;
                m_mfxVideoParams.mfx.FrameInfo.Height = MSDK_ALIGN16(m_mfxVideoParams.mfx.FrameInfo.CropH);
                m_mfxVideoParams.mfx.FrameInfo.PicStruct = m_mfxBS.PicStruct;
            }

            switch(pParams->nRotation)
            {
            case 0:
                m_mfxVideoParams.mfx.Rotation = MFX_ROTATION_0;
                break;
            case 90:
                m_mfxVideoParams.mfx.Rotation = MFX_ROTATION_90;
                break;
            case 180:
                m_mfxVideoParams.mfx.Rotation = MFX_ROTATION_180;
                break;
            case 270:
                m_mfxVideoParams.mfx.Rotation = MFX_ROTATION_270;
                break;
            default:
                return MFX_ERR_UNSUPPORTED;
            }
printf("============InitMfxParams 5\n");
            break;
        }
    }

    // check DecodeHeader status
    if (MFX_WRN_PARTIAL_ACCELERATION == sts)
    {
        msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
printf("============InitMfxParams 6\n");
    // If MVC mode we need to detect number of views in stream
    if (m_bIsMVC)
    {
        mfxExtMVCSeqDesc* pSequenceBuffer;
        pSequenceBuffer = (mfxExtMVCSeqDesc*) GetExtBuffer(m_mfxVideoParams.ExtParam, m_mfxVideoParams.NumExtParam, MFX_EXTBUFF_MVC_SEQ_DESC);
        MSDK_CHECK_POINTER(pSequenceBuffer, MFX_ERR_INVALID_VIDEO_PARAM);

        mfxU32 i = 0;
        numViews = 0;
        for (i = 0; i < pSequenceBuffer->NumView; ++i)
        {
            /* Some MVC streams can contain different information about
               number of views and view IDs, e.x. numVews = 2
               and ViewId[0, 1] = 0, 2 instead of ViewId[0, 1] = 0, 1.
               numViews should be equal (max(ViewId[i]) + 1)
               to prevent crashes during output files writing */
            if (pSequenceBuffer->View[i].ViewId >= numViews)
                numViews = pSequenceBuffer->View[i].ViewId + 1;
        }
    }
    else
    {
        numViews = 1;
    }
printf("============InitMfxParams 7\n");
    // specify memory type
    m_mfxVideoParams.IOPattern = (mfxU16)(m_memType != SYSTEM_MEMORY ? MFX_IOPATTERN_OUT_VIDEO_MEMORY : MFX_IOPATTERN_OUT_SYSTEM_MEMORY);

    m_mfxVideoParams.AsyncDepth = pParams->nAsyncDepth;

    return MFX_ERR_NONE;
}
#endif
mfxStatus CDecodingPipeline::InitMfxParams_reach(sInputParams_reach *pInParams)
{
    m_mfxVideoParams.mfx.CodecId                 	= pInParams->CodecId;
    //m_mfxVideoParams.mfx.TargetUsage             	= pInParams->nTargetUsage; // trade-off between quality and speed
    //m_mfxVideoParams.mfx.MaxKbps              	= 8000; // in Kbps
    //m_mfxVideoParams.mfx.TargetKbps              = pInParams->nBitRate; // in Kbps
    //m_mfxVideoParams.mfx.RateControlMethod       = pInParams->nRateControlMethod;
#if 0
    if (m_mfxVideoParams.mfx.RateControlMethod == MFX_RATECONTROL_CQP)
    {
        m_mfxVideoParams.mfx.QPI = pInParams->nQPI;
        m_mfxVideoParams.mfx.QPP = pInParams->nQPP;
        m_mfxVideoParams.mfx.QPB = pInParams->nQPB;
    }
#endif
    m_mfxVideoParams.mfx.NumSlice = pInParams->nNumSlice;
    ConvertFrameRate(pInParams->dFrameRate, &m_mfxVideoParams.mfx.FrameInfo.FrameRateExtN, &m_mfxVideoParams.mfx.FrameInfo.FrameRateExtD);
    m_mfxVideoParams.mfx.EncodedOrder            = 0; // binary flag, 0 signals encoder to take frames in display order

    // specify memory type
    if (D3D9_MEMORY == pInParams->memType || D3D11_MEMORY == pInParams->memType)
    {
        m_mfxVideoParams.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY;
    }
    else
    {
        m_mfxVideoParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
    }

    // frame info parameters
    m_mfxVideoParams.mfx.FrameInfo.FourCC       = MFX_FOURCC_NV12;
    m_mfxVideoParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    m_mfxVideoParams.mfx.FrameInfo.PicStruct    = pInParams->nPicStruct;

    // set frame size and crops
    // width must be a multiple of 16
    // height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
    m_mfxVideoParams.mfx.FrameInfo.Width  = MSDK_ALIGN16(pInParams->nDstWidth);
    m_mfxVideoParams.mfx.FrameInfo.Height = (MFX_PICSTRUCT_PROGRESSIVE == m_mfxVideoParams.mfx.FrameInfo.PicStruct)?
        MSDK_ALIGN16(pInParams->nDstHeight) : MSDK_ALIGN32(pInParams->nDstHeight);

    m_mfxVideoParams.mfx.FrameInfo.CropX = 0;
    m_mfxVideoParams.mfx.FrameInfo.CropY = 0;
    m_mfxVideoParams.mfx.FrameInfo.CropW = pInParams->nDstWidth;
    m_mfxVideoParams.mfx.FrameInfo.CropH = pInParams->nDstHeight;

	//AVC Profile
	if (pInParams->nEncodeProfie==ENCODER_Baseline_Profile)
	{
		m_mfxVideoParams.mfx.CodecProfile = MFX_PROFILE_AVC_BASELINE;
	}
	else if (pInParams->nEncodeProfie==ENCODER_Main_Profile)
	{
		m_mfxVideoParams.mfx.CodecProfile = MFX_PROFILE_AVC_MAIN;
	}
	else if (pInParams->nEncodeProfie==ENCODER_High_Profile)
	{
		m_mfxVideoParams.mfx.CodecProfile = MFX_PROFILE_AVC_HIGH;
	}
	else
	{
		m_mfxVideoParams.mfx.CodecProfile = MFX_PROFILE_AVC_HIGH;
	}
	//AVC Profile Level
	if (pInParams->nProfieLevel==ENCODER_Profile_Levels1)
	{
		m_mfxVideoParams.mfx.CodecLevel = MFX_LEVEL_AVC_1;
	}
	else if(pInParams->nProfieLevel==ENCODER_Profile_Levels2)
	{
		m_mfxVideoParams.mfx.CodecLevel = MFX_LEVEL_AVC_2;
	}
	else if(pInParams->nProfieLevel==ENCODER_Profile_Levels3)
	{
		m_mfxVideoParams.mfx.CodecLevel = MFX_LEVEL_AVC_3;
	}
	else if(pInParams->nProfieLevel==ENCODER_Profile_Levels4)
	{
		m_mfxVideoParams.mfx.CodecLevel = MFX_LEVEL_AVC_4;
	}
	else
	{
		m_mfxVideoParams.mfx.CodecLevel = MFX_LEVEL_AVC_4;
	}

    m_mfxVideoParams.AsyncDepth = pInParams->nAsyncDepth;
	m_mfxVideoParams.mfx.GopPicSize=pInParams->nGopszie;
	//Ð´ËÀ ²»±àbÖ¡
	//m_mfxVideoParams.mfx.GopRefDist = 1;
	//m_mfxVideoParams.mfx.NumRefFrame = 1;

	msdk_printf(_T("m_mfxVideoParams:\n"));
	msdk_printf(_T("AsyncDepth:%d\n")
		_T("GopPicSize:%d\n")	
		_T("CodecId:%d\n")
		_T("TargetUsage:%d\n")
		_T("TargetKbps:%d\n")
		_T("RateControlMethod:%d\n")
		_T("NumSlice:%d\n")
		_T("IOPattern:%d\n")
		_T("PicStruct:%d\n")
		_T("Width:%d\n")
		_T("Height:%d\n")
		_T("CropX:%d\n")
		_T("CropY:%d\n")
		_T("CropW:%d\n")
		_T("CropH:%d\n")
		,m_mfxVideoParams.AsyncDepth,m_mfxVideoParams.mfx.GopPicSize,m_mfxVideoParams.mfx.CodecId,m_mfxVideoParams.mfx.TargetUsage,
		m_mfxVideoParams.mfx.TargetKbps,m_mfxVideoParams.mfx.RateControlMethod,m_mfxVideoParams.mfx.NumSlice,
		m_mfxVideoParams.IOPattern,m_mfxVideoParams.mfx.FrameInfo.PicStruct,m_mfxVideoParams.mfx.FrameInfo.Width,
		m_mfxVideoParams.mfx.FrameInfo.Height,m_mfxVideoParams.mfx.FrameInfo.CropX,m_mfxVideoParams.mfx.FrameInfo.CropY,
		m_mfxVideoParams.mfx.FrameInfo.CropW,m_mfxVideoParams.mfx.FrameInfo.CropH
		);


    return MFX_ERR_NONE;
}

mfxStatus CDecodingPipeline::CreateHWDevice()
{
#if D3D_SURFACES_SUPPORT
    mfxStatus sts = MFX_ERR_NONE;

    HWND window = NULL;
    bool render = (m_eWorkMode == MODE_RENDERING);

    if (render) {
        window = (D3D11_MEMORY == m_memType) ? NULL : m_d3dRender.GetWindowHandle();
    }

#if MFX_D3D11_SUPPORT
    if (D3D11_MEMORY == m_memType)
        m_hwdev = new CD3D11Device();
    else
#endif // #if MFX_D3D11_SUPPORT
        m_hwdev = new CD3D9Device();

    if (NULL == m_hwdev)
        return MFX_ERR_MEMORY_ALLOC;

    if (render && m_bIsMVC && m_memType == D3D9_MEMORY) {
        sts = m_hwdev->SetHandle((mfxHandleType)MFX_HANDLE_GFXS3DCONTROL, m_pS3DControl);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }
    sts = m_hwdev->Init(
        window,
        render ? (m_bIsMVC ? 2 : 1) : 0,
        MSDKAdapter::GetNumber(m_mfxSession));
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    if (render)
        m_d3dRender.SetHWDevice(m_hwdev);
#elif LIBVA_SUPPORT
    mfxStatus sts = MFX_ERR_NONE;
    m_hwdev = CreateVAAPIDevice();

    if (NULL == m_hwdev) {
        return MFX_ERR_MEMORY_ALLOC;
    }

    sts = m_hwdev->Init(NULL, (m_eWorkMode == MODE_RENDERING) ? 1 : 0, MSDKAdapter::GetNumber(m_mfxSession));
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
#endif
    return MFX_ERR_NONE;
}

mfxStatus CDecodingPipeline::ResetDevice()
{
    return m_hwdev->Reset();
}

mfxStatus CDecodingPipeline::AllocFrames()
{
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_NULL_PTR);

    mfxStatus sts = MFX_ERR_NONE;

    mfxFrameAllocRequest Request;

    mfxU16 nSurfNum = 0; // number of surfaces for decoder

    MSDK_ZERO_MEMORY(Request);

    sts = m_pmfxDEC->Query(&m_mfxVideoParams, &m_mfxVideoParams);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    // calculate number of surfaces required for decoder
    sts = m_pmfxDEC->QueryIOSurf(&m_mfxVideoParams, &Request);
    if (MFX_WRN_PARTIAL_ACCELERATION == sts)
    {
        msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    mfxIMPL impl = 0;
    sts = m_mfxSession.QueryIMPL(&impl);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    if ((Request.NumFrameSuggested < m_mfxVideoParams.AsyncDepth) &&
        (impl & MFX_IMPL_HARDWARE_ANY))
        return MFX_ERR_MEMORY_ALLOC;

    nSurfNum = MSDK_MAX(Request.NumFrameSuggested, 1);

    // prepare allocation request
    Request.NumFrameSuggested = Request.NumFrameMin = nSurfNum;

    // alloc frames for decoder
    sts = m_pMFXAllocator->Alloc(m_pMFXAllocator->pthis, &Request, &m_mfxResponse);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // prepare mfxFrameSurface1 array for decoder
    nSurfNum = m_mfxResponse.NumFrameActual;

    //sts = AllocBuffers(nSurfNum);
    sts = AllocBuffers_FrameBuff(nSurfNum);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
#if 0
    printf("==========Width:%d,Height:%d,cropX:%d,cropY:%d,cropWidth:%d,cropHeight:%d,PicStruct:%d,ChromaFormat:%d,FrameId:%d,FourCC:%d,BitDepthLuma:%d,BitDepthChroma:%d,Shift:%d\n",
		Request.Info.Width,Request.Info.Height,Request.Info.CropX,Request.Info.CropY,Request.Info.CropW,Request.Info.CropH,
		Request.Info.PicStruct,Request.Info.ChromaFormat,Request.Info.FrameId,Request.Info.FourCC,Request.Info.BitDepthLuma,Request.Info.BitDepthChroma,Request.Info.Shift);

    for (int i = 0; i < nSurfNum; i++)
    {
        // initating each frame:
        MSDK_MEMCPY_VAR(m_pSurfaces[i].frame.Info, &(Request.Info), sizeof(mfxFrameInfo));
        if (m_bExternalAlloc)
        {
            m_pSurfaces[i].frame.Data.MemId = m_mfxResponse.mids[i];
        }
        else
        {
            sts = m_pMFXAllocator->Lock(m_pMFXAllocator->pthis, m_mfxResponse.mids[i], &(m_pSurfaces[i].frame.Data));
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        }
    }
#endif
    return MFX_ERR_NONE;
}

mfxStatus CDecodingPipeline::CreateAllocator()
{
    mfxStatus sts = MFX_ERR_NONE;

    if (m_memType != SYSTEM_MEMORY)
    {
#if D3D_SURFACES_SUPPORT
        sts = CreateHWDevice();
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        // provide device manager to MediaSDK
        mfxHDL hdl = NULL;
        mfxHandleType hdl_t =
#if MFX_D3D11_SUPPORT
            D3D11_MEMORY == m_memType ? MFX_HANDLE_D3D11_DEVICE :
#endif // #if MFX_D3D11_SUPPORT
            MFX_HANDLE_D3D9_DEVICE_MANAGER;

        sts = m_hwdev->GetHandle(hdl_t, &hdl);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        sts = m_mfxSession.SetHandle(hdl_t, hdl);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        // create D3D allocator
#if MFX_D3D11_SUPPORT
        if (D3D11_MEMORY == m_memType)
        {
            m_pMFXAllocator = new D3D11FrameAllocator;
            MSDK_CHECK_POINTER(m_pMFXAllocator, MFX_ERR_MEMORY_ALLOC);

            D3D11AllocatorParams *pd3d11AllocParams = new D3D11AllocatorParams;
            MSDK_CHECK_POINTER(pd3d11AllocParams, MFX_ERR_MEMORY_ALLOC);
            pd3d11AllocParams->pDevice = reinterpret_cast<ID3D11Device *>(hdl);

            m_pmfxAllocatorParams = pd3d11AllocParams;
        }
        else
#endif // #if MFX_D3D11_SUPPORT
        {
            m_pMFXAllocator = new D3DFrameAllocator;
            MSDK_CHECK_POINTER(m_pMFXAllocator, MFX_ERR_MEMORY_ALLOC);

            D3DAllocatorParams *pd3dAllocParams = new D3DAllocatorParams;
            MSDK_CHECK_POINTER(pd3dAllocParams, MFX_ERR_MEMORY_ALLOC);
            pd3dAllocParams->pManager = reinterpret_cast<IDirect3DDeviceManager9 *>(hdl);

            m_pmfxAllocatorParams = pd3dAllocParams;
        }

        /* In case of video memory we must provide MediaSDK with external allocator
        thus we demonstrate "external allocator" usage model.
        Call SetAllocator to pass allocator to mediasdk */
        sts = m_mfxSession.SetFrameAllocator(m_pMFXAllocator);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        m_bExternalAlloc = true;
#elif LIBVA_SUPPORT
        sts = CreateHWDevice();
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        /* It's possible to skip failed result here and switch to SW implementation,
           but we don't process this way */

        // provide device manager to MediaSDK
        VADisplay va_dpy = NULL;
        sts = m_hwdev->GetHandle(MFX_HANDLE_VA_DISPLAY, (mfxHDL *)&va_dpy);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        sts = m_mfxSession.SetHandle(MFX_HANDLE_VA_DISPLAY, va_dpy);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);


        // create VAAPI allocator
        m_pMFXAllocator = new vaapiFrameAllocator;
        MSDK_CHECK_POINTER(m_pMFXAllocator, MFX_ERR_MEMORY_ALLOC);
        vaapiAllocatorParams *p_vaapiAllocParams = new vaapiAllocatorParams;
        MSDK_CHECK_POINTER(p_vaapiAllocParams, MFX_ERR_MEMORY_ALLOC);

        p_vaapiAllocParams->m_dpy = va_dpy;
        m_pmfxAllocatorParams = p_vaapiAllocParams;

        /* In case of video memory we must provide MediaSDK with external allocator
        thus we demonstrate "external allocator" usage model.
        Call SetAllocator to pass allocator to mediasdk */
        sts = m_mfxSession.SetFrameAllocator(m_pMFXAllocator);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        m_bExternalAlloc = true;
#endif
    }
    else
    {
#ifdef LIBVA_SUPPORT
        //in case of system memory allocator we also have to pass MFX_HANDLE_VA_DISPLAY to HW library
        mfxIMPL impl;
        m_mfxSession.QueryIMPL(&impl);

        if(MFX_IMPL_HARDWARE == MFX_IMPL_BASETYPE(impl))
        {
            sts = CreateHWDevice();
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

            // provide device manager to MediaSDK
            VADisplay va_dpy = NULL;
            sts = m_hwdev->GetHandle(MFX_HANDLE_VA_DISPLAY, (mfxHDL *)&va_dpy);
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
            sts = m_mfxSession.SetHandle(MFX_HANDLE_VA_DISPLAY, va_dpy);
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        }
#endif
        // create system memory allocator
        m_pMFXAllocator = new SysMemFrameAllocator;
        MSDK_CHECK_POINTER(m_pMFXAllocator, MFX_ERR_MEMORY_ALLOC);

        /* In case of system memory we demonstrate "no external allocator" usage model.
        We don't call SetAllocator, MediaSDK uses internal allocator.
        We use system memory allocator simply as a memory manager for application*/
    }

    // initialize memory allocator
    sts = m_pMFXAllocator->Init(m_pmfxAllocatorParams);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    return MFX_ERR_NONE;
}

void CDecodingPipeline::DeleteFrames()
{
    FreeBuffers();

    m_pCurrentFreeSurface = NULL;
    MSDK_SAFE_FREE(m_pCurrentFreeOutputSurface);

    // delete frames
    if (m_pMFXAllocator)
    {
        m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_mfxResponse);
    }

    return;
}

void CDecodingPipeline::DeleteAllocator()
{
    // delete allocator
    MSDK_SAFE_DELETE(m_pMFXAllocator);
    MSDK_SAFE_DELETE(m_pmfxAllocatorParams);
    MSDK_SAFE_DELETE(m_hwdev);
}

void CDecodingPipeline::SetMultiView()
{
    m_FileWriter.SetMultiView();
    m_bIsMVC = true;
}

// function for allocating a specific external buffer
template <typename Buffer>
mfxStatus CDecodingPipeline::AllocateExtBuffer()
{
    std::auto_ptr<Buffer> pExtBuffer (new Buffer());
    if (!pExtBuffer.get())
        return MFX_ERR_MEMORY_ALLOC;

    init_ext_buffer(*pExtBuffer);

    m_ExtBuffers.push_back(reinterpret_cast<mfxExtBuffer*>(pExtBuffer.release()));

    return MFX_ERR_NONE;
}

void CDecodingPipeline::AttachExtParam()
{
    m_mfxVideoParams.ExtParam = reinterpret_cast<mfxExtBuffer**>(&m_ExtBuffers[0]);
    m_mfxVideoParams.NumExtParam = static_cast<mfxU16>(m_ExtBuffers.size());
}

void CDecodingPipeline::DeleteExtBuffers()
{
    for (std::vector<mfxExtBuffer *>::iterator it = m_ExtBuffers.begin(); it != m_ExtBuffers.end(); ++it)
        delete *it;
    m_ExtBuffers.clear();
}

mfxStatus CDecodingPipeline::AllocateExtMVCBuffers()
{
    mfxU32 i;

    mfxExtMVCSeqDesc* pExtMVCBuffer = (mfxExtMVCSeqDesc*) m_mfxVideoParams.ExtParam[0];
    MSDK_CHECK_POINTER(pExtMVCBuffer, MFX_ERR_MEMORY_ALLOC);

    pExtMVCBuffer->View = new mfxMVCViewDependency[pExtMVCBuffer->NumView];
    MSDK_CHECK_POINTER(pExtMVCBuffer->View, MFX_ERR_MEMORY_ALLOC);
    for (i = 0; i < pExtMVCBuffer->NumView; ++i)
    {
        MSDK_ZERO_MEMORY(pExtMVCBuffer->View[i]);
    }
    pExtMVCBuffer->NumViewAlloc = pExtMVCBuffer->NumView;

    pExtMVCBuffer->ViewId = new mfxU16[pExtMVCBuffer->NumViewId];
    MSDK_CHECK_POINTER(pExtMVCBuffer->ViewId, MFX_ERR_MEMORY_ALLOC);
    for (i = 0; i < pExtMVCBuffer->NumViewId; ++i)
    {
        MSDK_ZERO_MEMORY(pExtMVCBuffer->ViewId[i]);
    }
    pExtMVCBuffer->NumViewIdAlloc = pExtMVCBuffer->NumViewId;

    pExtMVCBuffer->OP = new mfxMVCOperationPoint[pExtMVCBuffer->NumOP];
    MSDK_CHECK_POINTER(pExtMVCBuffer->OP, MFX_ERR_MEMORY_ALLOC);
    for (i = 0; i < pExtMVCBuffer->NumOP; ++i)
    {
        MSDK_ZERO_MEMORY(pExtMVCBuffer->OP[i]);
    }
    pExtMVCBuffer->NumOPAlloc = pExtMVCBuffer->NumOP;

    return MFX_ERR_NONE;
}

void CDecodingPipeline::DeallocateExtMVCBuffers()
{
    mfxExtMVCSeqDesc* pExtMVCBuffer = (mfxExtMVCSeqDesc*) m_mfxVideoParams.ExtParam[0];
    if (pExtMVCBuffer != NULL)
    {
        MSDK_SAFE_DELETE_ARRAY(pExtMVCBuffer->View);
        MSDK_SAFE_DELETE_ARRAY(pExtMVCBuffer->ViewId);
        MSDK_SAFE_DELETE_ARRAY(pExtMVCBuffer->OP);
    }

    MSDK_SAFE_DELETE(m_mfxVideoParams.ExtParam[0]);

    m_bIsExtBuffers = false;
}

mfxStatus CDecodingPipeline::ResetDecoder(sInputParams_dec *pParams)
{
    mfxStatus sts = MFX_ERR_NONE;

    // close decoder
    sts = m_pmfxDEC->Close();
    MSDK_IGNORE_MFX_STS(sts, MFX_ERR_NOT_INITIALIZED);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // free allocated frames
    DeleteFrames();

    // initialize parameters with values from parsed header
    sts = InitMfxParams(pParams);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // in case of HW accelerated decode frames must be allocated prior to decoder initialization
    sts = AllocFrames();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // init decoder
    sts = m_pmfxDEC->Init(&m_mfxVideoParams);
    if (MFX_WRN_PARTIAL_ACCELERATION == sts)
    {
        msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    return MFX_ERR_NONE;
}
#if 1
mfxStatus CDecodingPipeline::DeliverOutput(mfxFrameSurface1* frame)
{
    CAutoTimer timer_fwrite(m_tick_fwrite);

    mfxStatus res = MFX_ERR_NONE, sts = MFX_ERR_NONE;
    if (!frame) {
        return MFX_ERR_NULL_PTR;
    }
    if (m_bExternalAlloc) {
        if (m_eWorkMode == MODE_FILE_DUMP) {
            res = m_pMFXAllocator->Lock(m_pMFXAllocator->pthis, frame->Data.MemId, &(frame->Data));
            if (MFX_ERR_NONE == res) {
                res = m_FileWriter.DecOutFrame(frame,NULL,m_nDecNo);
                sts = m_pMFXAllocator->Unlock(m_pMFXAllocator->pthis, frame->Data.MemId, &(frame->Data));
            }
            if ((MFX_ERR_NONE == res) && (MFX_ERR_NONE != sts)) {
                res = sts;
            }
        } else if (m_eWorkMode == MODE_RENDERING) {
#if D3D_SURFACES_SUPPORT
            res = m_d3dRender.RenderFrame(frame, m_pMFXAllocator);
#elif LIBVA_SUPPORT
            res = m_hwdev->RenderFrame(frame, m_pMFXAllocator);
#endif
        }
    }
    else {
		res = m_FileWriter.DecOutFrame(frame,NULL,m_nDecNo);
    }

    return res;
}
#else
mfxStatus CDecodingPipeline::DeliverOutput(mfxFrameSurface1* frame)
{
    CAutoTimer timer_fwrite(m_tick_fwrite);

    mfxStatus res = MFX_ERR_NONE, sts = MFX_ERR_NONE;

    if (!frame) {
        return MFX_ERR_NULL_PTR;
    }

    if (m_bExternalAlloc) {
        if (m_eWorkMode == MODE_FILE_DUMP) {
            res = m_pMFXAllocator->Lock(m_pMFXAllocator->pthis, frame->Data.MemId, &(frame->Data));
            if (MFX_ERR_NONE == res) {
                res = m_FileWriter.WriteNextFrame(frame);
                sts = m_pMFXAllocator->Unlock(m_pMFXAllocator->pthis, frame->Data.MemId, &(frame->Data));
            }
            if ((MFX_ERR_NONE == res) && (MFX_ERR_NONE != sts)) {
                res = sts;
            }
        } else if (m_eWorkMode == MODE_RENDERING) {
#if D3D_SURFACES_SUPPORT
            res = m_d3dRender.RenderFrame(frame, m_pMFXAllocator);
#elif LIBVA_SUPPORT
            res = m_hwdev->RenderFrame(frame, m_pMFXAllocator);
#endif
        }
    }
    else {
        res = m_FileWriter.WriteNextFrame(frame);
    }

    return res;
}
#endif
mfxStatus CDecodingPipeline::DeliverLoop(void)
{
    mfxStatus res = MFX_ERR_NONE;

    while (!m_bStopDeliverLoop) {
        m_pDeliverOutputSemaphore->Wait();
        if (m_bStopDeliverLoop) {
            continue;
        }
        if (MFX_ERR_NONE != m_error) {
            continue;
        }
        msdkOutputSurface* pCurrentDeliveredSurface = m_DeliveredSurfacesPool.GetSurface();
        if (!pCurrentDeliveredSurface) {
            m_error = MFX_ERR_NULL_PTR;
            continue;
        }
        mfxFrameSurface1* frame = &(pCurrentDeliveredSurface->surface->frame);

        m_error = DeliverOutput(frame);
        ReturnSurfaceToBuffers(pCurrentDeliveredSurface);

        pCurrentDeliveredSurface = NULL;
        ++m_output_count;
        m_pDeliveredEvent->Signal();
    }
    return res;
}

unsigned int MFX_STDCALL CDecodingPipeline::DeliverThreadFunc(void* ctx)
{
    CDecodingPipeline* pipeline = (CDecodingPipeline*)ctx;

    mfxStatus sts;
    sts = pipeline->DeliverLoop();

    return 0;
}

void CDecodingPipeline::PrintPerFrameStat(bool force)
{
#define MY_COUNT 1 // TODO: this will be cmd option
#define MY_THRESHOLD 10000.0
    if ((!(m_output_count % MY_COUNT) && (m_eWorkMode != MODE_PERFORMANCE)) || force) {
        double fps, fps_fread, fps_fwrite;

        m_timer_overall.Sync();

        fps = (m_tick_overall)? m_output_count/CTimer::ConvertToSeconds(m_tick_overall): 0.0;
        fps_fread = (m_tick_fread)? m_output_count/CTimer::ConvertToSeconds(m_tick_fread): 0.0;
        fps_fwrite = (m_tick_fwrite)? m_output_count/CTimer::ConvertToSeconds(m_tick_fwrite): 0.0;
        // decoding progress
//        msdk_printf(MSDK_STRING("Frame number: %4d, fps: %0.3f, fread_fps: %0.3f, fwrite_fps: %.3f\r\n"),
//            m_output_count,
//            fps,
//            (fps_fread < MY_THRESHOLD)? fps_fread: 0.0,
//            (fps_fwrite < MY_THRESHOLD)? fps_fwrite: 0.0);
        fflush(NULL);
#if D3D_SURFACES_SUPPORT
        m_d3dRender.UpdateTitle(fps);
#elif LIBVA_SUPPORT
        if (m_hwdev) m_hwdev->UpdateTitle(fps);
#endif
    }
}

mfxStatus CDecodingPipeline::SyncOutputSurface_edukit(video_info_t *pvideo_info,mfxU32 wait)
{
    
    msdkFrameBuff* pFramebuff = NULL;
    mfxStatus sts = MFX_ERR_NONE;
    int i = 0;

    for(i = 0; i < m_FrameBuffCurNumber; i++)
    {
		pFramebuff = &(m_pFrameBuff[i]);
		sts = m_mfxSession.SyncOperation(pFramebuff->syncp, wait);
		if (MFX_ERR_NONE == sts) {
			pFramebuff->frame->pTexture2D = NULL;
			if (D3D11_MEMORY == m_memType)
			{
				mfxHDLPair pair = { NULL };
				sts = m_pMFXAllocator->GetHDL(m_pMFXAllocator->pthis, pFramebuff->frame->pSurfaces->Data.MemId, (mfxHDL*)&pair);
				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
				pFramebuff->frame->pTexture2D = reinterpret_cast<ID3D11Texture2D*>(pair.first);
			}
			pvideo_info->EncFrameBuffList.bufs[pvideo_info->EncFrameBuffList.numBufs] = pFramebuff->frame;
			pvideo_info->EncFrameBuffList.bufs[pvideo_info->EncFrameBuffList.numBufs]->ValidFrame = 1;
			pvideo_info->EncFrameBuffList.numBufs++;
			FreeFrameBuff(i);
		}
	}

    return sts;
}

mfxStatus CDecodingPipeline::SyncOutputSurface(mfxU32 wait)
{
    if (!m_pCurrentOutputSurface) {
        m_pCurrentOutputSurface = m_OutputSurfacesPool.GetSurface();
    }
    if (!m_pCurrentOutputSurface) {
        return MFX_ERR_MORE_DATA;
    }

    mfxStatus sts = m_mfxSession.SyncOperation(m_pCurrentOutputSurface->syncp, wait);

    if (MFX_WRN_IN_EXECUTION == sts) {
        return sts;
    }
    if (MFX_ERR_NONE == sts) {
        // we got completely decoded frame - pushing it to the delivering thread...
        ++m_synced_count;
        if (m_bPrintLatency) {
            m_vLatency.push_back(m_timer_overall.Sync() - m_pCurrentOutputSurface->surface->submit);
        }
        else {
            PrintPerFrameStat();
        }

        if (m_eWorkMode == MODE_PERFORMANCE) {
            m_output_count = m_synced_count;
            ReturnSurfaceToBuffers(m_pCurrentOutputSurface);
        } else if (m_eWorkMode == MODE_FILE_DUMP) {
            m_output_count = m_synced_count;
            sts = DeliverOutput(&(m_pCurrentOutputSurface->surface->frame));
            if (MFX_ERR_NONE != sts) {
                sts = MFX_ERR_UNKNOWN;
            }
            ReturnSurfaceToBuffers(m_pCurrentOutputSurface);
        } else if (m_eWorkMode == MODE_RENDERING) {
            if(m_nMaxFps)
            {
                //calculation of a time to sleep in order not to exceed a given fps
                mfxF64 currentTime = (m_output_count) ? CTimer::ConvertToSeconds(m_tick_overall) : 0.0;
                int time_to_sleep = (int)(1000 * ((double)m_output_count / m_nMaxFps - currentTime));
                if (time_to_sleep > 0)
                {
                    MSDK_SLEEP(time_to_sleep);
                }
            }
            m_DeliveredSurfacesPool.AddSurface(m_pCurrentOutputSurface);
            m_pDeliveredEvent->Reset();
            m_pDeliverOutputSemaphore->Post();
        }
        m_pCurrentOutputSurface = NULL;
    }

    if (MFX_ERR_NONE != sts) {
        sts = MFX_ERR_UNKNOWN;
    }

    return sts;
}
#if 1
mfxStatus CDecodingPipeline::RunDecodingBits(video_info_t *pvideo_info)
{
	mfxFrameSurface1*   pOutSurface = NULL;
	mfxFrameSurface1*   pInSurface = NULL;
	mfxSyncPoint 		psyncp = NULL;
	mfxBitstream*       pBitstream = &pvideo_info->pEncBitStrameBuff->Bitstream;
	mfxStatus           sts = MFX_ERR_NONE;
	mfxU32  TmpDataOffset = pBitstream->DataOffset;
#if 0
	if(m_mfxBS.DataLength > 0)
	{
		memmove(pBitstream->Data + m_mfxBS.DataLength,pBitstream->Data + pBitstream->DataOffset,pBitstream->DataLength);
		memcpy(pBitstream->Data,m_mfxBS.Data + m_mfxBS.DataOffset,m_mfxBS.DataLength);
		m_mfxBS.DataOffset = 0;
		m_mfxBS.DataLength = 0;
		pBitstream->DataOffset = 0;
		pBitstream->DataLength += m_mfxBS.DataLength;
	}
#endif
	pInSurface = pvideo_info->pEncSurfaces->pSurfaces;
	pInSurface->Data.Locked = 0;
        pOutSurface = NULL;
        do {
            sts = m_pmfxDEC->DecodeFrameAsync(pBitstream, pInSurface, &pOutSurface, &psyncp);

        } while (MFX_WRN_DEVICE_BUSY == sts);

        if (sts > MFX_ERR_NONE) {
            // ignoring warnings...
            if (psyncp) {
                // output is available
                sts = MFX_ERR_NONE;
            } else {
                // output is not available
                sts = MFX_ERR_MORE_SURFACE;
            }
        } else if ((MFX_ERR_MORE_DATA == sts) && pBitstream) {
        	  if(pBitstream->DataLength && (m_mfxBS.Data != NULL))
    	  	{
	            // In low_latency mode decoder have to process bitstream completely
	            msdk_printf(MSDK_STRING("error: Incorrect decoder behavior in low latency mode (bitstream length is not equal to 0 after decoding)\n"));
		     #if 0
		     memcpy(m_mfxBS.Data,pBitstream->Data + pBitstream->DataOffset,pBitstream->DataLength);
		     m_mfxBS.DataOffset = 0;
	            m_mfxBS.DataLength = pBitstream->DataLength;
		     pBitstream->DataOffset += pBitstream->DataLength;
	            pBitstream->DataLength = 0;
		     #else
		     pBitstream->DataOffset += pBitstream->DataLength;
	            pBitstream->DataLength = 0;
		     #endif
	            sts = MFX_ERR_UNDEFINED_BEHAVIOR;
    	  	}
        } 

        if (MFX_ERR_NONE == sts) {
		AddFrameBuff(pvideo_info->pEncSurfaces,psyncp);
		psyncp = NULL;
		pOutSurface= NULL;
        }

	 if(MFX_ERR_NONE != sts)
 	{
		pvideo_info->EncFrameBuffList.bufs[pvideo_info->EncFrameBuffList.numBufs] = pvideo_info->pEncSurfaces;
		pvideo_info->EncFrameBuffList.bufs[pvideo_info->EncFrameBuffList.numBufs]->ValidFrame = 0;
		pvideo_info->EncFrameBuffList.numBufs++;
 	}


	 //printf("===========sts:%d,data:%p,DataOffset:%d,DataLength:%d\n",sts,pvideo_info->pEncBitStrameBuff->Bitstream.Data,pvideo_info->pEncBitStrameBuff->Bitstream.DataOffset,pvideo_info->pEncBitStrameBuff->Bitstream.DataLength);

	if((pvideo_info->pEncBitStrameBuff->Bitstream.DataOffset == 0) || (pvideo_info->pEncBitStrameBuff->Bitstream.DataLength != 0))
	{
		pvideo_info->EncBitStrameBuffList.NeedGetBits = 0;
	}
	else
	{
		pvideo_info->EncBitStrameBuffList.bufs[pvideo_info->EncBitStrameBuffList.numBufs] = pvideo_info->pEncBitStrameBuff;
		pvideo_info->EncBitStrameBuffList.numBufs++;
		pvideo_info->EncBitStrameBuffList.NeedGetBits = 1;
	}
	 sts = SyncOutputSurface_edukit(pvideo_info,MSDK_DEC_WAIT_INTERVAL);

	 return sts;

}
#else
mfxStatus CDecodingPipeline::RunDecodingBits(video_info_t *pvideo_info)
{
    mfxFrameSurface1*   pOutSurface = NULL;
    mfxBitstream*       pBitstream = &pvideo_info->pEncBitStrameBuff->Bitstream;//&m_mfxBS;
    mfxStatus           sts = MFX_ERR_NONE;
    bool                bErrIncompatibleVideoParams = false;
    CTimeInterval<>     decodeTimer(m_bIsCompleteFrame);
    time_t start_time = time(0);
    MSDKThread * pDeliverThread = NULL;

    if (m_eWorkMode == MODE_RENDERING) {
        m_pDeliverOutputSemaphore = new MSDKSemaphore(sts);
        m_pDeliveredEvent = new MSDKEvent(sts, false, false);
        pDeliverThread = new MSDKThread(sts, DeliverThreadFunc, this);
        if (!pDeliverThread || !m_pDeliverOutputSemaphore || !m_pDeliveredEvent) {
            MSDK_SAFE_DELETE(pDeliverThread);
            MSDK_SAFE_DELETE(m_pDeliverOutputSemaphore);
            MSDK_SAFE_DELETE(m_pDeliveredEvent);
            return MFX_ERR_MEMORY_ALLOC;
        }
    }

    if (MFX_CODEC_CAPTURE == this->m_mfxVideoParams.mfx.CodecId)
    {
        pBitstream = 0;
    }

    while (((sts == MFX_ERR_NONE) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) && (m_nFrames > m_output_count)){
        if (MFX_ERR_NONE != m_error) {
            msdk_printf(MSDK_STRING("DeliverOutput return error = %d\n"),m_error);
            break;
        }
        if (pBitstream && ((MFX_ERR_MORE_DATA == sts) || (m_bIsCompleteFrame && !pBitstream->DataLength))) {
            CAutoTimer timer_fread(m_tick_fread);
            sts = m_FileReader->GetNetData(pBitstream,m_nDecNo); // read more data to input bit stream
              
		//	 printf("wwwwwwwwwwwwwGetNetData=%x\n");  

            if (MFX_ERR_MORE_DATA == sts) {
                if (!m_bIsVideoWall) {
                    // we almost reached end of stream, need to pull buffered data now
                    pBitstream = NULL;
                    sts = MFX_ERR_NONE;
                } else {
                    // videowall mode: decoding in a loop
                    m_FileReader->Reset();
                    sts = MFX_ERR_NONE;
                    continue;
                }
			} else if (MFX_ERR_NONE != sts) {
                break;
            }
        }
        if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) {
            // here we check whether output is ready, though we do not wait...
#ifndef __SYNC_WA
            mfxStatus _sts = SyncOutputSurface(0);
            if (MFX_ERR_UNKNOWN == _sts) {
                sts = _sts;
                break;
            } else if (MFX_ERR_NONE == _sts) {
                continue;
            }
#endif
        }
        if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) {
            SyncFrameSurfaces();
            if (!m_pCurrentFreeSurface) {
                m_pCurrentFreeSurface = m_FreeSurfacesPool.GetSurface();
            }
#ifndef __SYNC_WA
            if (!m_pCurrentFreeSurface) 
#else
            if (!m_pCurrentFreeSurface || (m_OutputSurfacesPool.GetSurfaceCount() == m_mfxVideoParams.AsyncDepth)) 
#endif
           	{
           	// we stuck with no free surface available, now we will sync...
                sts = SyncOutputSurface(MSDK_DEC_WAIT_INTERVAL);
                if (MFX_ERR_MORE_DATA == sts) {
                    if ((m_eWorkMode == MODE_PERFORMANCE) || (m_eWorkMode == MODE_FILE_DUMP)) {
                        sts = MFX_ERR_NOT_FOUND;
                    } else if (m_eWorkMode == MODE_RENDERING) {
                        if (m_synced_count != m_output_count) {
                            sts = m_pDeliveredEvent->TimedWait(MSDK_DEC_WAIT_INTERVAL);
                        } else {
                            sts = MFX_ERR_NOT_FOUND;
                        }
                    }
                    if (MFX_ERR_NOT_FOUND == sts) {
                        msdk_printf(MSDK_STRING("fatal: failed to find output surface, that's a bug!\n"));
                        break;
                    }
                }
                // note: MFX_WRN_IN_EXECUTION will also be treated as an error at this point
                continue;
            }

            if (!m_pCurrentFreeOutputSurface) {
                m_pCurrentFreeOutputSurface = GetFreeOutputSurface();
            }
            if (!m_pCurrentFreeOutputSurface) {
                sts = MFX_ERR_NOT_FOUND;
                break;
            }
        }

        // exit by timeout
        if ((MFX_ERR_NONE == sts) && m_bIsVideoWall && (time(0)-start_time) >= m_nTimeout) {
            sts = MFX_ERR_NONE;
            break;
        }

        if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) {
            if (m_bIsCompleteFrame) {
                m_pCurrentFreeSurface->submit = m_timer_overall.Sync();
            }
            pOutSurface = NULL;
            do {
                sts = m_pmfxDEC->DecodeFrameAsync(pBitstream, &(m_pCurrentFreeSurface->frame), &pOutSurface, &(m_pCurrentFreeOutputSurface->syncp));

                if (MFX_WRN_DEVICE_BUSY == sts) {
                    if (m_bIsCompleteFrame) {
                        //in low latency mode device busy leads to increasing of latency
                        //msdk_printf(MSDK_STRING("Warning : latency increased due to MFX_WRN_DEVICE_BUSY\n"));
                    }
                    mfxStatus _sts = SyncOutputSurface(MSDK_DEC_WAIT_INTERVAL);
                    // note: everything except MFX_ERR_NONE are errors at this point
                    if (MFX_ERR_NONE == _sts) {
                        sts = MFX_WRN_DEVICE_BUSY;
                    } else {
                        sts = _sts;
                        if (MFX_ERR_MORE_DATA == sts) {
                            // we can't receive MFX_ERR_MORE_DATA and have no output - that's a bug
                            sts = MFX_WRN_DEVICE_BUSY;//MFX_ERR_NOT_FOUND;
                        }
                    }
                }

		printf("===============sts:%d,%p,%p,%p\n",sts,&(m_pCurrentFreeSurface->frame),pOutSurface,m_pCurrentFreeOutputSurface->syncp);
            } while (MFX_WRN_DEVICE_BUSY == sts);

            if (sts > MFX_ERR_NONE) {
                // ignoring warnings...
                if (m_pCurrentFreeOutputSurface->syncp) {
                    MSDK_SELF_CHECK(pOutSurface);
                    // output is available
                    sts = MFX_ERR_NONE;
                } else {
                    // output is not available
                    sts = MFX_ERR_MORE_SURFACE;
                }
            } else if ((MFX_ERR_MORE_DATA == sts) && pBitstream) {
                if (m_bIsCompleteFrame && pBitstream->DataLength)
                {
                    // In low_latency mode decoder have to process bitstream completely
                    msdk_printf(MSDK_STRING("error: Incorrect decoder behavior in low latency mode (bitstream length is not equal to 0 after decoding)\n"));
                    sts = MFX_ERR_UNDEFINED_BEHAVIOR;
                    continue;
                }
            } else if ((MFX_ERR_MORE_DATA == sts) && !pBitstream) {
                // that's it - we reached end of stream; now we need to render bufferred data...
                do {
                    sts = SyncOutputSurface(MSDK_DEC_WAIT_INTERVAL);
                } while (MFX_ERR_NONE == sts);

                while (m_synced_count != m_output_count) {
                    m_pDeliveredEvent->Wait();
                }

                if (MFX_ERR_MORE_DATA == sts) {
                    sts = MFX_ERR_NONE;
                }
                break;
            } else if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts) {
                bErrIncompatibleVideoParams = true;
                // need to go to the buffering loop prior to reset procedure
                pBitstream = NULL;
                sts = MFX_ERR_NONE;
                continue;
            }
        }

        if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) {
            // if current free surface is locked we are moving it to the used surfaces array
            /*if (m_pCurrentFreeSurface->frame.Data.Locked)*/ {
                m_UsedSurfacesPool.AddSurface(m_pCurrentFreeSurface);
                m_pCurrentFreeSurface = NULL;
            }
        }
        if (MFX_ERR_NONE == sts) {
            msdkFrameSurface* surface = FindUsedSurface(pOutSurface);

            msdk_atomic_inc16(&(surface->render_lock));

            m_pCurrentFreeOutputSurface->surface = surface;
            m_OutputSurfacesPool.AddSurface(m_pCurrentFreeOutputSurface);
            m_pCurrentFreeOutputSurface = NULL;
        }
    } //while processing

    PrintPerFrameStat(true);

    if (m_bPrintLatency && m_vLatency.size() > 0) {
        unsigned int frame_idx = 0;
        msdk_tick sum = 0;
        for (std::vector<msdk_tick>::iterator it = m_vLatency.begin(); it != m_vLatency.end(); ++it)
        {
            sum += *it;
            msdk_printf(MSDK_STRING("Frame %4d, latency=%5.5f ms\n"), ++frame_idx, CTimer::ConvertToSeconds(*it)*1000);
        }
        msdk_printf(MSDK_STRING("\nLatency summary:\n"));
        msdk_printf(MSDK_STRING("\nAVG=%5.5f ms, MAX=%5.5f ms, MIN=%5.5f ms"),
            CTimer::ConvertToSeconds((msdk_tick)((mfxF64)sum/m_vLatency.size()))*1000,
            CTimer::ConvertToSeconds(*std::max_element(m_vLatency.begin(), m_vLatency.end()))*1000,
            CTimer::ConvertToSeconds(*std::min_element(m_vLatency.begin(), m_vLatency.end()))*1000);
    }

    if (m_eWorkMode == MODE_RENDERING) {
        m_bStopDeliverLoop = true;
        m_pDeliverOutputSemaphore->Post();
        if (pDeliverThread)
            pDeliverThread->Wait();
    }

    MSDK_SAFE_DELETE(pDeliverThread);
    MSDK_SAFE_DELETE(m_pDeliverOutputSemaphore);
    MSDK_SAFE_DELETE(m_pDeliveredEvent);
    // exit in case of other errors
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // if we exited main decoding loop with ERR_INCOMPATIBLE_PARAM we need to send this status to caller
    if (bErrIncompatibleVideoParams) {
        sts = MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }

    return sts; // ERR_NONE or ERR_INCOMPATIBLE_VIDEO_PARAM
}

#endif

#if 0
mfxStatus CDecodingPipeline::RunDecoding()
{
    mfxFrameSurface1*   pOutSurface = NULL;
    mfxBitstream*       pBitstream = &m_mfxBS;
    mfxStatus           sts = MFX_ERR_NONE;
    bool                bErrIncompatibleVideoParams = false;
    CTimeInterval<>     decodeTimer(m_bIsCompleteFrame);
    time_t start_time = time(0);
    MSDKThread * pDeliverThread = NULL;

    if (m_eWorkMode == MODE_RENDERING) {
        m_pDeliverOutputSemaphore = new MSDKSemaphore(sts);
        m_pDeliveredEvent = new MSDKEvent(sts, false, false);
        pDeliverThread = new MSDKThread(sts, DeliverThreadFunc, this);
        if (!pDeliverThread || !m_pDeliverOutputSemaphore || !m_pDeliveredEvent) {
            MSDK_SAFE_DELETE(pDeliverThread);
            MSDK_SAFE_DELETE(m_pDeliverOutputSemaphore);
            MSDK_SAFE_DELETE(m_pDeliveredEvent);
            return MFX_ERR_MEMORY_ALLOC;
        }
    }

    if (MFX_CODEC_CAPTURE == this->m_mfxVideoParams.mfx.CodecId)
    {
        pBitstream = 0;
    }

    while (((sts == MFX_ERR_NONE) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) && (m_nFrames > m_output_count)){
        if (MFX_ERR_NONE != m_error) {
            msdk_printf(MSDK_STRING("DeliverOutput return error = %d\n"),m_error);
            break;
        }
        if (pBitstream && ((MFX_ERR_MORE_DATA == sts) || (m_bIsCompleteFrame && !pBitstream->DataLength))) {
            CAutoTimer timer_fread(m_tick_fread);
            sts = m_FileReader->ReadNextFrame(pBitstream); // read more data to input bit stream
              
		//	 printf("wwwwwwwwwwwwwGetNetData=%x\n");  

            if (MFX_ERR_MORE_DATA == sts) {
                if (!m_bIsVideoWall) {
                    // we almost reached end of stream, need to pull buffered data now
                    pBitstream = NULL;
                    sts = MFX_ERR_NONE;
                } else {
                    // videowall mode: decoding in a loop
                    m_FileReader->Reset();
                    sts = MFX_ERR_NONE;
                    continue;
                }
			} else if (MFX_ERR_NONE != sts) {
                break;
            }
        }
        if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) {
            // here we check whether output is ready, though we do not wait...
#ifndef __SYNC_WA
            mfxStatus _sts = SyncOutputSurface(0);
            if (MFX_ERR_UNKNOWN == _sts) {
                sts = _sts;
                break;
            } else if (MFX_ERR_NONE == _sts) {
                continue;
            }
#endif
        }
        if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) {
            SyncFrameSurfaces();
            if (!m_pCurrentFreeSurface) {
                m_pCurrentFreeSurface = m_FreeSurfacesPool.GetSurface();
            }
#ifndef __SYNC_WA
            if (!m_pCurrentFreeSurface) 
#else
            if (!m_pCurrentFreeSurface || (m_OutputSurfacesPool.GetSurfaceCount() == m_mfxVideoParams.AsyncDepth)) 
#endif
           	{
           	// we stuck with no free surface available, now we will sync...
                sts = SyncOutputSurface(MSDK_DEC_WAIT_INTERVAL);
                if (MFX_ERR_MORE_DATA == sts) {
                    if ((m_eWorkMode == MODE_PERFORMANCE) || (m_eWorkMode == MODE_FILE_DUMP)) {
                        sts = MFX_ERR_NOT_FOUND;
                    } else if (m_eWorkMode == MODE_RENDERING) {
                        if (m_synced_count != m_output_count) {
                            sts = m_pDeliveredEvent->TimedWait(MSDK_DEC_WAIT_INTERVAL);
                        } else {
                            sts = MFX_ERR_NOT_FOUND;
                        }
                    }
                    if (MFX_ERR_NOT_FOUND == sts) {
                        msdk_printf(MSDK_STRING("fatal: failed to find output surface, that's a bug!\n"));
                        break;
                    }
                }
                // note: MFX_WRN_IN_EXECUTION will also be treated as an error at this point
                continue;
            }

            if (!m_pCurrentFreeOutputSurface) {
                m_pCurrentFreeOutputSurface = GetFreeOutputSurface();
            }
            if (!m_pCurrentFreeOutputSurface) {
                sts = MFX_ERR_NOT_FOUND;
                break;
            }
        }

        // exit by timeout
        if ((MFX_ERR_NONE == sts) && m_bIsVideoWall && (time(0)-start_time) >= m_nTimeout) {
            sts = MFX_ERR_NONE;
            break;
        }

        if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) {
            if (m_bIsCompleteFrame) {
                m_pCurrentFreeSurface->submit = m_timer_overall.Sync();
            }
            pOutSurface = NULL;
            do {
                sts = m_pmfxDEC->DecodeFrameAsync(pBitstream, &(m_pCurrentFreeSurface->frame), &pOutSurface, &(m_pCurrentFreeOutputSurface->syncp));
			printf("DecodeFrameAsync sts:%d\n",sts);
                if (MFX_WRN_DEVICE_BUSY == sts) {
                    if (m_bIsCompleteFrame) {
                        //in low latency mode device busy leads to increasing of latency
                        //msdk_printf(MSDK_STRING("Warning : latency increased due to MFX_WRN_DEVICE_BUSY\n"));
                    }
                    mfxStatus _sts = SyncOutputSurface(MSDK_DEC_WAIT_INTERVAL);
                    // note: everything except MFX_ERR_NONE are errors at this point
                    if (MFX_ERR_NONE == _sts) {
                        sts = MFX_WRN_DEVICE_BUSY;
                    } else {
                        sts = _sts;
                        if (MFX_ERR_MORE_DATA == sts) {
                            // we can't receive MFX_ERR_MORE_DATA and have no output - that's a bug
                            sts = MFX_WRN_DEVICE_BUSY;//MFX_ERR_NOT_FOUND;
                        }
                    }
                }
            } while (MFX_WRN_DEVICE_BUSY == sts);

            if (sts > MFX_ERR_NONE) {
                // ignoring warnings...
                if (m_pCurrentFreeOutputSurface->syncp) {
                    MSDK_SELF_CHECK(pOutSurface);
                    // output is available
                    sts = MFX_ERR_NONE;
                } else {
                    // output is not available
                    sts = MFX_ERR_MORE_SURFACE;
                }
            } else if ((MFX_ERR_MORE_DATA == sts) && pBitstream) {
                if (m_bIsCompleteFrame && pBitstream->DataLength)
                {
                    // In low_latency mode decoder have to process bitstream completely
                    msdk_printf(MSDK_STRING("error: Incorrect decoder behavior in low latency mode (bitstream length is not equal to 0 after decoding)\n"));
                    sts = MFX_ERR_UNDEFINED_BEHAVIOR;
                    continue;
                }
            } else if ((MFX_ERR_MORE_DATA == sts) && !pBitstream) {
                // that's it - we reached end of stream; now we need to render bufferred data...
                do {
                    sts = SyncOutputSurface(MSDK_DEC_WAIT_INTERVAL);
                } while (MFX_ERR_NONE == sts);

                while (m_synced_count != m_output_count) {
                    m_pDeliveredEvent->Wait();
                }

                if (MFX_ERR_MORE_DATA == sts) {
                    sts = MFX_ERR_NONE;
                }
                break;
            } else if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts) {
                bErrIncompatibleVideoParams = true;
                // need to go to the buffering loop prior to reset procedure
                pBitstream = NULL;
                sts = MFX_ERR_NONE;
                continue;
            }
        }

        if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) {
            // if current free surface is locked we are moving it to the used surfaces array
            /*if (m_pCurrentFreeSurface->frame.Data.Locked)*/ {
                m_UsedSurfacesPool.AddSurface(m_pCurrentFreeSurface);
                m_pCurrentFreeSurface = NULL;
            }
        }
        if (MFX_ERR_NONE == sts) {
            msdkFrameSurface* surface = FindUsedSurface(pOutSurface);

            msdk_atomic_inc16(&(surface->render_lock));

            m_pCurrentFreeOutputSurface->surface = surface;
            m_OutputSurfacesPool.AddSurface(m_pCurrentFreeOutputSurface);
            m_pCurrentFreeOutputSurface = NULL;
        }
    } //while processing

    PrintPerFrameStat(true);

    if (m_bPrintLatency && m_vLatency.size() > 0) {
        unsigned int frame_idx = 0;
        msdk_tick sum = 0;
        for (std::vector<msdk_tick>::iterator it = m_vLatency.begin(); it != m_vLatency.end(); ++it)
        {
            sum += *it;
            msdk_printf(MSDK_STRING("Frame %4d, latency=%5.5f ms\n"), ++frame_idx, CTimer::ConvertToSeconds(*it)*1000);
        }
        msdk_printf(MSDK_STRING("\nLatency summary:\n"));
        msdk_printf(MSDK_STRING("\nAVG=%5.5f ms, MAX=%5.5f ms, MIN=%5.5f ms"),
            CTimer::ConvertToSeconds((msdk_tick)((mfxF64)sum/m_vLatency.size()))*1000,
            CTimer::ConvertToSeconds(*std::max_element(m_vLatency.begin(), m_vLatency.end()))*1000,
            CTimer::ConvertToSeconds(*std::min_element(m_vLatency.begin(), m_vLatency.end()))*1000);
    }

    if (m_eWorkMode == MODE_RENDERING) {
        m_bStopDeliverLoop = true;
        m_pDeliverOutputSemaphore->Post();
        if (pDeliverThread)
            pDeliverThread->Wait();
    }

    MSDK_SAFE_DELETE(pDeliverThread);
    MSDK_SAFE_DELETE(m_pDeliverOutputSemaphore);
    MSDK_SAFE_DELETE(m_pDeliveredEvent);
    // exit in case of other errors
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // if we exited main decoding loop with ERR_INCOMPATIBLE_PARAM we need to send this status to caller
    if (bErrIncompatibleVideoParams) {
        sts = MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }

    return sts; // ERR_NONE or ERR_INCOMPATIBLE_VIDEO_PARAM
}
#else
mfxStatus CDecodingPipeline::RunDecoding()
{
   mfxFrameSurface1*   pOutSurface = NULL;
    mfxBitstream*       pBitstream = &m_mfxBS;
    mfxStatus           sts = MFX_ERR_NONE;

    while (((sts == MFX_ERR_NONE) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts))){
        if (pBitstream && ((MFX_ERR_MORE_DATA == sts) ||!pBitstream->DataLength)) {
            sts = m_FileReader->ReadNextFrame(pBitstream); // read more data to input bit stream
        }
		
        if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) {
            SyncFrameSurfaces();
            if (!m_pCurrentFreeSurface) {
                m_pCurrentFreeSurface = m_FreeSurfacesPool.GetSurface();
            }
#if 0
#ifndef __SYNC_WA
            if (!m_pCurrentFreeSurface) 
#else
            if (!m_pCurrentFreeSurface || (m_OutputSurfacesPool.GetSurfaceCount() == m_mfxVideoParams.AsyncDepth)) 
#endif
           	{
           	// we stuck with no free surface available, now we will sync...
                sts = SyncOutputSurface(MSDK_DEC_WAIT_INTERVAL);
                if (MFX_ERR_MORE_DATA == sts) {
                    if ((m_eWorkMode == MODE_PERFORMANCE) || (m_eWorkMode == MODE_FILE_DUMP)) {
                        sts = MFX_ERR_NOT_FOUND;
                    } else if (m_eWorkMode == MODE_RENDERING) {
                        if (m_synced_count != m_output_count) {
                            sts = m_pDeliveredEvent->TimedWait(MSDK_DEC_WAIT_INTERVAL);
                        } else {
                            sts = MFX_ERR_NOT_FOUND;
                        }
                    }
                    if (MFX_ERR_NOT_FOUND == sts) {
                        msdk_printf(MSDK_STRING("fatal: failed to find output surface, that's a bug!\n"));
                        break;
                    }
                }
                // note: MFX_WRN_IN_EXECUTION will also be treated as an error at this point
                continue;
            }
#endif
            if (!m_pCurrentFreeOutputSurface) {
                m_pCurrentFreeOutputSurface = GetFreeOutputSurface();
            }
            if (!m_pCurrentFreeOutputSurface) {
                sts = MFX_ERR_NOT_FOUND;
                break;
            }
        }

        if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) {
            pOutSurface = NULL;
            do {
		   printf("pBitstream:%p,%p,%p,%p\n",pBitstream,m_pCurrentFreeSurface,pOutSurface,m_pCurrentFreeOutputSurface);
                sts = m_pmfxDEC->DecodeFrameAsync(pBitstream, &(m_pCurrentFreeSurface->frame), &pOutSurface, &(m_pCurrentFreeOutputSurface->syncp));
			printf("DecodeFrameAsync sts:%d\n",sts);
                if (MFX_WRN_DEVICE_BUSY == sts) {
                    if (m_bIsCompleteFrame) {
                        //in low latency mode device busy leads to increasing of latency
                        //msdk_printf(MSDK_STRING("Warning : latency increased due to MFX_WRN_DEVICE_BUSY\n"));
                    }
                    mfxStatus _sts = SyncOutputSurface(MSDK_DEC_WAIT_INTERVAL);
                    // note: everything except MFX_ERR_NONE are errors at this point
                    if (MFX_ERR_NONE == _sts) {
                        sts = MFX_WRN_DEVICE_BUSY;
                    } else {
                        sts = _sts;
                        if (MFX_ERR_MORE_DATA == sts) {
                            // we can't receive MFX_ERR_MORE_DATA and have no output - that's a bug
                            sts = MFX_WRN_DEVICE_BUSY;//MFX_ERR_NOT_FOUND;
                        }
                    }
                }
            } while (MFX_WRN_DEVICE_BUSY == sts);

            if (sts > MFX_ERR_NONE) {
                // ignoring warnings...
                if (m_pCurrentFreeOutputSurface->syncp) {
                    MSDK_SELF_CHECK(pOutSurface);
                    // output is available
                    sts = MFX_ERR_NONE;
                } else {
                    // output is not available
                    sts = MFX_ERR_MORE_SURFACE;
                }
            } else if ((MFX_ERR_MORE_DATA == sts) && pBitstream) {
                if (m_bIsCompleteFrame && pBitstream->DataLength)
                {
                    // In low_latency mode decoder have to process bitstream completely
                    msdk_printf(MSDK_STRING("error: Incorrect decoder behavior in low latency mode (bitstream length is not equal to 0 after decoding)\n"));
                    sts = MFX_ERR_UNDEFINED_BEHAVIOR;
                    continue;
                }
            } else if ((MFX_ERR_MORE_DATA == sts) && !pBitstream) {
                // that's it - we reached end of stream; now we need to render bufferred data...
                do {
                    sts = SyncOutputSurface(MSDK_DEC_WAIT_INTERVAL);
                } while (MFX_ERR_NONE == sts);

                if (MFX_ERR_MORE_DATA == sts) {
                    sts = MFX_ERR_NONE;
                }
                break;
            } else if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts) {
                // need to go to the buffering loop prior to reset procedure
                pBitstream = NULL;
                sts = MFX_ERR_NONE;
                continue;
            }
        }

        if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_DATA == sts) || (MFX_ERR_MORE_SURFACE == sts)) {
            // if current free surface is locked we are moving it to the used surfaces array
            /*if (m_pCurrentFreeSurface->frame.Data.Locked)*/ {
                m_UsedSurfacesPool.AddSurface(m_pCurrentFreeSurface);
                m_pCurrentFreeSurface = NULL;
            }
        }

printf("DecodeFrameAsync8 sts:%d,m_pCurrentFreeSurface:%p,syncp:%p,pOutSurface:%p,offset:%d,datalen:%d\n",sts,m_pCurrentFreeSurface,m_pCurrentFreeOutputSurface->syncp,pOutSurface,pBitstream->DataOffset,pBitstream->DataLength);
        if (MFX_ERR_NONE == sts) {
            msdkFrameSurface* surface = FindUsedSurface(pOutSurface);

            msdk_atomic_inc16(&(surface->render_lock));

            m_pCurrentFreeOutputSurface->surface = surface;
            m_OutputSurfacesPool.AddSurface(m_pCurrentFreeOutputSurface);
            m_pCurrentFreeOutputSurface = NULL;
        }
    } //while processing

    return sts; // ERR_NONE or ERR_INCOMPATIBLE_VIDEO_PARAM

}
#endif

void CDecodingPipeline::PrintInfo()
{
    msdk_printf(MSDK_STRING("Decoding Sample Version %s\n\n"), MSDK_SAMPLE_VERSION);
    msdk_printf(MSDK_STRING("\nInput video\t%s\n"), CodecIdToStr(m_mfxVideoParams.mfx.CodecId).c_str());
    msdk_printf(MSDK_STRING("Output format\t%s\n"), MSDK_STRING("YUV420"));

    mfxFrameInfo Info = m_mfxVideoParams.mfx.FrameInfo;
    msdk_printf(MSDK_STRING("Resolution\t%dx%d\n"), Info.Width, Info.Height);
    msdk_printf(MSDK_STRING("Crop X,Y,W,H\t%d,%d,%d,%d\n"), Info.CropX, Info.CropY, Info.CropW, Info.CropH);

    mfxF64 dFrameRate = CalculateFrameRate(Info.FrameRateExtN, Info.FrameRateExtD);
    msdk_printf(MSDK_STRING("Frame rate\t%.2f\n"), dFrameRate);

    const msdk_char* sMemType = m_memType == D3D9_MEMORY  ? MSDK_STRING("d3d")
                             : (m_memType == D3D11_MEMORY ? MSDK_STRING("d3d11")
                                                          : MSDK_STRING("system"));
    msdk_printf(MSDK_STRING("Memory type\t\t%s\n"), sMemType);

    mfxIMPL impl;
    m_mfxSession.QueryIMPL(&impl);

    const msdk_char* sImpl = (MFX_IMPL_VIA_D3D11 == MFX_IMPL_VIA_MASK(impl)) ? MSDK_STRING("hw_d3d11")
                           : (MFX_IMPL_HARDWARE & impl)  ? MSDK_STRING("hw")
                                                         : MSDK_STRING("sw");
    msdk_printf(MSDK_STRING("MediaSDK impl\t\t%s\n"), sImpl);

    mfxVersion ver;
    m_mfxSession.QueryVersion(&ver);
    msdk_printf(MSDK_STRING("MediaSDK version\t%d.%d\n"), ver.Major, ver.Minor);

    msdk_printf(MSDK_STRING("\n"));

    return;
}
