//* ////////////////////////////////////////////////////////////////////////////// */
//*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//        Copyright (c) 2005-2014 Intel Corporation. All Rights Reserved.
//
//
//*/

#ifndef __PIPELINE_ENCODE_H__
#define __PIPELINE_ENCODE_H__
#include "pipeline_common.h"

#include "sample_defs.h"
#include "hw_device.h"

#ifdef D3D_SURFACES_SUPPORT
#pragma warning(disable : 4201)
#endif
#include "sample_utils.h"
#include "sample_params.h"
#include "base_allocator.h"

#include "mfxmvc.h"
#include "mfxvideo.h"
#include "mfxvp8.h"
#include "mfxvideo++.h"
#include "mfxplugin.h"
#include "mfxplugin++.h"

#include <vector>
#include <memory>

#include "plugin_loader.h"
#include "linkVideo.h"

msdk_tick time_get_tick(void);
msdk_tick time_get_frequency(void);

enum {
    MVC_DISABLED          = 0x0,
    MVC_ENABLED           = 0x1,
    MVC_VIEWOUTPUT        = 0x2,    // 2 output bitstreams
};
/*
enum MemType {
    SYSTEM_MEMORY = 0x00,
    D3D9_MEMORY   = 0x01,
    D3D11_MEMORY  = 0x02,
};*/

struct sInputParams
{
    mfxU16 nTargetUsage;
    mfxU32 CodecId;
    mfxU32 ColorFormat;
    mfxU16 nPicStruct;
    mfxU16 nWidth; // source picture width
    mfxU16 nHeight; // source picture height
    mfxF64 dFrameRate;
    mfxU16 nBitRate;
    mfxU16 MVC_flags;
    mfxU16 nQuality; // quality parameter for JPEG encoder

    mfxU32 numViews; // number of views for Multi-View Codec

    mfxU16 nStartX; // destination picture width, specified if resizing required
    mfxU16 nStartY; // destination picture height, specified if resizing required
    mfxU16 nDstWidth; // destination picture width, specified if resizing required
    mfxU16 nDstHeight; // destination picture height, specified if resizing required

    MemType memType;
    bool bUseHWLib; // true if application wants to use HW MSDK library

    msdk_char strSrcFile[MSDK_MAX_FILENAME_LEN];

    sPluginParams pluginParams;

    std::vector<msdk_char*> srcFileBuff;
    std::vector<msdk_char*> dstFileBuff;

    mfxU32  HEVCPluginVersion;
    mfxU8 nRotationAngle; // if specified, enables rotation plugin in mfx pipeline
    msdk_char strPluginDLLPath[MSDK_MAX_FILENAME_LEN]; // plugin dll path and name

    mfxU16 nAsyncDepth; // depth of asynchronous pipeline, this number can be tuned to achieve better performance

    mfxU16 nRateControlMethod;
    mfxU16 nLADepth; // depth of the look ahead bitrate control  algorithm
    mfxU16 nMaxSliceSize; //maximum size of slice
    mfxU16 nQPI;
    mfxU16 nQPP;
    mfxU16 nQPB;

    mfxU16 nNumSlice;
    bool UseRegionEncode;
	//20160114 add
	mfxU16 nEncodeProfie; // depth of the look ahead bitrate control  algorithm
	mfxU16 nProfieLevel; // depth of the look ahead bitrate control  algorithm
	mfxU16 nGopszie; // GOP
    mfxU32  nEncNo;//编码路数
};

struct sTask
{
    mfxBitstream *pmfxBS;
    mfxSyncPoint EncSyncP;
    std::list<mfxSyncPoint> DependentVppTasks;
    CSmplBitstreamWriter *pWriter;

    sTask();
    mfxStatus WriteBitstream();
    mfxStatus Reset();
    mfxStatus Init(mfxU32 nBufferSize, CSmplBitstreamWriter *pWriter = NULL);
    mfxStatus Close();
};

class CEncTaskPool
{
public:
    CEncTaskPool();
    virtual ~CEncTaskPool();

    virtual mfxStatus Init(MFXVideoSession* pmfxSession, CSmplBitstreamWriter* pWriter, mfxU32 nPoolSize, mfxU32 nBufferSize, CSmplBitstreamWriter *pOtherWriter = NULL);
    virtual mfxStatus GetFreeTask(sTask **ppTask,mfxBitstream *pmfxBS);
    virtual mfxStatus SynchronizeFirstTask(video_info_t *pvideo_info);
    virtual mfxStatus ReleaseTask(video_info_t *pvideo_info);
    virtual void Close();

protected:
    sTask* m_pTasks;
    mfxU32 m_nPoolSize;
    mfxU32 m_nPoolCount;
    mfxU32 m_nTaskBufferStart;

