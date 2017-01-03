#include "stdafx.h"
#include "pipe_decode.h"
#include "plugin_utils.h"
#include "plugin_loader.h"
#include "vm/strings_defs.h"
#include "mfx_vpp_plugin.h"
#include <conio.h>

void CDecodingPipe::ThreadDecLoop(LPARAM lParam)
{
	((CDecodingPipe*)lParam)->Loop();
}

CDecodingPipe::CDecodingPipe()
{
	m_pInputParams = NULL;
	m_numEncoders = 0;
	m_bUseOpaqueMemory = false;
	m_pmfxBS = NULL;
	m_bIsVpp = false;
	m_bIsPlugin = false;
	m_nVPPCompEnable = 0;
	m_EncSurfaceType = 0;
	m_DecSurfaceType = 0;
	m_bExit = false;
	m_LastDecSyncPoint = NULL;

	MSDK_ZERO_MEMORY(m_mfxDecParams);
	MSDK_ZERO_MEMORY(m_mfxVppParams);
	MSDK_ZERO_MEMORY(m_mfxEncParams);
	MSDK_ZERO_MEMORY(m_mfxPluginParams);
	//MSDK_ZERO_MEMORY(m_RotateParam);
	MSDK_ZERO_MEMORY(m_mfxPreEncParams);

	MSDK_ZERO_MEMORY(m_mfxDecResponse);
	MSDK_ZERO_MEMORY(m_mfxEncResponse);

	MSDK_ZERO_MEMORY(m_Request);

	MSDK_ZERO_MEMORY(m_VppDoNotUse);
	MSDK_ZERO_MEMORY(m_MVCSeqDesc);
	MSDK_ZERO_MEMORY(m_EncOpaqueAlloc);
	MSDK_ZERO_MEMORY(m_VppOpaqueAlloc);
	MSDK_ZERO_MEMORY(m_DecOpaqueAlloc);
	MSDK_ZERO_MEMORY(m_PluginOpaqueAlloc);
	MSDK_ZERO_MEMORY(m_PreEncOpaqueAlloc);
	//MSDK_ZERO_MEMORY(m_ExtLAControl);
	MSDK_ZERO_MEMORY(m_CodingOption2);
	MSDK_ZERO_MEMORY(m_CodingOption3);

	m_MVCSeqDesc.Header.BufferId = MFX_EXTBUFF_MVC_SEQ_DESC;
	m_MVCSeqDesc.Header.BufferSz = sizeof(mfxExtMVCSeqDesc);

	m_VppDoNotUse.Header.BufferId = MFX_EXTBUFF_VPP_DONOTUSE;
	m_VppDoNotUse.Header.BufferSz = sizeof(mfxExtVPPDoNotUse);

	m_ExtHEVCParam.Header.BufferId = MFX_EXTBUFF_HEVC_PARAM;
	m_ExtHEVCParam.Header.BufferSz = sizeof(mfxExtHEVCParam);

	m_EncOpaqueAlloc.Header.BufferId = MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION;
	m_VppOpaqueAlloc.Header.BufferId = MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION;
	m_DecOpaqueAlloc.Header.BufferId = MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION;
	m_PluginOpaqueAlloc.Header.BufferId = MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION;
	m_PreEncOpaqueAlloc.Header.BufferId = MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION;

	m_EncOpaqueAlloc.Header.BufferSz = sizeof(mfxExtOpaqueSurfaceAlloc);
	m_VppOpaqueAlloc.Header.BufferSz = sizeof(mfxExtOpaqueSurfaceAlloc);
	m_DecOpaqueAlloc.Header.BufferSz = sizeof(mfxExtOpaqueSurfaceAlloc);
	m_PluginOpaqueAlloc.Header.BufferSz = sizeof(mfxExtOpaqueSurfaceAlloc);
	m_PreEncOpaqueAlloc.Header.BufferSz = sizeof(mfxExtOpaqueSurfaceAlloc);

	m_VppCompParams.InputStream = NULL;
	m_CodingOption2.Header.BufferId = MFX_EXTBUFF_CODING_OPTION2;
	m_CodingOption2.Header.BufferSz = sizeof(m_CodingOption2);

	m_CodingOption3.Header.BufferId = MFX_EXTBUFF_CODING_OPTION3;
	m_CodingOption3.Header.BufferSz = sizeof(m_CodingOption3);

	m_VppCompParams.InputStream = NULL;


	m_listInput = NULL;
	//AllocConsole();
	//freopen("CONOUT$", "w", stdout);
}

CDecodingPipe::~CDecodingPipe()
{
	//FreeConsole();
	Release();
}

void CDecodingPipe::Set_decoder_tex2d_callback(intelmedia_decoder_tex2d_callback _cb, void* reserve)
{
}

