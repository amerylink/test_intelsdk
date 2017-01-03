#pragma once
#include <d3d9.h>
#include <dxva2api.h>
#include <vector>
#include <memory>
#include <d3d11.h>
#include "sample_defs.h"
#include "base_allocator.h"
#include "hw_device.h"
#include "d3d_device.h"
#include "d3d_allocator.h"
#include "d3d11_allocator.h"
#include "d3d11_device.h"
#include "sysmem_allocator.h"
#include "mfx_buffering.h"
#include "mfxmvc.h"
#include <mfxplugin++.h>
#include <mfxvp8.h>
#include <mfxla.h>
#include "general_allocator.h"
#include "mfx_multi_vpp.h"

#include "comm_func.h"
#include "comm_def.h"

class CDecodingPipe : public CBuffering
{
public:
	CDecodingPipe();
	virtual ~CDecodingPipe();

	mfxStatus Init(sInputParams& InputParams);
	void Set_decoder_tex2d_callback(intelmedia_decoder_tex2d_callback _cb, void* reserve);
	void Release();
	mfxStatus Connect(sStrQue* conn_handle);

private:
	static void ThreadDecLoop(LPARAM lParam);
	mfxStatus Loop();

	mfxStatus FormatParam(sInputParams& InputParams);
	mfxStatus CreateAllocator();
	mfxStatus InitSession();
	mfxStatus CheckRequiredAPIVersion(mfxVersion& version);
	mfxStatus DecodePreInit();//Decode component initialization
	mfxStatus InitDecMfxParams();
	mfxStatus AllocMVCSeqDesc();
	mfxStatus VPPPreInit();//VPP component initialization
	mfxStatus InitVppMfxParams();
	mfxStatus AllocAndInitVppDoNotUse();
	mfxStatus InitPluginMfxParams();
	mfxStatus PreEncPreInit();//LA component initialization
	mfxStatus InitPreEncMfxParams();
	mfxStatus EncodePreInit();//Encode component initialization
	mfxStatus InitEncMfxParams();
	mfxStatus AllocFrames();
	mfxStatus AllocFrames(mfxFrameAllocRequest  *pRequest, bool isDecAlloc);
	mfxStatus CalculateNumberOfReqFrames(mfxFrameAllocRequest  &pRequestDecOut, mfxFrameAllocRequest  &pRequestVPPOut);
	mfxStatus AllocPreEncAuxPool();
	mfxStatus InitOpaqueAllocBuffers();
	mfxStatus AllocateSufficientBuffer(mfxBitstream* pBS);
	mfxStatus PutBS();

	mfxStatus DecodeOneFrame(ExtendedSurface *pExtSurface);
	mfxFrameSurface1* GetFreeSurface(bool isDec, mfxU64 timeout);
	mfxStatus DecodeLastFrame(ExtendedSurface *pExtSurface);
	mfxStatus VPPOneFrame(ExtendedSurface *pSurfaceIn, ExtendedSurface *pExtSurface);
	mfxStatus EncodeOneFrame(ExtendedSurface *pExtSurface, mfxBitstream *pBS);

private:
	sInputParams*                     m_pInputParams;
	std::auto_ptr<mfxAllocatorParams> m_pAllocParam;
	std::auto_ptr<GeneralAllocator>   m_pGeneralAllocator;
	std::auto_ptr<CHWDevice>          m_hwdev;
	std::auto_ptr<MFXVideoSession>    m_pmfxSession;
	std::auto_ptr<MFXVideoDECODE>     m_pmfxDEC;
	std::auto_ptr<MFXVideoENCODE>     m_pmfxENC;
	std::auto_ptr<MFXVideoMultiVPP>   m_pmfxVPP; // either VPP or VPPPlugin which wraps [VPP]-Plugin-[VPP] pipeline
	std::auto_ptr<MFXVideoENC>        m_pmfxPreENC;