    MFXVideoSession* m_pmfxSession;

    virtual mfxU32 GetFreeTaskIndex(mfxBitstream *pmfxBS);
    virtual mfxStatus DeleteUsedTask(mfxU32 index);
};

/* This class implements a pipeline with 2 mfx components: vpp (video preprocessing) and encode */
class CEncodingPipeline
{
public:
    mfxFrameSurface1* m_pEncSurfaces; // frames array for encoder input (vpp output)
    mfxFrameAllocResponse m_EncResponse;  // memory allocation response for encoder
    CEncodingPipeline();
    virtual ~CEncodingPipeline();

    virtual mfxStatus Init(sInputParams *pParams);
    virtual mfxStatus InitAlloc(sInputParams *pParams);
    virtual mfxStatus Run();
    virtual mfxStatus EncodeRun(video_info_t *pvideo_info);
    virtual void Close();
    virtual mfxStatus ResetMFXComponents(sInputParams* pParams);
    virtual mfxStatus ResetMFXComponents_alloc(sInputParams* pParams);
    virtual mfxStatus ResetDevice();
    virtual mfxStatus ResetEncPara(sInputParams *pParams,video_info_t *pvideo_info);

    void SetMultiView();
    void SetNumView(mfxU32 numViews) { m_nNumView = numViews; }
    virtual void  PrintInfo();

protected:
    std::pair<CSmplBitstreamWriter *,
              CSmplBitstreamWriter *> m_FileWriters;
    CSmplYUVReader m_FileReader;
    CEncTaskPool m_TaskPool;

    MFXVideoSession m_mfxSession;
    MFXVideoENCODE* m_pmfxENC;

    mfxVideoParam m_mfxEncParams;

    mfxU16 m_MVCflags; // MVC codec is in use

    std::auto_ptr<MFXVideoUSER> m_pUserModule;
    std::auto_ptr<MFXPlugin> m_pPlugin;

    MFXFrameAllocator* m_pMFXAllocator;
    mfxAllocatorParams* m_pmfxAllocatorParams;
    MemType m_memType;
    bool m_bExternalAlloc; // use memory allocator as external for Media SDK

    FRAME_Buf** m_ppEncSurfaces; // frames array for encoder input (vpp output)

    mfxU32 m_MaxSurfacesNum;
    mfxU32 m_CurSurfacesNum;

    mfxU32 m_nNumView;
    mfxU32 m_nEncNo;//表示编码路数，第几路
    mfxF64 m_nEncFrameRate;//表示编码帧频

    // for disabling VPP algorithms
    mfxExtVPPDoNotUse m_VppDoNotUse;
    // for MVC encoder and VPP configuration
    mfxExtMVCSeqDesc m_MVCSeqDesc;
    mfxExtCodingOption m_CodingOption;
    // for look ahead BRC configuration
    mfxExtCodingOption2 m_CodingOption2;
    // HEVC
    mfxExtHEVCParam m_ExtHEVCParam;

    // external parameters for each component are stored in a vector
    std::vector<mfxExtBuffer*> m_EncExtParams;

    CHWDevice *m_hwdev;

    virtual mfxStatus InitMfxEncParams(sInputParams *pParams);
    virtual mfxStatus InitMfxVppParams(sInputParams *pParams);

    virtual mfxStatus InitFileWriters(sInputParams *pParams);
    virtual mfxStatus InitEncSendParam(CSmplBitstreamWriter **ppWriter, sInputParams *pParams);
    virtual void FreeFileWriters();
    virtual mfxStatus InitFileWriter(CSmplBitstreamWriter **ppWriter, const msdk_char *filename);

    virtual mfxStatus AllocAndInitVppDoNotUse();
    virtual void FreeVppDoNotUse();

    virtual mfxStatus AllocAndInitMVCSeqDesc();
    virtual void FreeMVCSeqDesc();

    virtual mfxStatus CreateAllocator();
    virtual void DeleteAllocator();

    virtual mfxStatus CreateHWDevice();
    virtual void DeleteHWDevice();

    virtual mfxStatus AllocFrames();
    virtual mfxStatus AllocManageBuff();
    virtual void DeleteFrames();
    virtual void DeleteManageBuff();

    virtual mfxStatus AllocateSufficientBuffer(mfxBitstream* pBS);

    virtual mfxStatus GetFreeTask(sTask **ppTask,mfxBitstream * pBitBuff);
    virtual MFXVideoSession& GetFirstSession(){return m_mfxSession;}
    virtual MFXVideoENCODE* GetFirstEncoder(){return m_pmfxENC;}
};

#endif // __PIPELINE_ENCODE_H__