mfxStatus CDecodingPipe::FormatParam(sInputParams& InputParams)
{
	InputParams.bUseHW = true;
	InputParams.bUseOpaqueMemory = true;
	InputParams.eModeExt = Native;
	InputParams.DecodeId = MFX_CODEC_AVC;
	InputParams.EncodeId = MFX_CODEC_MPEG2;
	InputParams.nAsyncDepth = 2;
	InputParams.nDstWidth = 360;
	InputParams.nDstHeight = 240;
	StrCpy(InputParams.strSrcFile, _T("C:/Users/XJ/Desktop/test.h264"));
	StrCpy(InputParams.strDstFile, _T("C:/Users/XJ/Desktop/test1.mpeg2"));
	InputParams.MemType = enmMemType_D3D11;
	InputParams.nTargetUsage = MFX_TARGETUSAGE_BALANCED;

	switch (InputParams.DecodeId)
	{
	case MFX_CODEC_MPEG2:
	case MFX_CODEC_AVC:
	case MFX_CODEC_HEVC:
	case MFX_CODEC_VC1:
	case CODEC_MVC:
	case MFX_CODEC_JPEG:
	case MFX_CODEC_VP8:
	case MFX_CODEC_CAPTURE:
		break;
	default:
		return MFX_ERR_UNSUPPORTED;
	}

	if (InputParams.DecodeId == CODEC_MVC)
	{
		InputParams.DecodeId = MFX_CODEC_AVC;
		InputParams.bIsMVC = true;
	}
	if (InputParams.EncodeId == CODEC_MVC)
	{
		InputParams.EncodeId = MFX_CODEC_AVC;
		InputParams.bIsMVC = true;
	}
	if (InputParams.nTargetUsage<MFX_TARGETUSAGE_BEST_QUALITY || InputParams.nTargetUsage>MFX_TARGETUSAGE_BEST_SPEED)
		InputParams.nTargetUsage = MFX_TARGETUSAGE_BALANCED;
	// Ignoring user-defined Async Depth for LA
	InputParams.nAsyncDepth = InputParams.nMaxSliceSize > 0 ? 1 : InputParams.nAsyncDepth;
	if (InputParams.bLABRC && !InputParams.bUseHW)
	{
		msdk_printf(MSDK_STRING("Look ahead BRC is supported only with -hw option!\n"));
		return MFX_ERR_UNSUPPORTED;
	}
	if (InputParams.bLABRC && (InputParams.EncodeId != MFX_CODEC_AVC) && (InputParams.eMode == Source))
	{
		msdk_printf(MSDK_STRING("Look ahead BRC is supported only with H.264 encoder!\n"));
		return MFX_ERR_UNSUPPORTED;
	}
	if (InputParams.nLADepth && (InputParams.nLADepth < 10 || InputParams.nLADepth > 100))
	{
		if ((InputParams.nLADepth != 1) || (!InputParams.nMaxSliceSize))
		{
			msdk_printf(MSDK_STRING("Unsupported value of -lad parameter, must be in range [10, 100] or 1 in case of -mss option!\n"));
			return MFX_ERR_UNSUPPORTED;
		}
	}
	InputParams.nRateControlMethod = InputParams.nRateControlMethod == 0 ? MFX_RATECONTROL_CBR : InputParams.nRateControlMethod;
	if ((InputParams.nMaxSliceSize) && !InputParams.bUseHW)
	{
		msdk_printf(MSDK_STRING("MaxSliceSize option is supported only with -hw option!\n"));
		return MFX_ERR_UNSUPPORTED;
	}
	if ((InputParams.nMaxSliceSize) && (InputParams.nSlices))
	{
		msdk_printf(MSDK_STRING("-mss and -l options are not compatible!\n"));
	}
	if ((InputParams.nMaxSliceSize) && (InputParams.EncodeId != MFX_CODEC_AVC))
	{
		msdk_printf(MSDK_STRING("MaxSliceSize option is supported only with H.264 encoder!\n"));
		return MFX_ERR_UNSUPPORTED;
	}
	if (InputParams.enableQSVFF && InputParams.eMode == Sink)
	{
		msdk_printf(MSDK_STRING("WARNING: -qsv-ff option is not valid for decoder-only sessions, this parameter will be ignored.\n"));
	}
	//插件没处理   IsPluginCodecSupported
	if (InputParams.EncoderFourCC && InputParams.eMode == Sink)
	{
		msdk_printf(MSDK_STRING("WARNING: -ec option is used in session without encoder, this parameter will be ignored \n"));
	}

	if (InputParams.DecoderFourCC && InputParams.eMode != Native && InputParams.eMode != Sink)
	{
		msdk_printf(MSDK_STRING("WARNING: -dc option is used in session without decoder, this parameter will be ignored \n"));
	}

	if (InputParams.FRCAlgorithm && InputParams.eMode == Sink)
	{
		msdk_printf(NULL, MSDK_STRING("-FRC option should not be used in -o::sink pipelines \n"));
		return MFX_ERR_UNSUPPORTED;
	}
	if (InputParams.EncoderFourCC && InputParams.EncoderFourCC != MFX_FOURCC_NV12 &&
		InputParams.EncoderFourCC != MFX_FOURCC_RGB4 && InputParams.EncoderFourCC != MFX_FOURCC_YUY2 &&
		InputParams.EncodeId == MFX_FOURCC_DUMP)
	{
		msdk_printf(MSDK_STRING("-o::raw option can be used with NV12, RGB4 and YUY2 color formats only.\n"));
		return MFX_ERR_UNSUPPORTED;
	}
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::Init(sInputParams& InputParams)
{
	mfxStatus sts = MFX_ERR_NONE;
	sts = FormatParam(InputParams);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	m_pInputParams = &InputParams;
	sts = m_BSProcesser.Init(m_pInputParams->strSrcFile, m_pInputParams->strDstFile);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	m_numEncoders = 0;
	m_pBSStore.reset(new ExtendedBSStore(InputParams.nAsyncDepth));
	sts = InitSession();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	sts = CreateAllocator();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	// Initialize pipeline components following downstream direction
	// Pre-init methods fill parameters and create components
	sts = DecodePreInit();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	sts = VPPPreInit();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	sts = PreEncPreInit();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	// Encode component initialization
	if ((m_nVPPCompEnable != VppCompOnly) || (m_nVPPCompEnable == VppCompOnlyEncode))
	{
		sts = EncodePreInit();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
	sts = AllocFrames();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// common session settings
	if (m_Version.Major >= 1 && m_Version.Minor >= 1)
		sts = m_pmfxSession->SetPriority(m_pInputParams->priority);

	// after surfaces arrays are allocated configure mfxOpaqueAlloc buffers to be passed to components' Inits
	if (m_bUseOpaqueMemory)
	{
		sts = InitOpaqueAllocBuffers();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}

	// Init decode
	if (m_pmfxDEC.get())
	{
		sts = m_pmfxDEC->Init(&m_mfxDecParams);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
	// Init VPP
	if (m_pmfxVPP.get())
	{
		if (m_bIsPlugin && m_bIsVpp)
			sts = m_pmfxVPP->Init(&m_mfxPluginParams, &m_mfxVppParams);
		else if (m_bIsPlugin)
			sts = m_pmfxVPP->Init(&m_mfxPluginParams);
		else
			sts = m_pmfxVPP->Init(&m_mfxVppParams);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
	// LA initialization
	if (m_pmfxPreENC.get())
	{
		sts = m_pmfxPreENC->Init(&m_mfxEncParams);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
	// Init encode
	if (m_pmfxENC.get())
	{
		sts = m_pmfxENC->Init(&m_mfxEncParams);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
	PrintInfo(0, m_pInputParams, &m_Version);
	CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadDecLoop, (PVOID)this, 0, NULL));
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::InitOpaqueAllocBuffers()
{
	if (m_pmfxDEC.get() || (m_bUseOpaqueMemory && (m_nVPPCompEnable == VppComp || m_nVPPCompEnable == VppCompOnly ||
		m_nVPPCompEnable == VppCompOnlyEncode) && m_pSurfaceDecPool.size()))
	{
		m_DecOpaqueAlloc.Out.Surfaces = &m_pSurfaceDecPool[0]; // vestor is stored linearly in memory
		m_DecOpaqueAlloc.Out.NumSurface = (mfxU16)m_pSurfaceDecPool.size();
		m_DecOpaqueAlloc.Out.Type = (mfxU16)(MFX_MEMTYPE_BASE(m_DecSurfaceType) | MFX_MEMTYPE_FROM_DECODE);
	}
	else
	{
		// if no decoder in the pipeline we need to query m_DecOpaqueAlloc structure from parent sink pipeline
		//m_DecOpaqueAlloc = m_pParentPipeline->GetDecOpaqueAlloc();
	}

	if (m_pmfxVPP.get())
	{
		m_EncOpaqueAlloc.In.Surfaces = &m_pSurfaceEncPool[0];
		m_EncOpaqueAlloc.In.NumSurface = (mfxU16)m_pSurfaceEncPool.size();
		m_EncOpaqueAlloc.In.Type = (mfxU16)(MFX_MEMTYPE_BASE(m_EncSurfaceType) | MFX_MEMTYPE_FROM_ENCODE);

		// decode will be connected with either VPP or Plugin
		if (m_bIsVpp)
		{
			m_VppOpaqueAlloc.In = m_DecOpaqueAlloc.Out;
		}
		else if (m_bIsPlugin)
		{
			m_PluginOpaqueAlloc.In = m_DecOpaqueAlloc.Out;
		}
		else
			return MFX_ERR_UNSUPPORTED;

		// encode will be connected with either Plugin or VPP
		if (m_bIsPlugin)
			m_PluginOpaqueAlloc.Out = m_EncOpaqueAlloc.In;
		else if (m_bIsVpp)
			m_VppOpaqueAlloc.Out = m_EncOpaqueAlloc.In;
		else
			return MFX_ERR_UNSUPPORTED;
	}
	else if (m_pmfxENC.get() || m_pmfxPreENC.get())
		m_EncOpaqueAlloc.In = m_DecOpaqueAlloc.Out;
	if (m_pmfxPreENC.get())
		m_PreEncOpaqueAlloc.In = m_EncOpaqueAlloc.In;
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::AllocFrames()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	bool bAddFrames = true;   // correct shared pool between session
	mfxFrameAllocRequest DecOut;
	mfxFrameAllocRequest VPPOut;
	MSDK_ZERO_MEMORY(DecOut);
	MSDK_ZERO_MEMORY(VPPOut);
	sts = CalculateNumberOfReqFrames(DecOut, VPPOut);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	if (VPPOut.NumFrameSuggested)
	{
		if (bAddFrames)
		{
			SumAllocRequest(VPPOut, m_Request);
			bAddFrames = false;
		}

		// Do not correct anything if we're using raw output - we'll need those surfaces for storing data for writer
		if (m_mfxEncParams.mfx.CodecId != MFX_FOURCC_DUMP)
		{
			// In case of rendering enabled we need to add 1 additional surface for renderer
			if ((m_nVPPCompEnable == VppCompOnly) || (m_nVPPCompEnable == VppCompOnlyEncode))
			{
				VPPOut.NumFrameSuggested++;
				VPPOut.NumFrameMin++;
			}
			sts = CorrectAsyncDepth(VPPOut, m_pInputParams->nAsyncDepth);
		}
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		sts = AllocFrames(&VPPOut, false);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}

	if (DecOut.NumFrameSuggested)
	{
		if (bAddFrames)
		{
			SumAllocRequest(DecOut, m_Request);
			bAddFrames = false;
		}

		//if (m_bDecodeEnable)
		{
			if (0 == m_nVPPCompEnable && m_mfxEncParams.mfx.CodecId != MFX_FOURCC_DUMP)
			{
				//--- Make correction to number of surfaces only if composition is not enabled. In case of composition we need all the surfaces QueryIOSurf has requested to pass them to another session's VPP
				// In other inter-session cases, other sessions request additional surfaces using additional calls to AllocFrames
				sts = CorrectAsyncDepth(DecOut, m_pInputParams->nAsyncDepth);
			}
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

			// AllocId just opaque handle which allow separate decoder requests in case of VPP Composition with external allocator
			static mfxU32 mark_alloc = 0;
			m_mfxDecParams.AllocId = mark_alloc;
			DecOut.AllocId = mark_alloc;
			if (m_nVPPCompEnable) // WORKAROUND: Remove this if clause after problem with AllocID is fixed in library (mark_alloc++ should be left here)
			{
				mark_alloc++;
			}

			sts = AllocFrames(&DecOut, true);
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			if (m_pmfxPreENC.get())
			{
				mfxU16 aux_size = (VPPOut.NumFrameSuggested ? VPPOut.NumFrameSuggested : DecOut.NumFrameSuggested) + m_pInputParams->nAsyncDepth;
				if (m_pPreEncAuxPool.size()<aux_size)
					m_pPreEncAuxPool.resize(aux_size);
			}
			sts = AllocPreEncAuxPool();
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}
		//else if ((m_nVPPCompEnable == VppComp || m_nVPPCompEnable == VppCompOnly || m_nVPPCompEnable == VppCompOnlyEncode) && m_bUseOpaqueMemory)
		//{
		//	//--- N->1 case, allocating empty pool for opaque only
		//	sts = AllocFrames(&DecOut, true);
		//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		//}
	}


	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::AllocPreEncAuxPool()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	MSDK_CHECK_POINTER(m_pmfxPreENC.get(), MFX_ERR_NONE);
	mfxU16 num_resolutions = m_ExtLAControl.NumOutStream;
	int buff_size = sizeof(mfxExtLAFrameStatistics)+sizeof(mfxLAFrameInfo)*num_resolutions*m_ExtLAControl.LookAheadDepth;

	for (size_t i = 0; i < m_pPreEncAuxPool.size(); i++)
	{
		memset(&m_pPreEncAuxPool[i], 0, sizeof(m_pPreEncAuxPool[i]));
		m_pPreEncAuxPool[i].encCtrl.NumExtParam = 1;
		m_pPreEncAuxPool[i].encCtrl.ExtParam = new mfxExtBuffer*[1];

		char *pBuff = new char[buff_size];
		memset(pBuff, 0, buff_size);

		m_pPreEncAuxPool[i].encCtrl.ExtParam[0] = (mfxExtBuffer *)pBuff;
		mfxExtLAFrameStatistics *pExtBuffer = (mfxExtLAFrameStatistics *)pBuff;

		pExtBuffer = (mfxExtLAFrameStatistics*)pBuff;
		pExtBuffer->Header.BufferId = MFX_EXTBUFF_LOOKAHEAD_STAT;
		pExtBuffer->Header.BufferSz = buff_size;
		pExtBuffer->NumAlloc = num_resolutions*m_ExtLAControl.LookAheadDepth;
		pExtBuffer->FrameStat = (mfxLAFrameInfo *)(pBuff + sizeof(mfxExtLAFrameStatistics));

		m_pPreEncAuxPool[i].encOutput.NumExtParam = 1;
		m_pPreEncAuxPool[i].encOutput.ExtParam = m_pPreEncAuxPool[i].encCtrl.ExtParam;
	}
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::AllocFrames(mfxFrameAllocRequest *pRequest, bool isDecAlloc)
{
	mfxStatus sts = MFX_ERR_NONE;
	mfxU16 nSurfNum = 0; // number of surfaces
	mfxU16 i;
	nSurfNum = pRequest->NumFrameMin = pRequest->NumFrameSuggested;
	msdk_printf(MSDK_STRING("Pipeline surfaces number (%s): %d\n"), isDecAlloc ? MSDK_STRING("DecPool") : MSDK_STRING("EncPool"), nSurfNum);
	mfxFrameAllocResponse *pResponse = isDecAlloc ? &m_mfxDecResponse : &m_mfxEncResponse;
	// no actual memory is allocated if opaque memory type is used
	if (!m_bUseOpaqueMemory)
	{
		sts = m_pGeneralAllocator.get()->Alloc(m_pGeneralAllocator.get()->pthis, pRequest, pResponse);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
	for (i = 0; i < nSurfNum; i++)
	{
		mfxFrameSurface1 *surface = new mfxFrameSurface1;
		MSDK_CHECK_POINTER(surface, MFX_ERR_MEMORY_ALLOC);
		MSDK_ZERO_MEMORY(*surface);
		MSDK_MEMCPY_VAR(surface->Info, &(pRequest->Info), sizeof(mfxFrameInfo));
		// no actual memory is allocated if opaque memory type is used (surface pointers and MemId field remain 0)
		if (!m_bUseOpaqueMemory)
			surface->Data.MemId = pResponse->mids[i];
		(isDecAlloc) ? m_pSurfaceDecPool.push_back(surface) : m_pSurfaceEncPool.push_back(surface);
	}
	(isDecAlloc) ? m_DecSurfaceType = pRequest->Type : m_EncSurfaceType = pRequest->Type;
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::CalculateNumberOfReqFrames(mfxFrameAllocRequest  &pDecOut, mfxFrameAllocRequest  &pVPPOut)
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	mfxFrameAllocRequest *pSumRequest = &pDecOut;
	memset(&pDecOut, 0, sizeof(pDecOut));
	memset(&pVPPOut, 0, sizeof(pVPPOut));
	mfxFrameAllocRequest DecRequest;
	MSDK_ZERO_MEMORY(DecRequest);

	if (m_pmfxDEC.get())
	{
		sts = m_pmfxDEC.get()->QueryIOSurf(&m_mfxDecParams, &DecRequest);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		if (DecRequest.NumFrameSuggested < m_mfxDecParams.AsyncDepth)
			return MFX_ERR_MEMORY_ALLOC;
		SumAllocRequest(*pSumRequest, DecRequest);
	}
	if (m_pmfxVPP.get())
	{
		mfxFrameAllocRequest VppRequest[2];
		MSDK_ZERO_MEMORY(VppRequest);
		if (m_bIsPlugin && m_bIsVpp)
		{
			sts = m_pmfxVPP.get()->QueryIOSurf(&m_mfxPluginParams, &VppRequest[0], &m_mfxVppParams);
			if (VppRequest[0].NumFrameSuggested < m_mfxPluginParams.AsyncDepth ||
				VppRequest[1].NumFrameSuggested < m_mfxPluginParams.AsyncDepth ||
				VppRequest[0].NumFrameSuggested < m_mfxVppParams.AsyncDepth ||
				VppRequest[1].NumFrameSuggested < m_mfxVppParams.AsyncDepth)
				return MFX_ERR_MEMORY_ALLOC;
		}
		else if (m_bIsPlugin)
		{
			sts = m_pmfxVPP.get()->QueryIOSurf(&m_mfxPluginParams, &(VppRequest[0]));
			if (VppRequest[0].NumFrameSuggested < m_mfxPluginParams.AsyncDepth ||
				VppRequest[1].NumFrameSuggested < m_mfxPluginParams.AsyncDepth)
				return MFX_ERR_MEMORY_ALLOC;
		}
		else
		{
			sts = m_pmfxVPP.get()->QueryIOSurf(&m_mfxVppParams, &(VppRequest[0]));
			if (VppRequest[0].NumFrameSuggested < m_mfxVppParams.AsyncDepth ||
				VppRequest[1].NumFrameSuggested < m_mfxVppParams.AsyncDepth)
				return MFX_ERR_MEMORY_ALLOC;
		}

		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		SumAllocRequest(*pSumRequest, VppRequest[0]);
		pSumRequest = &pVPPOut;
		SumAllocRequest(*pSumRequest, VppRequest[1]);
	}
	if (m_pmfxPreENC.get())
	{
		mfxFrameAllocRequest PreEncRequest;
		MSDK_ZERO_MEMORY(PreEncRequest);
		sts = m_pmfxPreENC.get()->QueryIOSurf(&m_mfxPreEncParams, &PreEncRequest);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		if (PreEncRequest.NumFrameSuggested < m_mfxPreEncParams.AsyncDepth)
			return MFX_ERR_MEMORY_ALLOC;
		SumAllocRequest(*pSumRequest, PreEncRequest);
	}
	if (m_pmfxENC.get())
	{
		mfxFrameAllocRequest EncRequest;
		MSDK_ZERO_MEMORY(EncRequest);
		sts = m_pmfxENC.get()->QueryIOSurf(&m_mfxEncParams, &EncRequest);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		if (EncRequest.NumFrameSuggested < m_mfxEncParams.AsyncDepth)
			return MFX_ERR_MEMORY_ALLOC;
		SumAllocRequest(*pSumRequest, EncRequest);
	}

	if (!pSumRequest->Type && m_pmfxDEC.get())
	{
		//--- If noone has set type to VPP request type yet, set it now basing on decoder request type
		pSumRequest->Type = MFX_MEMTYPE_BASE(DecRequest.Type) | MFX_MEMTYPE_FROM_VPPOUT;
	}
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::EncodePreInit()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	if (m_pInputParams->EncodeId != MFX_FOURCC_DUMP)
	{
		if (CheckVersion(&m_Version, MSDK_FEATURE_PLUGIN_API) && (m_pUserEncPlugin.get() == NULL))
		{
			/* Here we actually define the following codec initialization scheme:
			*  1. If plugin path or guid is specified: we load user-defined plugin (example: HEVC encoder plugin)
			*  2. If plugin path not specified:
			*    2.a) we check if codec is distributed as a mediasdk plugin and load it if yes
			*    2.b) if codec is not in the list of mediasdk plugins, we assume, that it is supported inside mediasdk library
			*/
			if (m_pInputParams->encoderPluginParams.pluginType == MFX_PLUGINLOAD_TYPE_FILE && strnlen_s(m_pInputParams->encoderPluginParams.pluginPath, sizeof(m_pInputParams->encoderPluginParams.pluginPath)))
			{
				m_pUserEncoderModule.reset(new MFXVideoUSER(*m_pmfxSession.get()));
				m_pUserEncoderPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_ENCODE, *m_pmfxSession.get(), m_pInputParams->encoderPluginParams.pluginGuid, 1, m_pInputParams->encoderPluginParams.pluginPath, (mfxU32)strnlen_s(m_pInputParams->encoderPluginParams.pluginPath, sizeof(m_pInputParams->encoderPluginParams.pluginPath))));
				if (m_pUserEncoderPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
			}
			else
			{
				if (AreGuidsEqual(m_pInputParams->encoderPluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
					m_pInputParams->encoderPluginParams.pluginGuid = msdkGetPluginUID(m_pInputParams->bUseHW ? MFX_IMPL_HARDWARE_ANY : MFX_IMPL_SOFTWARE, MSDK_VENCODE, m_pInputParams->EncodeId);
				if (!AreGuidsEqual(m_pInputParams->encoderPluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
				{
					m_pUserEncoderPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_ENCODE, *m_pmfxSession.get(), m_pInputParams->encoderPluginParams.pluginGuid, 1));
					if (m_pUserEncoderPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
				}
				if (sts == MFX_ERR_UNSUPPORTED)
					msdk_printf(MSDK_STRING("Default plugin cannot be loaded (possibly you have to define plugin explicitly)\n"));
			}
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}
		// create encoder
		m_pmfxENC.reset(new MFXVideoENCODE(*m_pmfxSession.get()));
		if (m_nVPPCompEnable == VppCompOnlyEncode)
			m_pInputParams->EncoderFourCC = MFX_FOURCC_NV12;

		sts = InitEncMfxParams();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		// Querying parameters
		mfxU16 ioPattern = m_mfxEncParams.IOPattern;
		sts = m_pmfxENC->Query(&m_mfxEncParams, &m_mfxEncParams);
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
		m_mfxEncParams.IOPattern = ioPattern; // Workaround for a problem: Query changes IOPattern incorrectly
		MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, msdk_printf(MSDK_STRING("Encoder parameters query failed.\n")));
	}
	else
	{
		//--- This one is required for YUV output
		m_mfxEncParams.mfx.CodecId = m_pInputParams->EncodeId;
	}
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::InitEncMfxParams()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	m_mfxEncParams.mfx.CodecId = m_pInputParams->EncodeId;
	m_mfxEncParams.mfx.TargetUsage = m_pInputParams->nTargetUsage; // trade-off between quality and speed
	m_mfxEncParams.AsyncDepth = m_pInputParams->nAsyncDepth;
	m_mfxEncParams.mfx.RateControlMethod = m_pInputParams->nRateControlMethod;
	m_mfxEncParams.mfx.NumSlice = m_pInputParams->nSlices;

	if (m_pInputParams->nRateControlMethod == MFX_RATECONTROL_CQP)
	{
		m_mfxEncParams.mfx.QPI = m_pInputParams->nQPI;
		m_mfxEncParams.mfx.QPP = m_pInputParams->nQPP;
		m_mfxEncParams.mfx.QPB = m_pInputParams->nQPB;
	}
	if (m_pInputParams->enableQSVFF)
		m_mfxEncParams.mfx.LowPower = MFX_CODINGOPTION_ON;

	if (m_bIsVpp)
		MSDK_MEMCPY_VAR(m_mfxEncParams.mfx.FrameInfo, &m_mfxVppParams.vpp.Out, sizeof(mfxFrameInfo));
	else if (m_bIsPlugin)
		MSDK_MEMCPY_VAR(m_mfxEncParams.mfx.FrameInfo, &m_mfxPluginParams.vpp.Out, sizeof(mfxFrameInfo));
	else
		MSDK_MEMCPY_VAR(m_mfxEncParams.mfx.FrameInfo, &m_mfxDecParams.mfx.FrameInfo, sizeof(mfxFrameInfo));

	// leave PAR unset to avoid MPEG2 encoder rejecting streams with unsupported DAR
	m_mfxEncParams.mfx.FrameInfo.AspectRatioW = m_mfxEncParams.mfx.FrameInfo.AspectRatioH = 0;

	// calculate default bitrate based on resolution and framerate
	// set framerate if specified
	if (m_pInputParams->dEncoderFrameRate)
		ConvertFrameRate(m_pInputParams->dEncoderFrameRate, &m_mfxEncParams.mfx.FrameInfo.FrameRateExtN, &m_mfxEncParams.mfx.FrameInfo.FrameRateExtD);
	MSDK_CHECK_ERROR(m_mfxEncParams.mfx.FrameInfo.FrameRateExtN * m_mfxEncParams.mfx.FrameInfo.FrameRateExtD, 0, MFX_ERR_INVALID_VIDEO_PARAM);

	if (m_pInputParams->nRateControlMethod != MFX_RATECONTROL_CQP)
	{
		if (m_pInputParams->nBitRate == 0)
		{
			m_pInputParams->nBitRate = CalculateDefaultBitrate(m_pInputParams->EncodeId,
				m_pInputParams->nTargetUsage, m_mfxEncParams.mfx.FrameInfo.Width, m_mfxEncParams.mfx.FrameInfo.Height,
				1.0 * m_mfxEncParams.mfx.FrameInfo.FrameRateExtN / m_mfxEncParams.mfx.FrameInfo.FrameRateExtD);
		}
		m_mfxEncParams.mfx.TargetKbps = (mfxU16)(m_pInputParams->nBitRate); // in Kbps
	}

	// In case of HEVC when height and/or width divided with 8 but not divided with 16
	// add extended parameter to increase performance
	if ((!((m_mfxEncParams.mfx.FrameInfo.CropW & 15) ^ 8) ||
		!((m_mfxEncParams.mfx.FrameInfo.CropH & 15) ^ 8)) &&
		(m_mfxEncParams.mfx.CodecId == MFX_CODEC_HEVC))
	{
		m_ExtHEVCParam.PicWidthInLumaSamples = m_mfxEncParams.mfx.FrameInfo.CropW;
		m_ExtHEVCParam.PicHeightInLumaSamples = m_mfxEncParams.mfx.FrameInfo.CropH;
		m_EncExtParams.push_back((mfxExtBuffer*)&m_ExtHEVCParam);
	}

	m_mfxEncParams.mfx.FrameInfo.CropX = 0;
	m_mfxEncParams.mfx.FrameInfo.CropY = 0;
	mfxU16 InPatternFromParent = (mfxU16)((MFX_IOPATTERN_OUT_VIDEO_MEMORY == m_mfxDecParams.IOPattern) ? MFX_IOPATTERN_IN_VIDEO_MEMORY : MFX_IOPATTERN_IN_SYSTEM_MEMORY);

	// set memory pattern
	if (m_bUseOpaqueMemory)
		m_mfxEncParams.IOPattern = MFX_IOPATTERN_IN_OPAQUE_MEMORY;
	else
		m_mfxEncParams.IOPattern = InPatternFromParent;

	// we don't specify profile and level and let the encoder choose those basing on parameters
	// we must specify profile only for MVC codec
	if (m_pInputParams->bIsMVC)
		m_mfxEncParams.mfx.CodecProfile = m_mfxDecParams.mfx.CodecProfile;

	// JPEG encoder settings overlap nasc other encoders settings in mfxInfoMFX structure
	if (MFX_CODEC_JPEG == m_pInputParams->EncodeId)
	{
		m_mfxEncParams.mfx.Interleaved = 1;
		m_mfxEncParams.mfx.Quality = m_pInputParams->nQuality;
		m_mfxEncParams.mfx.RestartInterval = 0;
		MSDK_ZERO_MEMORY(m_mfxEncParams.mfx.reserved5);
	}
	// configure and attach external parameters
	if (m_pInputParams->bLABRC || m_pInputParams->nMaxSliceSize)
	{
		m_CodingOption2.LookAheadDepth = m_pInputParams->nLADepth;
		m_CodingOption2.MaxSliceSize = m_pInputParams->nMaxSliceSize;
		m_EncExtParams.push_back((mfxExtBuffer *)&m_CodingOption2);
	}
	if (m_pInputParams->WinBRCMaxAvgKbps || m_pInputParams->WinBRCSize)
	{
		m_CodingOption3.WinBRCMaxAvgKbps = m_pInputParams->WinBRCMaxAvgKbps;
		m_CodingOption3.WinBRCSize = m_pInputParams->WinBRCSize;
		m_EncExtParams.push_back((mfxExtBuffer *)&m_CodingOption3);
	}
	if (m_bUseOpaqueMemory)
		m_EncExtParams.push_back((mfxExtBuffer *)&m_EncOpaqueAlloc);
	if (m_pInputParams->bIsMVC)
		m_EncExtParams.push_back((mfxExtBuffer *)&m_MVCSeqDesc);

	if (!m_EncExtParams.empty())
	{
		m_mfxEncParams.ExtParam = &m_EncExtParams[0]; // vector is stored linearly in memory
		m_mfxEncParams.NumExtParam = (mfxU16)m_EncExtParams.size();
	}
	//if (m_pParentPipeline)
	//{
	//	m_pParentPipeline->AddLaStreams(m_mfxEncParams.mfx.FrameInfo.Width, m_mfxEncParams.mfx.FrameInfo.Height);
	//}

	//--- Settings HRD buffer size  
	// buffer for 4 seconds
	m_mfxEncParams.mfx.BufferSizeInKB = m_pInputParams->BufferSizeInKB ? m_pInputParams->BufferSizeInKB	: (mfxU16)(m_mfxEncParams.mfx.TargetKbps * 4L / 8); 
	//--- Force setting fourcc type if required
	if (m_pInputParams->EncoderFourCC)
	{
		m_mfxEncParams.mfx.FrameInfo.FourCC = m_pInputParams->EncoderFourCC;
		m_mfxEncParams.mfx.FrameInfo.ChromaFormat = FourCCToChroma(m_pInputParams->EncoderFourCC);
	}
	if (m_pInputParams->GopPicSize)
		m_mfxEncParams.mfx.GopPicSize = m_pInputParams->GopPicSize;
	if (m_pInputParams->GopRefDist)
		m_mfxEncParams.mfx.GopRefDist = m_pInputParams->GopRefDist;
	if (m_pInputParams->NumRefFrame)
		m_mfxEncParams.mfx.NumRefFrame = m_pInputParams->NumRefFrame;
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::PreEncPreInit()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	// PreInit is allowed in decoder session only
	if (m_pInputParams->bEnableExtLA /*&& m_bDecodeEnable*/)
	{
		/* Here we actually define the following codec initialization scheme:
		*    a) we check if codec is distributed as a user plugin and load it if yes
		*    b) we check if codec is distributed as a mediasdk plugin and load it if yes
		*    c) if codec is not in the list of user plugins or mediasdk plugins, we assume, that it is supported inside mediasdk library
		*/
		m_pUserEncPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_ENCODE, *m_pmfxSession.get(), MFX_PLUGINID_H264LA_HW, 1));
		MSDK_CHECK_POINTER(m_pUserEncPlugin.get(), MFX_ERR_UNSUPPORTED);

		// create encoder
		m_pmfxPreENC.reset(new MFXVideoENC(*m_pmfxSession.get()));
		sts = InitPreEncMfxParams();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::InitPreEncMfxParams()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	MSDK_CHECK_ERROR(m_pInputParams->bEnableExtLA, false, MFX_ERR_INVALID_VIDEO_PARAM);
	mfxStatus sts = MFX_ERR_NONE;
	m_mfxPreEncParams.AsyncDepth = m_pInputParams->nAsyncDepth;
	MSDK_ZERO_MEMORY(m_mfxPreEncParams.mfx);
	m_mfxPreEncParams.mfx.CodecId = MFX_CODEC_AVC;
	m_mfxPreEncParams.mfx.TargetUsage = m_pInputParams->nTargetUsage;
	if (m_bIsVpp)
		MSDK_MEMCPY_VAR(m_mfxPreEncParams.mfx.FrameInfo, &m_mfxVppParams.vpp.Out, sizeof(mfxFrameInfo));
	else if (m_bIsPlugin)
		MSDK_MEMCPY_VAR(m_mfxPreEncParams.mfx.FrameInfo, &m_mfxPluginParams.vpp.Out, sizeof(mfxFrameInfo));
	else
		MSDK_MEMCPY_VAR(m_mfxPreEncParams.mfx.FrameInfo, &m_mfxDecParams.mfx.FrameInfo, sizeof(mfxFrameInfo));
	mfxU16 InPatternFromParent = (mfxU16)((MFX_IOPATTERN_OUT_VIDEO_MEMORY == m_mfxDecParams.IOPattern) ? MFX_IOPATTERN_IN_VIDEO_MEMORY : MFX_IOPATTERN_IN_SYSTEM_MEMORY);

	// set memory pattern
	if (m_bUseOpaqueMemory)
		m_mfxPreEncParams.IOPattern = MFX_IOPATTERN_IN_OPAQUE_MEMORY;
	else
		m_mfxPreEncParams.IOPattern = InPatternFromParent;

	// configure and attach external parameters
	if (m_bUseOpaqueMemory)
		m_PreEncExtParams.push_back((mfxExtBuffer *)&m_PreEncOpaqueAlloc);

	MSDK_ZERO_MEMORY(m_ExtLAControl);
	m_ExtLAControl.Header.BufferId = MFX_EXTBUFF_LOOKAHEAD_CTRL;
	m_ExtLAControl.Header.BufferSz = sizeof(m_ExtLAControl);
	m_ExtLAControl.LookAheadDepth = m_pInputParams->nLADepth ? m_pInputParams->nLADepth : 40;
	m_ExtLAControl.NumOutStream = 0;
	m_ExtLAControl.BPyramid = (mfxU16)(m_pInputParams->bEnableBPyramid ? MFX_CODINGOPTION_ON : MFX_CODINGOPTION_OFF);
	m_PreEncExtParams.push_back((mfxExtBuffer *)&m_ExtLAControl);

	m_mfxPreEncParams.mfx.GopPicSize = m_pInputParams->GopPicSize ? m_pInputParams->GopPicSize : 1500;
	if (m_pInputParams->GopRefDist)
		m_mfxPreEncParams.mfx.GopRefDist = m_pInputParams->GopRefDist;
	if (m_pInputParams->nTargetUsage)
		m_mfxPreEncParams.mfx.TargetUsage = m_pInputParams->nTargetUsage;

	if (!m_PreEncExtParams.empty())
	{
		m_mfxPreEncParams.ExtParam = &m_PreEncExtParams[0]; // vector is stored linearly in memory
		m_mfxPreEncParams.NumExtParam = (mfxU16)m_PreEncExtParams.size();
	}
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::VPPPreInit()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	bool bVppCompInitRequire = ((m_pInputParams->eModeExt == VppComp || m_pInputParams->eModeExt == VppCompOnly) && m_pInputParams->eMode == Source) ? true : false;

	if ((m_mfxDecParams.mfx.FrameInfo.CropW != m_pInputParams->nDstWidth && m_pInputParams->nDstWidth) ||
		(m_mfxDecParams.mfx.FrameInfo.CropH != m_pInputParams->nDstHeight && m_pInputParams->nDstHeight) ||
		m_pInputParams->bEnableDeinterlacing || m_pInputParams->DenoiseLevel != -1 || 
		m_pInputParams->DetailLevel != -1 || m_pInputParams->FRCAlgorithm ||
		bVppCompInitRequire || m_pInputParams->fieldProcessingMode ||
		(m_pInputParams->EncoderFourCC && m_mfxDecParams.mfx.FrameInfo.FourCC && m_pInputParams->EncoderFourCC != m_mfxDecParams.mfx.FrameInfo.FourCC))
	{
		m_bIsVpp = true;
		sts = InitVppMfxParams();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
	if (m_pInputParams->nRotationAngle) // plugin was requested
	{
		m_bIsPlugin = true;
		sts = InitPluginMfxParams();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		std::auto_ptr<MFXVideoVPPPlugin> pVPPPlugin(new MFXVideoVPPPlugin(*m_pmfxSession.get()));
		MSDK_CHECK_POINTER(pVPPPlugin.get(), MFX_ERR_NULL_PTR);
		sts = pVPPPlugin->LoadDLL(m_pInputParams->strVPPPluginDLLPath);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		sts = pVPPPlugin->SetAuxParam(&m_pInputParams->nRotationAngle, sizeof(m_pInputParams->nRotationAngle));
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		if (!m_bUseOpaqueMemory)
		{
			sts = pVPPPlugin->SetFrameAllocator(m_pGeneralAllocator.get());
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}
		m_pmfxVPP.reset(pVPPPlugin.release());
	}

	if (!m_bIsPlugin && m_bIsVpp) // only VPP was requested
		m_pmfxVPP.reset(new MFXVideoMultiVPP(*m_pmfxSession.get()));
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::InitPluginMfxParams()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	mfxU16 parentPattern = m_bIsVpp ? m_mfxVppParams.IOPattern : m_mfxDecParams.IOPattern;
	mfxU16 InPatternFromParent = (MFX_IOPATTERN_OUT_VIDEO_MEMORY && parentPattern) ? MFX_IOPATTERN_IN_VIDEO_MEMORY : MFX_IOPATTERN_IN_SYSTEM_MEMORY;
	// set memory pattern
	if (m_bUseOpaqueMemory)
		m_mfxPluginParams.IOPattern = MFX_IOPATTERN_IN_OPAQUE_MEMORY | MFX_IOPATTERN_OUT_OPAQUE_MEMORY;
	else if (m_pInputParams->bForceSysMem || !m_pInputParams->bUseHW)
		m_mfxPluginParams.IOPattern = (mfxU16)(InPatternFromParent | MFX_IOPATTERN_OUT_SYSTEM_MEMORY);
	else
		m_mfxPluginParams.IOPattern = (mfxU16)(InPatternFromParent | MFX_IOPATTERN_OUT_VIDEO_MEMORY);
	m_mfxPluginParams.AsyncDepth = m_pInputParams->nAsyncDepth;
	// input frame info
	if (m_bIsVpp)
		MSDK_MEMCPY_VAR(m_mfxPluginParams.vpp.In, &m_mfxVppParams.vpp.Out, sizeof(mfxFrameInfo));
	else
		MSDK_MEMCPY_VAR(m_mfxPluginParams.vpp.In, &m_mfxDecParams.mfx.FrameInfo, sizeof(mfxFrameInfo));
	// fill output frame info
	// in case of rotation plugin sample output frameinfo is same as input
	MSDK_MEMCPY_VAR(m_mfxPluginParams.vpp.Out, &m_mfxPluginParams.vpp.In, sizeof(mfxFrameInfo));
	// configure and attach external parameters
	if (m_bUseOpaqueMemory)
		m_PluginExtParams.push_back((mfxExtBuffer *)&m_PluginOpaqueAlloc);
	if (!m_PluginExtParams.empty())
	{
		m_mfxPluginParams.ExtParam = &m_PluginExtParams[0]; // vector is stored linearly in memory
		m_mfxPluginParams.NumExtParam = (mfxU16)m_PluginExtParams.size();
	}
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::InitVppMfxParams()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	m_mfxVppParams.AsyncDepth = m_pInputParams->nAsyncDepth;
	mfxU16 InPatternFromParent = (mfxU16)((MFX_IOPATTERN_OUT_VIDEO_MEMORY == m_mfxDecParams.IOPattern) ?
	MFX_IOPATTERN_IN_VIDEO_MEMORY : MFX_IOPATTERN_IN_SYSTEM_MEMORY);
	// set memory pattern
	if (m_bUseOpaqueMemory)
		m_mfxVppParams.IOPattern = MFX_IOPATTERN_IN_OPAQUE_MEMORY | MFX_IOPATTERN_OUT_OPAQUE_MEMORY;
	else if (m_pInputParams->bForceSysMem || !m_pInputParams->bUseHW)
		m_mfxVppParams.IOPattern = (mfxU16)(InPatternFromParent | MFX_IOPATTERN_OUT_SYSTEM_MEMORY);
	else
		m_mfxVppParams.IOPattern = (mfxU16)(InPatternFromParent | MFX_IOPATTERN_OUT_VIDEO_MEMORY);
	// fill input frame info
	MSDK_MEMCPY_VAR(m_mfxVppParams.vpp.In, &m_mfxDecParams.mfx.FrameInfo, sizeof(mfxFrameInfo));
	// fill output frame info
	MSDK_MEMCPY_VAR(m_mfxVppParams.vpp.Out, &m_mfxVppParams.vpp.In, sizeof(mfxFrameInfo));
	
	if (m_pInputParams->bEnableDeinterlacing)
		m_mfxVppParams.vpp.Out.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;

	// Framerate conversion
	if (m_pInputParams->dEncoderFrameRate)
		ConvertFrameRate(m_pInputParams->dEncoderFrameRate, &m_mfxVppParams.vpp.Out.FrameRateExtN, &m_mfxVppParams.vpp.Out.FrameRateExtD);
	// Resizing
	if (m_pInputParams->nDstWidth)
	{
		m_mfxVppParams.vpp.Out.CropW = m_pInputParams->nDstWidth;
		m_mfxVppParams.vpp.Out.Width = MSDK_ALIGN16(m_pInputParams->nDstWidth);
	}
	if (m_pInputParams->nDstHeight)
	{
		m_mfxVppParams.vpp.Out.CropH = m_pInputParams->nDstHeight;
		m_mfxVppParams.vpp.Out.Height = (MFX_PICSTRUCT_PROGRESSIVE == m_mfxVppParams.vpp.Out.PicStruct) ?
			MSDK_ALIGN16(m_pInputParams->nDstHeight) : MSDK_ALIGN32(m_pInputParams->nDstHeight);
	}

	if (m_pInputParams->bEnableDeinterlacing)
	{
		// If stream were interlaced before then 32 bit alignment were applied.
		// Discard 32 bit alignment as progressive doesn't require it.
		m_mfxVppParams.vpp.Out.Height = MSDK_ALIGN16(m_mfxVppParams.vpp.Out.CropH);
		m_mfxVppParams.vpp.Out.Width = MSDK_ALIGN16(m_mfxVppParams.vpp.Out.CropW);
	}
	//--- Setting output FourCC type (input type is taken from m_mfxDecParams)
	if (m_pInputParams->EncoderFourCC)
	{
		m_mfxVppParams.vpp.Out.FourCC = m_pInputParams->EncoderFourCC;
		m_mfxVppParams.vpp.Out.ChromaFormat = FourCCToChroma(m_pInputParams->EncoderFourCC);
	}

	// configure and attach external parameters
	sts = AllocAndInitVppDoNotUse();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	if (m_VppDoNotUse.NumAlg > 0)
		m_VppExtParams.push_back((mfxExtBuffer*)&m_VppDoNotUse);
	/* VPP Comp Init */
	if ((m_pInputParams->eModeExt == VppComp || m_pInputParams->eModeExt == VppCompOnly) && m_pInputParams->numSurf4Comp != 0)
	{
		if (m_nVPPCompEnable != VppCompOnlyEncode)
			m_nVPPCompEnable = m_pInputParams->eModeExt;
		m_VppCompParams.Header.BufferId = MFX_EXTBUFF_VPP_COMPOSITE;
		m_VppCompParams.Header.BufferSz = sizeof(mfxExtVPPComposite);
		m_VppCompParams.NumInputStream = (mfxU16)m_pInputParams->numSurf4Comp;
		m_VppCompParams.InputStream = (mfxVPPCompInputStream *)malloc(sizeof(mfxVPPCompInputStream) * m_VppCompParams.NumInputStream);
		MSDK_CHECK_POINTER(m_VppCompParams.InputStream, MFX_ERR_NULL_PTR);
		// stream params
		/* if input streams in NV12 format background color should be in YUV format too
		* The same for RGB4 input, background color should be in ARGB format
		* */
		switch (m_pInputParams->EncoderFourCC)
		{
		case MFX_FOURCC_RGB4:
			/* back color in RGB */
			m_VppCompParams.R = 0x00;
			m_VppCompParams.G = 0x00;
			m_VppCompParams.B = 0x00;
			break;
		case MFX_FOURCC_NV12:
		case MFX_FOURCC_P010:
		case MFX_FOURCC_NV16:
		case MFX_FOURCC_P210:
		case MFX_FOURCC_YUY2:
		default:
			/* back color in YUV */
			m_VppCompParams.Y = 0x10;
			m_VppCompParams.U = 0x80;
			m_VppCompParams.V = 0x80;
			break;
		}
		MSDK_CHECK_POINTER(m_pInputParams->pVppCompDstRects, MFX_ERR_NULL_PTR);
		for (mfxU32 i = 0; i < m_pInputParams->numSurf4Comp; i++)
		{
			m_VppCompParams.InputStream[i].DstX = m_pInputParams->pVppCompDstRects[i].DstX;
			m_VppCompParams.InputStream[i].DstY = m_pInputParams->pVppCompDstRects[i].DstY;
			m_VppCompParams.InputStream[i].DstW = m_pInputParams->pVppCompDstRects[i].DstW;
			m_VppCompParams.InputStream[i].DstH = m_pInputParams->pVppCompDstRects[i].DstH;
			m_VppCompParams.InputStream[i].GlobalAlpha = 0;
			m_VppCompParams.InputStream[i].GlobalAlphaEnable = 0;
			m_VppCompParams.InputStream[i].PixelAlphaEnable = 0;

			m_VppCompParams.InputStream[i].LumaKeyEnable = 0;
			m_VppCompParams.InputStream[i].LumaKeyMin = 0;
			m_VppCompParams.InputStream[i].LumaKeyMax = 0;
		}
		m_VppExtParams.push_back((mfxExtBuffer *)&m_VppCompParams);
	}
	if (m_bUseOpaqueMemory)
		m_VppExtParams.push_back((mfxExtBuffer *)&m_VppOpaqueAlloc);
	if (m_pInputParams->bIsMVC)
		m_VppExtParams.push_back((mfxExtBuffer *)&m_MVCSeqDesc);
	if (m_pInputParams->DenoiseLevel != -1)
	{
		m_VppDenoiseParams.Header.BufferId = MFX_EXTBUFF_VPP_DENOISE;
		m_VppDenoiseParams.Header.BufferSz = sizeof(m_VppDenoiseParams);
		m_VppDenoiseParams.DenoiseFactor = (mfxU16)m_pInputParams->DenoiseLevel;
		m_VppExtParams.push_back((mfxExtBuffer*)&m_VppDenoiseParams);
	}
	if (m_pInputParams->DetailLevel != -1)
	{
		m_VppDetailParams.Header.BufferId = MFX_EXTBUFF_VPP_DETAIL;
		m_VppDetailParams.Header.BufferSz = sizeof(m_VppDetailParams);
		m_VppDetailParams.DetailFactor = (mfxU16)m_pInputParams->DetailLevel;
		m_VppExtParams.push_back((mfxExtBuffer*)&m_VppDetailParams);
	}
	if (m_pInputParams->FRCAlgorithm)
	{
		memset(&m_VppFrameRateConversionParams, 0, sizeof(m_VppFrameRateConversionParams));
		m_VppFrameRateConversionParams.Header.BufferId = MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION;
		m_VppFrameRateConversionParams.Header.BufferSz = sizeof(m_VppFrameRateConversionParams);
		m_VppFrameRateConversionParams.Algorithm = m_pInputParams->FRCAlgorithm;
		m_VppExtParams.push_back((mfxExtBuffer*)&m_VppFrameRateConversionParams);
	}
	if (m_pInputParams->bEnableDeinterlacing && m_pInputParams->DeinterlacingMode)
	{
		memset(&m_VppDeinterlaceParams, 0, sizeof(m_VppDeinterlaceParams));
		m_VppDeinterlaceParams.Header.BufferId = MFX_EXTBUFF_VPP_DEINTERLACING;
		m_VppDeinterlaceParams.Header.BufferSz = sizeof(m_VppDeinterlaceParams);
		m_VppDeinterlaceParams.Mode = m_pInputParams->DeinterlacingMode;
		m_VppExtParams.push_back((mfxExtBuffer*)&m_VppDeinterlaceParams);
	}
	//--- Field Copy Mode
	if (m_pInputParams->fieldProcessingMode)
	{
		m_VppFieldProcessParams.Header.BufferId = MFX_EXTBUFF_VPP_FIELD_PROCESSING;
		m_VppFieldProcessParams.Header.BufferSz = sizeof(m_VppFieldProcessParams);
		m_VppFieldProcessParams.Mode = m_pInputParams->fieldProcessingMode == FC_FR2FR ? MFX_VPP_COPY_FRAME : MFX_VPP_COPY_FIELD;
		m_VppFieldProcessParams.InField = (m_pInputParams->fieldProcessingMode == FC_T2T || m_pInputParams->fieldProcessingMode == FC_T2B) ? MFX_PICSTRUCT_FIELD_TFF : MFX_PICSTRUCT_FIELD_BFF;
		m_VppFieldProcessParams.OutField = (m_pInputParams->fieldProcessingMode == FC_T2T || m_pInputParams->fieldProcessingMode == FC_B2T) ? MFX_PICSTRUCT_FIELD_TFF : MFX_PICSTRUCT_FIELD_BFF;
		m_VppExtParams.push_back((mfxExtBuffer *)&m_VppFieldProcessParams);
	}

	m_mfxVppParams.ExtParam = &m_VppExtParams[0]; // vector is stored linearly in memory
	m_mfxVppParams.NumExtParam = (mfxU16)m_VppExtParams.size();
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::AllocAndInitVppDoNotUse()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	std::vector<mfxU32> filtersDisabled;
	if (m_pInputParams->DenoiseLevel == -1)
		filtersDisabled.push_back(MFX_EXTBUFF_VPP_DENOISE); // turn off denoising (on by default)
	filtersDisabled.push_back(MFX_EXTBUFF_VPP_SCENE_ANALYSIS); // turn off scene analysis (on by default)

	m_VppDoNotUse.NumAlg = (mfxU32)filtersDisabled.size();
	m_VppDoNotUse.AlgList = new mfxU32[m_VppDoNotUse.NumAlg];
	MSDK_CHECK_POINTER(m_VppDoNotUse.AlgList, MFX_ERR_MEMORY_ALLOC);
	MSDK_MEMCPY(m_VppDoNotUse.AlgList, &filtersDisabled[0], sizeof(mfxU32)*filtersDisabled.size());
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::DecodePreInit()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	if (CheckVersion(&m_Version, MSDK_FEATURE_PLUGIN_API))
	{
		/* Here we actually define the following codec initialization scheme:
		*  1. If plugin path or guid is specified: we load user-defined plugin (example: VP8 sample decoder plugin)
		*  2. If plugin path not specified:
		*    2.a) we check if codec is distributed as a mediasdk plugin and load it if yes
		*    2.b) if codec is not in the list of mediasdk plugins, we assume, that it is supported inside mediasdk library
		*/
		if (m_pInputParams->decoderPluginParams.pluginType == MFX_PLUGINLOAD_TYPE_FILE && strnlen_s((const char*)m_pInputParams->decoderPluginParams.pluginPath, sizeof(m_pInputParams->decoderPluginParams.pluginPath)))
		{
			m_pUserDecoderModule.reset(new MFXVideoUSER(*m_pmfxSession.get()));
			m_pUserDecoderPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, *m_pmfxSession.get(), m_pInputParams->decoderPluginParams.pluginGuid, 1, m_pInputParams->decoderPluginParams.pluginPath, (mfxU32)strnlen_s((const char*)m_pInputParams->decoderPluginParams.pluginPath, sizeof(m_pInputParams->decoderPluginParams.pluginPath))));
			if (m_pUserDecoderPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
		}
		else
		{
			bool isDefaultPlugin = false;
			if (AreGuidsEqual(m_pInputParams->decoderPluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
			{
				m_pInputParams->decoderPluginParams.pluginGuid = msdkGetPluginUID(m_pInputParams->bUseHW ? MFX_IMPL_HARDWARE_ANY : MFX_IMPL_SOFTWARE, MSDK_VDECODE, m_pInputParams->DecodeId);
				isDefaultPlugin = true;
			}
			if (!AreGuidsEqual(m_pInputParams->decoderPluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
			{
				m_pUserDecoderPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, *m_pmfxSession.get(), m_pInputParams->decoderPluginParams.pluginGuid, 1));
				if (m_pUserDecoderPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
			}
			if (sts == MFX_ERR_UNSUPPORTED)
			{
				msdk_printf(isDefaultPlugin ?
					MSDK_STRING("Default plugin cannot be loaded (possibly you have to define plugin explicitly)\n")
					: MSDK_STRING("Explicitly specified plugin cannot be loaded.\n"));
			}
		}
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}

	// create decoder
	m_pmfxDEC.reset(new MFXVideoDECODE(*m_pmfxSession.get()));
	// set video type in parameters
	m_mfxDecParams.mfx.CodecId = m_pInputParams->DecodeId;
	// configure specific decoder parameters
	sts = InitDecMfxParams();
	if (MFX_ERR_MORE_DATA == sts)
	{
		m_pmfxDEC.reset(NULL);
		return sts;
	}
	else
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	m_nReqFrameTime = m_pInputParams->nFPS > 0 ? (1000000 / m_pInputParams->nFPS) : 0;
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::InitDecMfxParams()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	m_mfxDecParams.AsyncDepth = m_pInputParams->nAsyncDepth;
	// configure and attach external parameters
	if (m_bUseOpaqueMemory)
		m_DecExtParams.push_back((mfxExtBuffer *)&m_DecOpaqueAlloc);
	if (m_pInputParams->bIsMVC)
		m_DecExtParams.push_back((mfxExtBuffer *)&m_MVCSeqDesc);
	if (!m_DecExtParams.empty())
	{
		m_mfxDecParams.ExtParam = &m_DecExtParams[0]; // vector is stored linearly in memory
		m_mfxDecParams.NumExtParam = (mfxU16)m_DecExtParams.size();
	}

	// read a portion of data for DecodeHeader function
	sts = m_BSProcesser.GetInputBitstream(&m_pmfxBS);
	if (MFX_ERR_MORE_DATA == sts)
		return sts;
	else
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	// try to find a sequence header in the stream
	// if header is not found this function exits with error (e.g. if device was lost and there's no header in the remaining stream)
	for (;;)
	{
		// trying to find PicStruct information in AVI headers
		if (m_pInputParams->DecodeId == MFX_CODEC_JPEG)
			MJPEG_AVI_ParsePicStruct(m_pmfxBS);
		// parse bit stream and fill mfx params
		sts = m_pmfxDEC->DecodeHeader(m_pmfxBS, &m_mfxDecParams);
		if (MFX_ERR_MORE_DATA == sts)
		{
			if (m_pmfxBS->MaxLength == m_pmfxBS->DataLength)
			{
				sts = ExtendMfxBitstream(m_pmfxBS, m_pmfxBS->MaxLength * 2);
				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			}
			// read a portion of data for DecodeHeader function
			sts = m_BSProcesser.GetInputBitstream(&m_pmfxBS);
			if (MFX_ERR_MORE_DATA == sts)
				return sts;
			else
				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			continue;
		}
		else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts && m_pInputParams->bIsMVC)
		{
			sts = AllocMVCSeqDesc();
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

			continue;
		}
		else
			break;
	}
	// to enable decorative flags, has effect with 1.3 API libraries only
	// (in case of JPEG decoder - it is not valid to use this field)
	if (m_mfxDecParams.mfx.CodecId != MFX_CODEC_JPEG)
		m_mfxDecParams.mfx.ExtendedPicStruct = 1;
	// check DecodeHeader status
	if (MFX_WRN_PARTIAL_ACCELERATION == sts)
	{
		msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	}
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	// set memory pattern
	if (m_bUseOpaqueMemory)
		m_mfxDecParams.IOPattern = MFX_IOPATTERN_OUT_OPAQUE_MEMORY;
	else if (m_pInputParams->bForceSysMem || !m_pInputParams->bUseHW)
		m_mfxDecParams.IOPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
	else
		m_mfxDecParams.IOPattern = MFX_IOPATTERN_OUT_VIDEO_MEMORY;

	// if input is interlaced JPEG stream
	if (((m_pInputParams->DecodeId == MFX_CODEC_JPEG) && (m_pmfxBS->PicStruct == MFX_PICSTRUCT_FIELD_TFF))
		|| (m_pmfxBS->PicStruct == MFX_PICSTRUCT_FIELD_BFF))
	{
		m_mfxDecParams.mfx.FrameInfo.CropH *= 2;
		m_mfxDecParams.mfx.FrameInfo.Height = MSDK_ALIGN16(m_mfxDecParams.mfx.FrameInfo.CropH);
		m_mfxDecParams.mfx.FrameInfo.PicStruct = m_pmfxBS->PicStruct;
	}
	
	// if frame rate specified by user set it for decoder and the whole pipeline
	if (m_pInputParams->dFrameRate)
		ConvertFrameRate(m_pInputParams->dFrameRate, &m_mfxDecParams.mfx.FrameInfo.FrameRateExtN, &m_mfxDecParams.mfx.FrameInfo.FrameRateExtD);
	// if frame rate not specified and input stream header doesn't contain valid values use default (30.0)
	else if (!(m_mfxDecParams.mfx.FrameInfo.FrameRateExtN * m_mfxDecParams.mfx.FrameInfo.FrameRateExtD))
	{
		m_mfxDecParams.mfx.FrameInfo.FrameRateExtN = 30;
		m_mfxDecParams.mfx.FrameInfo.FrameRateExtD = 1;
	}
	else
	{
		// use the value from input stream header
	}

	//--- Force setting fourcc type if required
	if (m_pInputParams->DecoderFourCC)
	{
		m_mfxDecParams.mfx.FrameInfo.FourCC = m_pInputParams->DecoderFourCC;
		m_mfxDecParams.mfx.FrameInfo.ChromaFormat = FourCCToChroma(m_pInputParams->DecoderFourCC);
	}
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::AllocMVCSeqDesc()
{
	m_MVCSeqDesc.View = new mfxMVCViewDependency[m_MVCSeqDesc.NumView];
	MSDK_CHECK_POINTER(m_MVCSeqDesc.View, MFX_ERR_MEMORY_ALLOC);
	for (mfxU32 i = 0; i < m_MVCSeqDesc.NumView; ++i)
		MSDK_ZERO_MEMORY(m_MVCSeqDesc.View[i]);
	m_MVCSeqDesc.NumViewAlloc = m_MVCSeqDesc.NumView;

	m_MVCSeqDesc.ViewId = new mfxU16[m_MVCSeqDesc.NumViewId];
	MSDK_CHECK_POINTER(m_MVCSeqDesc.ViewId, MFX_ERR_MEMORY_ALLOC);
	for (mfxU32 i = 0; i < m_MVCSeqDesc.NumViewId; ++i)
		MSDK_ZERO_MEMORY(m_MVCSeqDesc.ViewId[i]);
	m_MVCSeqDesc.NumViewIdAlloc = m_MVCSeqDesc.NumViewId;

	m_MVCSeqDesc.OP = new mfxMVCOperationPoint[m_MVCSeqDesc.NumOP];
	MSDK_CHECK_POINTER(m_MVCSeqDesc.OP, MFX_ERR_MEMORY_ALLOC);
	for (mfxU32 i = 0; i < m_MVCSeqDesc.NumOP; ++i)
		MSDK_ZERO_MEMORY(m_MVCSeqDesc.OP[i]);
	m_MVCSeqDesc.NumOPAlloc = m_MVCSeqDesc.NumOP;
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::InitSession()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	mfxInitParam initPar;
	mfxExtThreadsParam threadsPar;
	mfxExtBuffer* extBufs[1];

	MSDK_ZERO_MEMORY(initPar);
	MSDK_ZERO_MEMORY(threadsPar);

	// we set version to 1.0 and later we will query actual version of the library which will got leaded
	initPar.Version.Major = 1;
	initPar.Version.Minor = 0;
	initPar.Implementation = m_pInputParams->bUseHW ? MFX_IMPL_HARDWARE_ANY : MFX_IMPL_SOFTWARE;

	init_ext_buffer(threadsPar);
	bool needInitExtPar = false;
	if (m_pInputParams->nThreadsNum) 
	{
		threadsPar.NumThread = m_pInputParams->nThreadsNum;
		needInitExtPar = true;
	}
	if (needInitExtPar) 
	{
		extBufs[0] = (mfxExtBuffer*)&threadsPar;
		initPar.ExtParam = extBufs;
		initPar.NumExtParam = 1;
	}
	//--- GPU Copy settings
	initPar.GPUCopy = m_pInputParams->nGpuCopyMode;
	// init session
	m_pmfxSession.reset(new MFXVideoSession);
	if (initPar.Implementation & MFX_IMPL_HARDWARE_ANY)
	{
		// try search for MSDK on all display adapters
		sts = m_pmfxSession->InitEx(initPar);
		// MSDK API version may have no support for multiple adapters - then try initialize on the default
		if (MFX_ERR_NONE != sts) 
		{
			initPar.Implementation = (initPar.Implementation & (!MFX_IMPL_HARDWARE_ANY)) | MFX_IMPL_HARDWARE;
			sts = m_pmfxSession->InitEx(initPar);
		}
	}
	else
		sts = m_pmfxSession->InitEx(initPar);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	// check the API version of actually loaded library
	sts = m_pmfxSession->QueryVersion(&m_Version);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	sts = CheckRequiredAPIVersion(m_Version);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	// opaque memory feature is available starting with API 1.3 and
	// can be used within 1 intra session or joined inter sessions only
	if (m_Version.Major >= 1 && m_Version.Minor >= 3 &&	(m_pInputParams->eMode == Native || m_pInputParams->bIsJoin))
		m_bUseOpaqueMemory = true;
	// Don't use opaque in case of yuv output or if it was specified explicitly
	if (!m_pInputParams->bUseOpaqueMemory || m_pInputParams->EncodeId == MFX_FOURCC_DUMP)
		m_bUseOpaqueMemory = false;
	// Media SDK session doesn't require external allocator if the application uses opaque memory
	if (!m_bUseOpaqueMemory)
	{
		sts = m_pmfxSession->SetFrameAllocator(m_pGeneralAllocator.get());
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::CheckRequiredAPIVersion(mfxVersion& version)
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	if (m_pInputParams->bIsMVC && !CheckVersion(&version, MSDK_FEATURE_MVC))
	{
		msdk_printf(MSDK_STRING("error: MVC is not supported in the %d.%d API version\n"), version.Major, version.Minor);
		return MFX_ERR_UNSUPPORTED;
	}
	if ((m_pInputParams->DecodeId == MFX_CODEC_JPEG) && !CheckVersion(&version, MSDK_FEATURE_JPEG_DECODE)) 
	{
		msdk_printf(MSDK_STRING("error: Jpeg decoder is not supported in the %d.%d API version\n"),	version.Major, version.Minor);
		return MFX_ERR_UNSUPPORTED;
	}
	if ((m_pInputParams->EncodeId == MFX_CODEC_JPEG) && !CheckVersion(&version, MSDK_FEATURE_JPEG_ENCODE)) 
	{
		msdk_printf(MSDK_STRING("error: Jpeg encoder is not supported in the %d.%d API version\n"),	version.Major, version.Minor);
		return MFX_ERR_UNSUPPORTED;
	}
	if ((m_pInputParams->bLABRC || m_pInputParams->nLADepth) && !CheckVersion(&version, MSDK_FEATURE_LOOK_AHEAD)) 
	{
		msdk_printf(MSDK_STRING("error: Look Ahead is not supported in the %d.%d API version\n"), version.Major, version.Minor);
		return MFX_ERR_UNSUPPORTED;
	}
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::CreateAllocator()
{
	MSDK_CHECK_POINTER(m_pInputParams, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	mfxHDL hdl = NULL;
	if (m_pInputParams->MemType == enmMemType_D3D9)
	{
		m_pAllocParam.reset(new D3DAllocatorParams);
		m_hwdev.reset(new CD3D9Device());
		/* The last param set in vector always describe VPP+ENCODE or Only VPP
		* So, if we want to do rendering we need to do pass HWDev to CTranscodingPipeline */
		if (m_pInputParams->eModeExt == VppCompOnly)
		{
			/* Rendering case */
			sts = m_hwdev->Init(NULL, 1, MSDKAdapter::GetNumber());
			//m_pInputParams->m_hwdev = m_hwdev.get();
		}
		else /* NO RENDERING*/
		{
			sts = m_hwdev->Init(NULL, 0, MSDKAdapter::GetNumber());
		}
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		sts = m_hwdev->GetHandle(MFX_HANDLE_D3D9_DEVICE_MANAGER, (mfxHDL*)&hdl);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		// set Device Manager to external dx9 allocator
		D3DAllocatorParams *pD3DParams = dynamic_cast<D3DAllocatorParams*>(m_pAllocParam.get());
		pD3DParams->pManager = (IDirect3DDeviceManager9*)hdl;
	}
	else if (m_pInputParams->MemType == enmMemType_D3D11)
	{
		m_pAllocParam.reset(new D3D11AllocatorParams);
		m_hwdev.reset(new CD3D11Device());
		/* The last param set in vector always describe VPP+ENCODE or Only VPP
		* So, if we want to do rendering we need to do pass HWDev to CTranscodingPipeline */
		if (m_pInputParams->eModeExt == VppCompOnly)
		{
			/* Rendering case */
			sts = m_hwdev->Init(NULL, 1, MSDKAdapter::GetNumber());
			//m_pInputParams->m_hwdev = m_hwdev.get();
		}
		else /* NO RENDERING*/
		{
			sts = m_hwdev->Init(NULL, 0, MSDKAdapter::GetNumber());
		}
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		sts = m_hwdev->GetHandle(MFX_HANDLE_D3D11_DEVICE, (mfxHDL*)&hdl);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		// set Device to external dx11 allocator
		D3D11AllocatorParams *pD3D11Params = dynamic_cast<D3D11AllocatorParams*>(m_pAllocParam.get());
		pD3D11Params->pDevice = (ID3D11Device*)hdl;
	}
	if (!m_pAllocParam.get())
		m_pAllocParam.reset(new SysMemAllocatorParams);

	m_pGeneralAllocator.reset(new GeneralAllocator);
	sts = m_pGeneralAllocator.get()->Init(m_pAllocParam.get());
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	mfxIMPL impl = 0;
	m_pmfxSession->QueryIMPL(&impl);
	mfxHandleType handleType = (mfxHandleType)0;
	bool bIsMustSetExternalHandle = false;
	if (MFX_IMPL_VIA_D3D11 == MFX_IMPL_VIA_MASK(impl))
	{
		handleType = MFX_HANDLE_D3D11_DEVICE;
		bIsMustSetExternalHandle = false;
	}
	else if (MFX_IMPL_VIA_D3D9 == MFX_IMPL_VIA_MASK(impl))
	{
		handleType = MFX_HANDLE_D3D9_DEVICE_MANAGER;
		bIsMustSetExternalHandle = false;
	}
	bool bIsInterOrJoined = m_pInputParams->eMode == Sink || m_pInputParams->eMode == Source || m_pInputParams->bIsJoin;
	if (hdl && (bIsMustSetExternalHandle || (bIsInterOrJoined || !m_bUseOpaqueMemory)))
	{
		sts = m_pmfxSession->SetHandle(handleType, hdl);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
	return MFX_ERR_NONE;
}

void CDecodingPipe::Release()
{
}
mfxStatus CDecodingPipe::Connect(sStrQue* conn_handle)
{
	MSDK_CHECK_POINTER(conn_handle, MFX_ERR_NULL_PTR);
	m_listInput = conn_handle;
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::Loop()
{
	mfxStatus sts = MFX_ERR_NONE;
	ExtendedSurface DecExtSurface = { 0 };
	ExtendedSurface VppExtSurface = { 0 };
	ExtendedBS *pBS = NULL;
	bool bNeedDecodedFrames = true; // indicates if we need to decode frames
	bool bEndOfFile = false;
	bool bLastCycle = false;
	bool bInsertIDR = false;
	bool shouldReadNextFrame = true;
	PreEncAuxBuffer encAuxCtrl;

	MSDK_ZERO_MEMORY(encAuxCtrl);
	encAuxCtrl.encCtrl.FrameType = MFX_FRAMETYPE_I | MFX_FRAMETYPE_IDR | MFX_FRAMETYPE_REF;

	time_t start = time(0);
	ULONGLONG tstart = GetTickCount64();
	while (MFX_ERR_NONE == sts)
	{
		msdk_tick nBeginTime = msdk_time_get_tick(); // microseconds.

		if (time(0) - start >= 0)
			bLastCycle = true;
		//if (m_MaxFramesForTranscode == m_nProcessedFramesNum)
		//{
		//	DecExtSurface.pSurface = NULL;  // to get buffered VPP or ENC frames
		//	bNeedDecodedFrames = false; // no more decoded frames needed
		//}

		// if need more decoded frames
		// decode a frame
		if (bNeedDecodedFrames && shouldReadNextFrame)
		{
			if (!bEndOfFile)
			{
				sts = DecodeOneFrame(&DecExtSurface);
				if (MFX_ERR_MORE_DATA == sts)
				{
					if (!bLastCycle)
					{
						bInsertIDR = true;

						//static_cast<FileBitstreamProcessor_WithReset*>(&m_BSProcesser)->ResetInput();
						//static_cast<FileBitstreamProcessor_WithReset*>(&m_BSProcesser)->ResetOutput();
						bNeedDecodedFrames = true;

						bEndOfFile = false;
						sts = MFX_ERR_NONE;
						continue;
					}
					else
					{
						bEndOfFile = true;
					}
				}
			}

			if (bEndOfFile)
			{
				sts = DecodeLastFrame(&DecExtSurface);
			}

			if (sts == MFX_ERR_MORE_DATA)
			{
				DecExtSurface.pSurface = NULL;  // to get buffered VPP or ENC frames
				sts = MFX_ERR_NONE;
			}

			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}

		// pre-process a frame
		if (m_pmfxVPP.get())
		{
			sts = VPPOneFrame(&DecExtSurface, &VppExtSurface);
		}
		else // no VPP - just copy pointers
		{
			VppExtSurface.pSurface = DecExtSurface.pSurface;
			VppExtSurface.pCtrl = DecExtSurface.pCtrl;
			VppExtSurface.Syncp = DecExtSurface.Syncp;
		}

		if (MFX_ERR_MORE_SURFACE == sts)
		{
			shouldReadNextFrame = false;
			sts = MFX_ERR_NONE;
		}
		else
		{
			shouldReadNextFrame = true;
		}

		if (sts == MFX_ERR_MORE_DATA)
		{
			sts = MFX_ERR_NONE;
			if (NULL == DecExtSurface.pSurface) // there are no more buffered frames in VPP
			{
				VppExtSurface.pSurface = NULL; // to get buffered ENC frames
			}
			else
			{
				continue; // go get next frame from Decode
			}
		}

		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		// encode frame
		pBS = m_pBSStore->GetNext();
		if (!pBS)
			return MFX_ERR_NOT_FOUND;

		m_BSPool.push_back(pBS);

		// encode frame only if it wasn't encoded enough
		SetSurfaceAuxIDR(VppExtSurface, &encAuxCtrl, bInsertIDR);
		bInsertIDR = false;

		if (bNeedDecodedFrames)
		{
			if (m_mfxEncParams.mfx.CodecId != MFX_FOURCC_DUMP)
			{
				sts = EncodeOneFrame(&VppExtSurface, &m_BSPool.back()->Bitstream);
			}
			else
			{
				//sts = Surface2BS(&VppExtSurface, &m_BSPool.back()->Bitstream, m_mfxVppParams.vpp.Out.FourCC);
			}
		}
		else
		{
			sts = MFX_ERR_MORE_DATA;
		}

		// check if we need one more frame from decode
		if (MFX_ERR_MORE_DATA == sts)
		{
			// the task in not in Encode queue
			m_BSPool.pop_back();
			m_pBSStore->Release(pBS);

			if (NULL == VppExtSurface.pSurface) // there are no more buffered frames in encoder
			{
				break;
			}
			sts = MFX_ERR_NONE;
			continue;
		}

		// check encoding result
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		m_BSPool.back()->Syncp = VppExtSurface.Syncp;
		m_BSPool.back()->Surface1 = DecExtSurface.pSurface;
		m_BSPool.back()->Surface2 = VppExtSurface.pSurface;

		if (m_BSPool.size() == m_pInputParams->nAsyncDepth)
		{
			static int index = 0;
			sts = PutBS();
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			index += 1;
			if (DecExtSurface.pSurface == NULL && VppExtSurface.pSurface == NULL)
			{
				ULONGLONG end = GetTickCount();
				int tt = index;
				char str[256] = { 0 };
				string strt;
				sprintf_s(str, 256, "Time_long=%f", (end - tstart)/1000.0);
				strt.append(str);
				sprintf_s(str, 256, " Frames=%d\n", index);
				strt.append(str);
				printf(strt.c_str());
				return MFX_ERR_NONE;
			}
		}

		msdk_tick nFrameTime = msdk_time_get_tick() - nBeginTime;
		if (nFrameTime < m_nReqFrameTime)
		{
			MSDK_USLEEP((mfxU32)(m_nReqFrameTime - nFrameTime));
		}
	}
	MSDK_IGNORE_MFX_STS(sts, MFX_ERR_MORE_DATA);

	// need to get buffered bitstream
	if (MFX_ERR_NONE == sts)
	{
		while (m_BSPool.size())
		{
			sts = PutBS();
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}
	}

	if (MFX_ERR_NONE == sts)
		sts = MFX_WRN_VALUE_NOT_CHANGED;

	return sts;
}

mfxStatus CDecodingPipe::PutBS()
{
	mfxStatus       sts = MFX_ERR_NONE;
	ExtendedBS *pBitstreamEx = m_BSPool.front();
	// get result coded stream, synchronize only if we still have sync point
	if (pBitstreamEx->Syncp)
	{
		sts = m_pmfxSession->SyncOperation(pBitstreamEx->Syncp, MSDK_WAIT_INTERVAL);
	}
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	//m_nOutputFramesNum++;

	////--- Time measurements
	//if (statisticsWindowSize)
	//{
	//	outputStatistics.StopTimeMeasurementWithCheck();
	//	outputStatistics.StartTimeMeasurement();
	//}

	sts = m_BSProcesser.ProcessOutputBitstream(&pBitstreamEx->Bitstream);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	if (pBitstreamEx->pCtrl)
		msdk_atomic_dec16(&pBitstreamEx->pCtrl->Locked);

	pBitstreamEx->Bitstream.DataLength = 0;
	pBitstreamEx->Bitstream.DataOffset = 0;

	m_BSPool.pop_front();
	m_pBSStore->Release(pBitstreamEx);
	return sts;
}

mfxStatus CDecodingPipe::EncodeOneFrame(ExtendedSurface *pExtSurface, mfxBitstream *pBS)
{
	mfxStatus sts = MFX_ERR_NONE;
	mfxEncodeCtrl *pCtrl = (pExtSurface->pCtrl) ? &pExtSurface->pCtrl->encCtrl : NULL;

	for (;;)
	{
		// at this point surface for encoder contains either a frame from file or a frame processed by vpp
		sts = m_pmfxENC->EncodeFrameAsync(pCtrl, pExtSurface->pSurface, pBS, &pExtSurface->Syncp);

		if (MFX_ERR_NONE < sts && !pExtSurface->Syncp) // repeat the call if warning and no output
		{
			if (MFX_WRN_DEVICE_BUSY == sts)
				Sleep(TIME_TO_SLEEP); // wait if device is busy
		}
		else if (MFX_ERR_NONE < sts && pExtSurface->Syncp)
		{
			sts = MFX_ERR_NONE; // ignore warnings if output is available
			break;
		}
		else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)
		{
			sts = AllocateSufficientBuffer(pBS);
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}
		else
		{
			break;
		}
	}
	return sts;
}

mfxStatus CDecodingPipe::AllocateSufficientBuffer(mfxBitstream* pBS)
{
	MSDK_CHECK_POINTER(pBS, MFX_ERR_NULL_PTR);
	mfxVideoParam par;
	MSDK_ZERO_MEMORY(par);
	// find out the required buffer size
	mfxStatus sts = m_pmfxENC->GetVideoParam(&par);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	mfxU32 new_size = 0;
	// if encoder provided us information about buffer size
	if (0 != par.mfx.BufferSizeInKB)
	{
		new_size = par.mfx.BufferSizeInKB * 1000;
	}
	else
	{
		// trying to guess the size (e.g. for JPEG encoder)
		new_size = (0 == pBS->MaxLength)
			// some heuristic init value
			? 4 + (par.mfx.FrameInfo.Width * par.mfx.FrameInfo.Height * 3 + 1023)
			// double existing size
			: 2 * pBS->MaxLength;
	}
	sts = ExtendMfxBitstream(pBS, new_size);
	MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMfxBitstream(pBS));
	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipe::VPPOneFrame(ExtendedSurface *pSurfaceIn, ExtendedSurface *pExtSurface)
{
	MSDK_CHECK_POINTER(pExtSurface, MFX_ERR_NULL_PTR);
	mfxFrameSurface1 *pmfxSurface = NULL;
	// find/wait for a free working surface
	pmfxSurface = GetFreeSurface(false, MSDK_SURFACE_WAIT_INTERVAL);
	MSDK_CHECK_POINTER_SAFE(pmfxSurface, MFX_ERR_MEMORY_ALLOC, msdk_printf(MSDK_STRING("ERROR: No free surfaces for VPP in encoder pool (during long period)\n"))); // return an error if a free surface wasn't found

	// make sure picture structure has the initial value
	// surfaces are reused and VPP may change this parameter in certain configurations
	pmfxSurface->Info.PicStruct = m_mfxVppParams.vpp.Out.PicStruct ? m_mfxVppParams.vpp.Out.PicStruct : (/*m_bEncodeEnable*/true ? m_mfxEncParams : m_mfxDecParams).mfx.FrameInfo.PicStruct;

	pExtSurface->pSurface = pmfxSurface;
	mfxStatus sts = MFX_ERR_NONE;
	for (;;)
	{
		sts = m_pmfxVPP->RunFrameVPPAsync(pSurfaceIn->pSurface, pmfxSurface, NULL, &pExtSurface->Syncp);

		if (MFX_ERR_NONE < sts && !pExtSurface->Syncp) // repeat the call if warning and no output
		{
			if (MFX_WRN_DEVICE_BUSY == sts)
				MSDK_SLEEP(1); // wait if device is busy
		}
		else if (MFX_ERR_NONE < sts && pExtSurface->Syncp)
		{
			sts = MFX_ERR_NONE; // ignore warnings if output is available
			break;
		}
		else
		{
			break;
		}
	}
	return sts;

}

mfxStatus CDecodingPipe::DecodeLastFrame(ExtendedSurface *pExtSurface)
{
	mfxFrameSurface1    *pmfxSurface = NULL;
	mfxStatus sts = MFX_ERR_MORE_SURFACE;

	////--- Time measurements
	//if (statisticsWindowSize)
	//{
	//	inputStatistics.StopTimeMeasurementWithCheck();
	//	inputStatistics.StartTimeMeasurement();
	//}

	CTimer DevBusyTimer;
	DevBusyTimer.Start();
	// retrieve the buffered decoded frames
	while (MFX_ERR_MORE_SURFACE == sts || MFX_WRN_DEVICE_BUSY == sts)
	{
		if (MFX_WRN_DEVICE_BUSY == sts)
		{
			WaitForDeviceToBecomeFree(*m_pmfxSession, m_LastDecSyncPoint, sts);
		}

		// find new working surface
		pmfxSurface = GetFreeSurface(true, MSDK_SURFACE_WAIT_INTERVAL);

		MSDK_CHECK_POINTER_SAFE(pmfxSurface, MFX_ERR_MEMORY_ALLOC, msdk_printf(MSDK_STRING("ERROR: No free surfaces in decoder pool (during long period)\n"))); // return an error if a free surface wasn't found
		sts = m_pmfxDEC->DecodeFrameAsync(NULL, pmfxSurface, &pExtSurface->pSurface, &pExtSurface->Syncp);

		if ((MFX_WRN_DEVICE_BUSY == sts) &&
			(DevBusyTimer.GetTime() > MSDK_DEVICE_FREE_WAIT_INTERVAL / 1000))
		{
			msdk_printf(MSDK_STRING("ERROR: Decoder device busy (during long period)\n"));
			return MFX_ERR_DEVICE_FAILED;
		}
	}
	return sts;
}

mfxStatus CDecodingPipe::DecodeOneFrame(ExtendedSurface *pExtSurface)
{
	MSDK_CHECK_POINTER(pExtSurface, MFX_ERR_NULL_PTR);

	mfxStatus sts = MFX_ERR_MORE_SURFACE;
	mfxFrameSurface1    *pmfxSurface = NULL;
	pExtSurface->pSurface = NULL;

	CTimer DevBusyTimer;
	DevBusyTimer.Start();
	while (MFX_ERR_MORE_DATA == sts || MFX_ERR_MORE_SURFACE == sts || MFX_ERR_NONE < sts)
	{
		if (MFX_WRN_DEVICE_BUSY == sts)
		{
			WaitForDeviceToBecomeFree(*m_pmfxSession, m_LastDecSyncPoint, sts);
		}
		else if (MFX_ERR_MORE_DATA == sts)
		{
			sts = m_BSProcesser.GetInputBitstream(&m_pmfxBS); // read more data to input bit stream
			MSDK_BREAK_ON_ERROR(sts);
			if (m_pmfxBS->DataOffset == 999)
				return MFX_ERR_MORE_DATA;
		}
		else if (MFX_ERR_MORE_SURFACE == sts)
		{
			// Find new working surface
			pmfxSurface = GetFreeSurface(true, MSDK_SURFACE_WAIT_INTERVAL);
			MSDK_CHECK_POINTER_SAFE(pmfxSurface, MFX_ERR_MEMORY_ALLOC, msdk_printf(MSDK_STRING("ERROR: No free surfaces in decoder pool (during long period)\n"))); // return an error if a free surface wasn't found
		}

		sts = m_pmfxDEC->DecodeFrameAsync(m_pmfxBS, pmfxSurface, &pExtSurface->pSurface, &pExtSurface->Syncp);

		if ((MFX_WRN_DEVICE_BUSY == sts) &&
			(DevBusyTimer.GetTime() > MSDK_DEVICE_FREE_WAIT_INTERVAL / 1000))
		{
			msdk_printf(MSDK_STRING("ERROR: Decoder device busy (during long period)\n"));
			return MFX_ERR_DEVICE_FAILED;
		}

		if (sts == MFX_ERR_NONE)
		{
			m_LastDecSyncPoint = pExtSurface->Syncp;
		}
		// ignore warnings if output is available,
		if (MFX_ERR_NONE < sts && pExtSurface->Syncp)
		{
			sts = MFX_ERR_NONE;
		}

	} //while processing

	return sts;

}

mfxFrameSurface1* CDecodingPipe::GetFreeSurface(bool isDec, mfxU64 timeout)
{
	mfxFrameSurface1* pSurf = NULL;

	CTimer t;
	t.Start();
	do
	{
		std::vector<mfxFrameSurface1*>& workArray = isDec ? m_pSurfaceDecPool : m_pSurfaceEncPool;
		for (mfxU32 i = 0; i < workArray.size(); i++)
		{
			if (!workArray[i]->Data.Locked)
			{
				pSurf = workArray[i];
				break;
			}
		}
		if (pSurf)
		{
			break;
		}
		else
		{
			Sleep(TIME_TO_SLEEP);
		}
	} while (t.GetTime() < timeout / 1000);

	return pSurf;
}