	std::auto_ptr<MFXVideoUSER>       m_pUserDecoderModule;
	std::auto_ptr<MFXPlugin>          m_pUserDecoderPlugin;
	std::auto_ptr<MFXPlugin>          m_pUserEncPlugin;
	std::auto_ptr<MFXVideoUSER>       m_pUserEncoderModule;
	std::auto_ptr<MFXPlugin>          m_pUserEncoderPlugin;

	mfxVideoParam                     m_mfxDecParams;
	mfxVideoParam                     m_mfxEncParams;
	mfxVideoParam                     m_mfxVppParams;
	mfxVideoParam                     m_mfxPluginParams;
	mfxVideoParam					  m_mfxPreEncParams;
	// external parameters for each component are stored in a vector
	std::vector<mfxExtBuffer*>        m_DecExtParams;
	std::vector<mfxExtBuffer*>        m_VppExtParams;
	std::vector<mfxExtBuffer*>        m_PluginExtParams;
	std::vector<mfxExtBuffer*>        m_PreEncExtParams;
	std::vector<mfxExtBuffer*>        m_EncExtParams;
	// for opaque memory
	mfxExtOpaqueSurfaceAlloc          m_DecOpaqueAlloc;
	mfxExtOpaqueSurfaceAlloc          m_VppOpaqueAlloc;
	mfxExtOpaqueSurfaceAlloc          m_PluginOpaqueAlloc;
	mfxExtOpaqueSurfaceAlloc          m_PreEncOpaqueAlloc;
	mfxExtOpaqueSurfaceAlloc          m_EncOpaqueAlloc;

	mfxExtMVCSeqDesc                  m_MVCSeqDesc;// for MVC decoder and encoder configuration
	mfxExtVPPDoNotUse                 m_VppDoNotUse;// for disabling VPP algorithms
	mfxExtVPPComposite                m_VppCompParams;
	mfxExtVPPDenoise                  m_VppDenoiseParams;
	mfxExtVPPDetail                   m_VppDetailParams;
	mfxExtVPPFrameRateConversion      m_VppFrameRateConversionParams;
	mfxExtVPPDeinterlacing            m_VppDeinterlaceParams;
	mfxExtVPPFieldProcessing          m_VppFieldProcessParams;

	mfxExtLAControl                   m_ExtLAControl;
	// HEVC
	mfxExtHEVCParam                   m_ExtHEVCParam;
	// for setting MaxSliceSize
	mfxExtCodingOption2               m_CodingOption2;
	mfxExtCodingOption3               m_CodingOption3;

	mfxFrameAllocRequest              m_Request;
	mfxFrameAllocResponse             m_mfxDecResponse;  // memory allocation response for decoder
	mfxFrameAllocResponse             m_mfxEncResponse;  // memory allocation response for encoder

	mfxI64                            m_nReqFrameTime;// time required to transcode one frame
	bool                              m_bIsVpp; // true if there's VPP in the pipeline
	bool                              m_bIsPlugin; //true if there's Plugin in the pipeline
	mfxU32                            m_nVPPCompEnable;
	FileBitstreamProcessor            m_BSProcesser;
	mfxU32                            m_numEncoders;
	std::auto_ptr<ExtendedBSStore>    m_pBSStore;
	mfxVersion                        m_Version; // real API version with which library is initialized
	bool                              m_bUseOpaqueMemory; // indicates if opaque memory is used in the pipeline
	mfxBitstream*                     m_pmfxBS;  // contains encoded input data
	mfxU16                            m_EncSurfaceType; // actual type of encoder surface pool
	mfxU16                            m_DecSurfaceType; // actual type of decoder surface pool
	std::vector<mfxFrameSurface1*>    m_pSurfaceDecPool;
	std::vector<mfxFrameSurface1*>    m_pSurfaceEncPool;
	std::vector<PreEncAuxBuffer>      m_pPreEncAuxPool;
	bool                              m_bExit;
	mfxSyncPoint                      m_LastDecSyncPoint;
	std::list<ExtendedBS*>            m_BSPool;

	sStrQue*                m_listInput;//关联进来的数据
	sStrQueExt              m_listLoop;
public:
	sStrQueEx               m_listOut;
};

