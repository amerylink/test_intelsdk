//#pragma once
//#include "comm_def.h"
//#include "sample_defs.h"
//#include "base_allocator.h"
//#include "hw_device.h"
//#include "d3d_device.h"
//#include "d3d_allocator.h"
//#include "d3d11_allocator.h"
//#include "d3d11_device.h"
//#include "sysmem_allocator.h"
//
//#include <mfxvideo++.h>
//#include <mfxplugin++.h>
//#include <mfxmvc.h>
//
//class CEncodingPipe
//{
//public:
//	CEncodingPipe();
//	virtual ~CEncodingPipe();
//
//	mfxStatus Init(sInputParams_Enc *pParams);
//	void Release();
//	mfxStatus Connect(sStrQue* conn_handle, SourceType type = SourceType_Bitstream);
//
//
//private:
//	static void ThreadEncLoop(LPARAM lParam);
//	void EncodeLoop();
//	mfxStatus CreateAllocator();
//	mfxStatus CreateHWDevice();
//	mfxStatus InitMfxParams();
//	mfxStatus InitEncoder();
//	mfxStatus InitMfxVppParams();
//	mfxStatus AllocAndInitVppDoNotUse();
//	mfxStatus AllocAndInitMVCSeqDesc();
//	mfxStatus ResetComponents(bool alloc = false);
//	mfxStatus AllocFrames();
//	void DeleteFrames();
//
//	void CopySurfaceToBitstream(mfxFrameSurface1& surface, mfxBitstream& bitstream);
//	mfxStatus RunEncoding(mfxFrameSurface1* surface);
//	mfxStatus sclrLink_sclrProcess(mfxFrameSurface1 *src, mfxFrameSurface1 *dst);
//
//
//private:
//	MFXVideoSession*		    m_mfxSession;
//	MFXVideoENCODE*             m_pmfxEnc;
//	mfxVideoParam               m_mfxVideoParams;
//	mfxFrameSurface1*           m_pSurfaces; // frames array for encoder input (vpp output)
//	mfxFrameAllocResponse       m_Response;  // memory allocation response for encoder
//
//	MFXVideoVPP*                m_pmfxVPP;
//	mfxVideoParam               m_mfxVppParams;
//	mfxFrameSurface1*           m_pVppSurfaces; // frames array for vpp input
//	mfxFrameAllocResponse       m_VppResponse;  // memory allocation response for vpp
//
//	MFXFrameAllocator*          m_pMFXAllocator;
//	mfxAllocatorParams*         m_pmfxAllocatorParams;
//	CHWDevice*                  m_hwdev;
//
//	std::auto_ptr<MFXVideoUSER> m_pUserModule;
//	std::auto_ptr<MFXPlugin>    m_pPlugin;
//	sInputParams_Enc*           m_inputParams;
//	std::vector<mfxExtBuffer*>  m_EncExtParams;
//	std::vector<mfxExtBuffer*>  m_VppExtParams;
//	//多视图就是输出多路流 涉及到vpp操作 暂时不管mvc
//	mfxExtVPPDoNotUse           m_VppDoNotUse;// for disabling VPP algorithms
//	mfxExtMVCSeqDesc            m_MVCSeqDesc;// for MVC encoder and VPP configuration
//	/*------------------------------------------*/
//	mfxExtCodingOption          m_CodingOption; 
//	mfxExtCodingOption2         m_CodingOption2;// for look ahead BRC configuration
//	mfxExtHEVCParam             m_ExtHEVCParam;// HEVC
//	bool                        m_bExternalAlloc; // use memory allocator as external for Media SDK
//	bool                        m_bInitEncode;
//	bool                        m_bExit;
//	HANDLE                      m_handleThread;
//
//	sStrQue*                    m_listInput;//关联进来的数据
//	SourceType                  m_listInput_Type;
//
//	sStrQueExt                  m_listLoop;//vpp的需要再定义一个queloop
//public:
//	sStrQueEx                   m_listOut;
//};
//
