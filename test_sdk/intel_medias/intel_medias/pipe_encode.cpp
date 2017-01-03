//#include "pipe_encode.h"
//#include "comm_func.h"
//#include "plugin_utils.h"
//#include "plugin_loader.h"
//
//void CEncodingPipe::ThreadEncLoop(LPARAM lParam)
//{
//	((CEncodingPipe*)lParam)->EncodeLoop();
//}
//
//CEncodingPipe::CEncodingPipe()
//{
//	m_mfxSession = NULL;
//	m_pmfxEnc = NULL;
//	m_pmfxVPP = NULL;
//	m_pMFXAllocator = NULL;
//	m_pmfxAllocatorParams = NULL;
//	m_hwdev = NULL;
//	m_bExit = false;
//	m_handleThread = NULL;
//	m_listInput = NULL;
//	m_listInput_Type = SourceType_Null;
//	m_inputParams = NULL;
//	m_bExternalAlloc = false;
//	m_pSurfaces = NULL;
//	m_pVppSurfaces = NULL;
//	m_bInitEncode = false;
//
//	MSDK_ZERO_MEMORY(m_VppDoNotUse);
//	m_VppDoNotUse.Header.BufferId = MFX_EXTBUFF_VPP_DONOTUSE;
//	m_VppDoNotUse.Header.BufferSz = sizeof(m_VppDoNotUse);
//	MSDK_ZERO_MEMORY(m_MVCSeqDesc);
//	m_MVCSeqDesc.Header.BufferId = MFX_EXTBUFF_MVC_SEQ_DESC;
//	m_MVCSeqDesc.Header.BufferSz = sizeof(m_MVCSeqDesc);
//	MSDK_ZERO_MEMORY(m_CodingOption);
//	m_CodingOption.Header.BufferId = MFX_EXTBUFF_CODING_OPTION;
//	m_CodingOption.Header.BufferSz = sizeof(m_CodingOption);
//	MSDK_ZERO_MEMORY(m_CodingOption2);
//	m_CodingOption2.Header.BufferId = MFX_EXTBUFF_CODING_OPTION2;
//	m_CodingOption2.Header.BufferSz = sizeof(m_CodingOption2);
//	MSDK_ZERO_MEMORY(m_ExtHEVCParam);
//	m_ExtHEVCParam.Header.BufferId = MFX_EXTBUFF_HEVC_PARAM;
//	m_ExtHEVCParam.Header.BufferSz = sizeof(m_ExtHEVCParam);
//}
//
//CEncodingPipe::~CEncodingPipe()
//{
//	Release();
//}
//
//mfxStatus CEncodingPipe::Init(sInputParams_Enc *pParams)
//{
//	MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);
//	if (m_mfxSession == NULL)
//		m_mfxSession = intelmedia_session_create(pParams);
//	MSDK_CHECK_POINTER(m_mfxSession, MFX_ERR_NULL_PTR);
//
//	mfxStatus sts = MFX_ERR_NONE;
//	mfxVersion version;
//	sts = MFXQueryVersion(*m_mfxSession, &version); // get real API version of the loaded library
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	if (CheckVersion(&version, MSDK_FEATURE_PLUGIN_API))
//	{
//		/* Here we actually define the following codec initialization scheme:
//		*  1. If plugin path or guid is specified: we load user-defined plugin (example: HEVC encoder plugin)
//		*  2. If plugin path not specified:
//		*    2.a) we check if codec is distributed as a mediasdk plugin and load it if yes
//		*    2.b) if codec is not in the list of mediasdk plugins, we assume, that it is supported inside mediasdk library
//		*/
//		if (pParams->pluginType == MFX_PLUGINLOAD_TYPE_FILE && strnlen_s(pParams->pluginPath, sizeof(pParams->pluginPath)))
//		{
//			m_pUserModule.reset(new MFXVideoUSER(*m_mfxSession));
//			if (pParams->codecId == CODEC_VP8 || pParams->codecId == MFX_CODEC_HEVC)
//				m_pPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, *m_mfxSession, pParams->pluginGUID, 1, pParams->pluginPath, (mfxU32)strnlen_s(pParams->pluginPath, sizeof(pParams->pluginPath))));
//			if (m_pPlugin.get() == NULL)
//				sts = MFX_ERR_UNSUPPORTED;
//		}
//		else
//		{
//			if (AreGuidsEqual(pParams->pluginGUID, MSDK_PLUGINGUID_NULL))
//			{
//				mfxIMPL impl = pParams->bUseHWLib ? MFX_IMPL_HARDWARE : MFX_IMPL_SOFTWARE;
//				pParams->pluginGUID = msdkGetPluginUID(impl, MSDK_VDECODE, pParams->codecId);
//			}
//			if (!AreGuidsEqual(pParams->pluginGUID, MSDK_PLUGINGUID_NULL))
//			{
//				m_pPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, *m_mfxSession, pParams->pluginGUID, 1));
//				if (m_pPlugin.get() == NULL)
//					sts = MFX_ERR_UNSUPPORTED;
//			}
//			if (sts == MFX_ERR_UNSUPPORTED)
//				msdk_printf(MSDK_STRING("Default plugin cannot be loaded (possibly you have to define plugin explicitly)\n"));
//		}
//		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	}
//	m_handleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadEncLoop, (PVOID)this, 0, NULL);
//	MSDK_CHECK_POINTER(m_handleThread, MFX_ERR_NULL_PTR);
//	m_inputParams = pParams;
//	return MFX_ERR_NONE;
//}
//
//void CEncodingPipe::Release()
//{
//	if (m_handleThread)
//	{
//		m_bExit = true;
//		while (m_bExit)
//			continue;
//		CloseHandle(m_handleThread);
//		m_handleThread = NULL;
//	}
//	m_pPlugin.reset();
//	MSDK_SAFE_DELETE(m_mfxSession);
//	MSDK_SAFE_DELETE(m_pmfxEnc);
//	MSDK_SAFE_DELETE(m_pmfxVPP);
//	MSDK_SAFE_DELETE_ARRAY(m_MVCSeqDesc.View);
//	MSDK_SAFE_DELETE_ARRAY(m_MVCSeqDesc.ViewId);
//	MSDK_SAFE_DELETE_ARRAY(m_MVCSeqDesc.OP);
//	MSDK_SAFE_DELETE_ARRAY(m_VppDoNotUse.AlgList);
//	MSDK_SAFE_DELETE(m_pMFXAllocator);
//	MSDK_SAFE_DELETE(m_pmfxAllocatorParams);
//	MSDK_SAFE_DELETE(m_hwdev);
//	DeleteFrames();
//}
//
//mfxStatus CEncodingPipe::Connect(sStrQue* conn_handle, SourceType type /*= SourceType_Bitstream*/)
//{
//	MSDK_CHECK_POINTER(conn_handle, MFX_ERR_NULL_PTR);
//	m_listInput = conn_handle;
//	m_listInput_Type = type;
//	return MFX_ERR_NONE;
//}
//
//mfxStatus CEncodingPipe::InitEncoder()
//{
//	mfxStatus sts = MFX_ERR_NONE;
//	sts = InitMfxParams();
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	sts = CreateAllocator();
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	sts = InitMfxVppParams();
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	if (MVC_ENABLED & m_inputParams->MVC_flags)
//	{
//		sts = AllocAndInitMVCSeqDesc();
//		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	}
//	if (m_pmfxEnc == NULL)
//		m_pmfxEnc = new MFXVideoENCODE(*m_mfxSession);
//	MSDK_CHECK_POINTER(m_pmfxEnc, MFX_ERR_MEMORY_ALLOC);
//	// create preprocessor if resizing was requested from command line
//	// or if different FourCC is set in InitMfxVppParams
//	if (m_inputParams->nWidth != m_inputParams->nDstWidth ||
//		m_inputParams->nHeight != m_inputParams->nDstHeight ||
//		m_mfxVppParams.vpp.In.FourCC != m_mfxVppParams.vpp.Out.FourCC)
//	{
//		if (m_pmfxVPP == NULL)
//			m_pmfxVPP = new MFXVideoVPP(*m_mfxSession);
//		MSDK_CHECK_POINTER(m_pmfxVPP, MFX_ERR_MEMORY_ALLOC);
//	}
//	sts = ResetComponents(true);
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	return MFX_ERR_NONE;
//}
//
//mfxStatus CEncodingPipe::ResetComponents(bool alloc /*= false*/)
//{
//	m_bInitEncode = false;
//	mfxStatus sts = MFX_ERR_NONE;
//	MSDK_CHECK_POINTER(m_inputParams, MFX_ERR_NULL_PTR);
//	MSDK_CHECK_POINTER(m_pmfxEnc, MFX_ERR_NOT_INITIALIZED);
//	sts = m_pmfxEnc->Close();
//	MSDK_IGNORE_MFX_STS(sts, MFX_ERR_NOT_INITIALIZED);
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	if (m_pmfxVPP)
//	{
//		sts = m_pmfxVPP->Close();
//		MSDK_IGNORE_MFX_STS(sts, MFX_ERR_NOT_INITIALIZED);
//		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	}
//	if (alloc)
//	{
//		DeleteFrames();
//		sts = AllocFrames();
//		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	}
//	sts = m_pmfxEnc->Init(&m_mfxVideoParams);
//	if (MFX_WRN_PARTIAL_ACCELERATION == sts)
//		msdk_printf(MSDK_STRING("WARNING: encode partial acceleration\n"));
//	MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	if (m_pmfxVPP)
//	{
//		sts = m_pmfxVPP->Init(&m_mfxVppParams);
//		if (MFX_WRN_PARTIAL_ACCELERATION == sts)
//			msdk_printf(MSDK_STRING("WARNING: encode_vpp partial acceleration\n"));
//		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
//		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	}
//	m_bInitEncode = true;
//	return MFX_ERR_NONE;
//}
//
//mfxStatus CEncodingPipe::AllocFrames()
//{
//	MSDK_CHECK_POINTER(m_pmfxEnc, MFX_ERR_NOT_INITIALIZED);
//	mfxStatus sts = MFX_ERR_NONE;
//	mfxFrameAllocRequest EncRequest = { 0 };
//	mfxFrameAllocRequest VppRequest[2] = { 0 };// VppRequest[0] for input frames request, VppRequest[1] for output frames request
//	mfxU16 nEncSurfNum = 0; // number of surfaces for encoder
//	mfxU16 nVppSurfNum = 0; // number of surfaces for vpp
//
//	sts = m_pmfxEnc->QueryIOSurf(&m_mfxVideoParams, &EncRequest);
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	sts = EncRequest.NumFrameSuggested < m_mfxVideoParams.AsyncDepth ? MFX_ERR_MEMORY_ALLOC : sts;
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	nEncSurfNum = max(EncRequest.NumFrameSuggested, 1);
//	if (m_pmfxVPP)
//	{
//		sts = m_pmfxVPP->QueryIOSurf(&m_mfxVppParams, VppRequest);
//		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//		// The number of surfaces for vpp input
//		nVppSurfNum = max(VppRequest[0].NumFrameSuggested, 1);
//		// If surfaces are shared by 2 components, c1 and c2. NumSurf = c1_out + c2_in - AsyncDepth + 1
//		nEncSurfNum = nVppSurfNum + nEncSurfNum - m_mfxVideoParams.AsyncDepth + 1;
//	}
//	EncRequest.NumFrameMin = EncRequest.NumFrameSuggested = nEncSurfNum;
//	memcpy_s(&EncRequest.Info, sizeof(mfxFrameInfo), &m_mfxVideoParams.mfx.FrameInfo, sizeof(mfxFrameInfo));
//	EncRequest.Type = m_pmfxVPP ? EncRequest.Type | MFX_MEMTYPE_FROM_VPPOUT : EncRequest.Type;
//
//	// alloc frames 
//	sts = m_pMFXAllocator->Alloc(m_pMFXAllocator->pthis, &EncRequest, &m_Response);
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	if (m_pmfxVPP)
//	{
//		VppRequest[0].NumFrameSuggested = VppRequest[0].NumFrameMin = nVppSurfNum;
//		memcpy_s(&VppRequest[0].Info, sizeof(mfxFrameInfo), &m_mfxVppParams.mfx.FrameInfo, sizeof(mfxFrameInfo));
//		sts = m_pMFXAllocator->Alloc(m_pMFXAllocator->pthis, &VppRequest[0], &m_VppResponse);
//		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	}
//	// prepare mfxFrameSurface1 array
//	nEncSurfNum = max(m_Response.NumFrameActual, 1);
//	MSDK_SAFE_DELETE_ARRAY(m_pSurfaces);
//	m_pSurfaces = new mfxFrameSurface1[nEncSurfNum];
//	MSDK_CHECK_POINTER(m_pSurfaces, MFX_ERR_MEMORY_ALLOC);
//
//	mfxU16 width = (mfxU16)MSDK_ALIGN32(EncRequest.Info.Width);
//	mfxU16 height = (mfxU16)MSDK_ALIGN32(EncRequest.Info.Height);
//	mfxU32 size = width*height * 12 / 8;// NV12 format is a 12 bits per pixel format
//	mfxU8* surfaceBuf = new mfxU8[size*nEncSurfNum];
//	MSDK_CHECK_POINTER(surfaceBuf, MFX_ERR_MEMORY_ALLOC);
//	memset(surfaceBuf, 0, sizeof(mfxU8)*size*nEncSurfNum);
//	
//	vector<void*> vecT;
//	for (mfxU16 i = 0; i < nEncSurfNum; i++)
//	{
//		memset(&m_pSurfaces[i], 0, sizeof(mfxFrameSurface1));
//		memcpy_s(&m_pSurfaces[i].Info, sizeof(mfxFrameInfo), &m_mfxVideoParams.mfx.FrameInfo, sizeof(mfxFrameInfo));
//		m_pSurfaces[i].Data.Y = &surfaceBuf[size*i];
//		m_pSurfaces[i].Data.UV = m_pSurfaces[i].Data.Y + width*height;
//		m_pSurfaces[i].Data.Pitch = width;
//		m_pSurfaces[i].Info.CropW = width;
//		m_pSurfaces[i].Info.CropH = height;
//		
//		if (m_bExternalAlloc)
//			m_pSurfaces[i].Data.MemId = m_Response.mids[i];
//		else
//		{
//			// get YUV pointers
//			sts = m_pMFXAllocator->Lock(m_pMFXAllocator->pthis, m_Response.mids[i], &m_pSurfaces[i].Data);
//			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//		}
//		vecT.push_back(&m_pSurfaces[i]);
//	}
//	m_listLoop.initList(vecT);
//	m_listLoop.connect((sStrQueExt*)&m_listOut);
//	m_listOut.connect(&m_listLoop);
//	vecT.clear();
//
//	if (m_pmfxVPP)
//	{
//		nVppSurfNum = max(m_VppResponse.NumFrameActual, 1);
//		MSDK_SAFE_DELETE_ARRAY(m_pVppSurfaces);
//		m_pVppSurfaces = new mfxFrameSurface1[nVppSurfNum];
//		MSDK_CHECK_POINTER(m_pVppSurfaces, MFX_ERR_MEMORY_ALLOC);
//		memset(m_pVppSurfaces, 0, sizeof(mfxFrameSurface1)*nVppSurfNum);
//		for (mfxU16 i = 0; i < nVppSurfNum; i++)
//		{
//			memcpy_s(&m_pVppSurfaces[i].Info, sizeof(mfxFrameInfo), &m_mfxVppParams.mfx.FrameInfo, sizeof(mfxFrameInfo));
//			if (m_bExternalAlloc)
//				m_pVppSurfaces[i].Data.MemId = m_VppResponse.mids[i];
//			else
//			{
//				sts = m_pMFXAllocator->Lock(m_pMFXAllocator->pthis, m_VppResponse.mids[i], &m_pVppSurfaces[i].Data);
//				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//			}
//		}
//	}
//	return MFX_ERR_NONE;
//}
//
//void CEncodingPipe::DeleteFrames()
//{
//	MSDK_SAFE_DELETE_ARRAY(m_pSurfaces);
//	MSDK_SAFE_DELETE_ARRAY(m_pVppSurfaces);
//	if (m_pMFXAllocator)
//	{
//		m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_Response);
//		m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_VppResponse);
//	}
//}
//
//mfxStatus CEncodingPipe::InitMfxParams()
//{
//	MSDK_CHECK_POINTER(m_inputParams, MFX_ERR_NULL_PTR);
//	m_EncExtParams.clear();
//
//	m_mfxVideoParams.mfx.CodecId = m_inputParams->codecId;
//	m_mfxVideoParams.mfx.TargetUsage = m_inputParams->nTargetUsage;
//	m_mfxVideoParams.mfx.RateControlMethod = m_inputParams->nRateControlMethod;
//	m_mfxVideoParams.mfx.GopRefDist = m_inputParams->nGopRefDist;
//	m_mfxVideoParams.mfx.GopPicSize = m_inputParams->nGopPicSize;
//	m_mfxVideoParams.mfx.NumRefFrame = m_inputParams->nNumRefFrame == 0 ? 1 : m_inputParams->nNumRefFrame;
//	m_mfxVideoParams.mfx.IdrInterval = m_inputParams->nIdrInterval;
//
//	if (m_mfxVideoParams.mfx.RateControlMethod == MFX_RATECONTROL_CQP)
//	{
//		m_mfxVideoParams.mfx.QPI = m_inputParams->nQPI;
//		m_mfxVideoParams.mfx.QPP = m_inputParams->nQPP;
//		m_mfxVideoParams.mfx.QPB = m_inputParams->nQPB;
//	}
//	else
//	{
//		m_mfxVideoParams.mfx.TargetKbps = m_inputParams->nBitRate;
//	}
//	m_mfxVideoParams.mfx.LowPower = m_inputParams->nQSVFF;
//	m_mfxVideoParams.mfx.NumSlice = m_inputParams->nNumSlice;
//	ConvertFrameRate(m_inputParams->dFrameRate, &m_mfxVideoParams.mfx.FrameInfo.FrameRateExtN, &m_mfxVideoParams.mfx.FrameInfo.FrameRateExtD);
//	m_mfxVideoParams.mfx.EncodedOrder = 0; // binary flag, 0 signals encoder to take frames in display order
//	m_mfxVideoParams.IOPattern = m_inputParams->memType == enmMemType_System ? MFX_IOPATTERN_IN_SYSTEM_MEMORY : MFX_IOPATTERN_IN_VIDEO_MEMORY;
//	// frame info parameters
//	m_mfxVideoParams.mfx.FrameInfo.FourCC = m_inputParams->fourcc;
//	m_mfxVideoParams.mfx.FrameInfo.ChromaFormat = m_inputParams->chromaType;
//	m_mfxVideoParams.mfx.FrameInfo.PicStruct = m_inputParams->nPicStruct;
//	// In case of JPEG there's fine tuning of ColorFormat and Chrome format
//	if (MFX_CODEC_JPEG == m_inputParams->codecId)
//	{
//		if (MFX_FOURCC_RGB4 == m_inputParams->fourcc)
//		{
//			m_mfxVideoParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV444;
//			m_mfxVideoParams.mfx.FrameInfo.FourCC = MFX_FOURCC_RGB4;
//		}
//		else if (MFX_FOURCC_YUY2 == m_inputParams->fourcc)
//		{
//			m_mfxVideoParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV422;
//			m_mfxVideoParams.mfx.FrameInfo.FourCC = MFX_FOURCC_YUY2;
//		}
//	}
//	// set frame size and crops
//	if (m_inputParams->codecId == MFX_CODEC_HEVC && !memcmp(m_inputParams->pluginGUID.Data, MFX_PLUGINID_HEVCE_HW.Data, sizeof(MFX_PLUGINID_HEVCE_HW.Data)))
//	{
//		// In case of HW HEVC decoder width and height must be aligned to 32 pixels. 
//		//This limitation is planned to be removed in later versions of plugin
//		m_mfxVideoParams.mfx.FrameInfo.Width = MSDK_ALIGN32(m_inputParams->nDstWidth);
//		m_mfxVideoParams.mfx.FrameInfo.Height = MSDK_ALIGN32(m_inputParams->nDstHeight);
//	}
//	else
//	{
//		// width must be a multiple of 16
//		// height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
//		m_mfxVideoParams.mfx.FrameInfo.Width = MSDK_ALIGN16(m_inputParams->nDstWidth);
//		m_mfxVideoParams.mfx.FrameInfo.Height = (MFX_PICSTRUCT_PROGRESSIVE == m_mfxVideoParams.mfx.FrameInfo.PicStruct) ?
//			MSDK_ALIGN16(m_inputParams->nDstHeight) : MSDK_ALIGN32(m_inputParams->nDstHeight);
//	}
//	m_mfxVideoParams.mfx.FrameInfo.CropX = 0;
//	m_mfxVideoParams.mfx.FrameInfo.CropY = 0;
//	m_mfxVideoParams.mfx.FrameInfo.CropW = m_inputParams->nDstWidth;
//	m_mfxVideoParams.mfx.FrameInfo.CropH = m_inputParams->nDstHeight;
//	// we don't specify profile and level and let the encoder choose those basing on parameters
//	// we must specify profile only for MVC codec
//	m_mfxVideoParams.mfx.CodecProfile = MVC_ENABLED&m_inputParams->MVC_flags ? MFX_PROFILE_AVC_STEREO_HIGH : m_mfxVideoParams.mfx.CodecProfile;
//	if (MVC_ENABLED&m_inputParams->MVC_flags)
//		m_EncExtParams.push_back((mfxExtBuffer*)&m_MVCSeqDesc);// configure and attach external parameters
//	if (MVC_VIEWOUTPUT&m_inputParams->MVC_flags)
//	{
//		m_CodingOption.ViewOutput = MFX_CODINGOPTION_ON;// ViewOuput option requested
//		m_EncExtParams.push_back((mfxExtBuffer*)&m_CodingOption);
//	}
//	// configure the depth of the look ahead BRC if specified in command line
//	if (m_inputParams->nLADepth || m_inputParams->nMaxSliceSize || m_inputParams->nBRefType)
//	{
//		m_CodingOption2.LookAheadDepth = m_inputParams->nLADepth;
//		m_CodingOption2.MaxSliceSize = m_inputParams->nMaxSliceSize;
//		m_CodingOption2.BRefType = m_inputParams->nBRefType;
//		m_EncExtParams.push_back((mfxExtBuffer *)&m_CodingOption2);
//	}
//	// In case of HEVC when height and/or width divided with 8 but not divided with 16
//	// add extended parameter to increase performance
//	if ((!((m_mfxVideoParams.mfx.FrameInfo.CropW & 15) ^ 8) ||
//		!((m_mfxVideoParams.mfx.FrameInfo.CropH & 15) ^ 8)) &&
//		(m_mfxVideoParams.mfx.CodecId == MFX_CODEC_HEVC))
//	{
//		m_ExtHEVCParam.PicWidthInLumaSamples = m_mfxVideoParams.mfx.FrameInfo.CropW;
//		m_ExtHEVCParam.PicHeightInLumaSamples = m_mfxVideoParams.mfx.FrameInfo.CropH;
//		m_EncExtParams.push_back((mfxExtBuffer*)&m_ExtHEVCParam);
//	}
//	if (m_EncExtParams.size() > 0)
//	{
//		m_mfxVideoParams.ExtParam = &m_EncExtParams[0];
//		m_mfxVideoParams.NumExtParam = (mfxU16)m_EncExtParams.size();
//	}
//	// JPEG encoder settings overlap with other encoders settings in mfxInfoMFX structure
//	if (MFX_CODEC_JPEG == m_inputParams->codecId)
//	{
//		m_mfxVideoParams.mfx.Interleaved = 1;
//		m_mfxVideoParams.mfx.Quality = m_inputParams->nQuality;
//		m_mfxVideoParams.mfx.RestartInterval = 0;
//		MSDK_ZERO_MEMORY(m_mfxVideoParams.mfx.reserved5);
//	}
//	m_mfxVideoParams.AsyncDepth = m_inputParams->nAsyncDepth;
//	return MFX_ERR_NONE;
//}
//
//mfxStatus CEncodingPipe::CreateAllocator()
//{
//	mfxStatus sts = MFX_ERR_NONE;
//	if (m_inputParams->memType != enmMemType_System)
//	{
//#if D3D_SURFACES_SUPPORT  
//		sts = CreateHWDevice();
//		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//
//		mfxHDL hdl = NULL;
//		mfxHandleType hdl_t =
//#if MFX_D3D11_SUPPORT
//			enmMemType_D3D11 == m_inputParams->memType ? MFX_HANDLE_D3D11_DEVICE :
//#endif
//			MFX_HANDLE_D3D9_DEVICE_MANAGER;
//		sts = m_hwdev->GetHandle(hdl_t, &hdl);
//		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//		sts = m_mfxSession->SetHandle(hdl_t, hdl);
//		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//
//		// create D3D allocator
//#if MFX_D3D11_SUPPORT
//		if (enmMemType_D3D11 == m_inputParams->memType)
//		{
//			if (m_pMFXAllocator == NULL)
//			{
//				m_pMFXAllocator = new D3D11FrameAllocator;
//				MSDK_CHECK_POINTER(m_pMFXAllocator, MFX_ERR_MEMORY_ALLOC);
//			}
//			if (m_pmfxAllocatorParams == NULL)
//			{
//				D3D11AllocatorParams *pd3d11AllocParams = new D3D11AllocatorParams;
//				MSDK_CHECK_POINTER(pd3d11AllocParams, MFX_ERR_MEMORY_ALLOC);
//				pd3d11AllocParams->pDevice = reinterpret_cast<ID3D11Device *>(hdl);
//				m_pmfxAllocatorParams = pd3d11AllocParams;
//			}
//		}
//		else
//#endif 
//		{
//			if (m_pMFXAllocator == NULL)
//			{
//				m_pMFXAllocator = new D3DFrameAllocator;
//				MSDK_CHECK_POINTER(m_pMFXAllocator, MFX_ERR_MEMORY_ALLOC);
//			}
//			if (m_pmfxAllocatorParams == NULL)
//			{
//				D3DAllocatorParams *pd3dAllocParams = new D3DAllocatorParams;
//				MSDK_CHECK_POINTER(pd3dAllocParams, MFX_ERR_MEMORY_ALLOC);
//				pd3dAllocParams->pManager = reinterpret_cast<IDirect3DDeviceManager9 *>(hdl);
//				m_pmfxAllocatorParams = pd3dAllocParams;
//			}
//		}
//		/* In case of video memory we must provide MediaSDK with external allocator
//		thus we demonstrate "external allocator" usage model.
//		Call SetAllocator to pass allocator to mediasdk */
//		sts = m_mfxSession->SetFrameAllocator(m_pMFXAllocator);
//		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//		m_bExternalAlloc = true;
//#endif
//	}
//	else
//	{
//		// create system memory allocator       
//		if (m_pMFXAllocator == NULL)
//		{
//			m_pMFXAllocator = new SysMemFrameAllocator;
//			MSDK_CHECK_POINTER(m_pMFXAllocator, MFX_ERR_MEMORY_ALLOC);
//		}
//
//		/* In case of system memory we demonstrate "no external allocator" usage model.
//		We don't call SetAllocator, MediaSDK uses internal allocator.
//		We use system memory allocator simply as a memory manager for application*/
//	}
//	// initialize memory allocator
//	sts = m_pMFXAllocator->Init(m_pmfxAllocatorParams);
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//	return MFX_ERR_NONE;
//}
//
//mfxStatus CEncodingPipe::CreateHWDevice()
//{
//#if D3D_SURFACES_SUPPORT
//	mfxStatus sts = MFX_ERR_NONE;
//	if (m_hwdev == NULL)
//	{
//		//if (m_inputParams->memType == enmMemType_D3D9)
//		//	m_hwdev = new CD3D9Device();
//		if (m_inputParams->memType == enmMemType_D3D11)
//			m_hwdev = new CD3D11Device();
//		MSDK_CHECK_POINTER(m_hwdev, MFX_ERR_MEMORY_ALLOC);
//	}
//	sts = m_hwdev->Init(NULL, 0, GetMSDKAdapterNumber(*m_mfxSession));
//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
//#endif
//	return MFX_ERR_NONE;
//}
//
//mfxStatus CEncodingPipe::InitMfxVppParams()
//{
//	MSDK_CHECK_POINTER(m_inputParams, MFX_ERR_NULL_PTR);
//	m_VppExtParams.clear();
//	if (m_inputParams->memType==enmMemType_System)
//		m_mfxVppParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
//	else
//		m_mfxVppParams.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
//	m_mfxVppParams.vpp.In.FourCC = m_inputParams->fourcc;
//	m_mfxVppParams.vpp.In.PicStruct = m_inputParams->nPicStruct;;
//	ConvertFrameRate(m_inputParams->dFrameRate, &m_mfxVppParams.vpp.In.FrameRateExtN, &m_mfxVppParams.vpp.In.FrameRateExtD);
//	// width must be a multiple of 16
//	// height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
//	m_mfxVppParams.vpp.In.Width = MSDK_ALIGN16(m_inputParams->nWidth);
//	m_mfxVppParams.vpp.In.Height = (MFX_PICSTRUCT_PROGRESSIVE == m_mfxVppParams.vpp.In.PicStruct) ?
//		MSDK_ALIGN16(m_inputParams->nHeight) : MSDK_ALIGN32(m_inputParams->nHeight);
//	// set crops in input mfxFrameInfo for correct work of file reader
//	// VPP itself ignores crops at initialization
//	m_mfxVppParams.vpp.In.CropW = m_inputParams->nWidth;
//	m_mfxVppParams.vpp.In.CropH = m_inputParams->nHeight;
//	// fill output frame info
//	MSDK_MEMCPY_VAR(m_mfxVppParams.vpp.Out, &m_mfxVppParams.vpp.In, sizeof(mfxFrameInfo));
//	// only resizing is supported
//	m_mfxVppParams.vpp.Out.Width = MSDK_ALIGN16(m_inputParams->nDstWidth);
//	m_mfxVppParams.vpp.Out.Height = (MFX_PICSTRUCT_PROGRESSIVE == m_mfxVppParams.vpp.Out.PicStruct) ?
//		MSDK_ALIGN16(m_inputParams->nDstHeight) : MSDK_ALIGN32(m_inputParams->nDstHeight);
//	// configure and attach external parameters
//	AllocAndInitVppDoNotUse();
//	m_VppExtParams.push_back((mfxExtBuffer *)&m_VppDoNotUse);
//
//	if (MVC_ENABLED & m_inputParams->MVC_flags)
//		m_VppExtParams.push_back((mfxExtBuffer *)&m_MVCSeqDesc);
//	m_mfxVppParams.ExtParam = &m_VppExtParams[0]; 
//	m_mfxVppParams.NumExtParam = (mfxU16)m_VppExtParams.size();
//	m_mfxVppParams.AsyncDepth = m_inputParams->nAsyncDepth;
//	return MFX_ERR_NONE;
//}
//
//mfxStatus CEncodingPipe::AllocAndInitVppDoNotUse()
//{
//	m_VppDoNotUse.NumAlg = 4;
//	MSDK_SAFE_DELETE_ARRAY(m_VppDoNotUse.AlgList);
//	m_VppDoNotUse.AlgList = new mfxU32[m_VppDoNotUse.NumAlg];
//	MSDK_CHECK_POINTER(m_VppDoNotUse.AlgList, MFX_ERR_MEMORY_ALLOC);
//	m_VppDoNotUse.AlgList[0] = MFX_EXTBUFF_VPP_DENOISE; // turn off denoising (on by default)
//	m_VppDoNotUse.AlgList[1] = MFX_EXTBUFF_VPP_SCENE_ANALYSIS; // turn off scene analysis (on by default)
//	m_VppDoNotUse.AlgList[2] = MFX_EXTBUFF_VPP_DETAIL; // turn off detail enhancement (on by default)
//	m_VppDoNotUse.AlgList[3] = MFX_EXTBUFF_VPP_PROCAMP; // turn off processing amplified (on by default)
//	return MFX_ERR_NONE;
//}
//
//mfxStatus CEncodingPipe::AllocAndInitMVCSeqDesc()
//{
//	// a simple example of mfxExtMVCSeqDesc structure filling
//	// actually equal to the "Default dependency mode" - when the structure fields are left 0,
//	// but we show how to properly allocate and fill the fields
//	// mfxMVCViewDependency array
//	m_MVCSeqDesc.NumView = m_inputParams->numViews;
//	m_MVCSeqDesc.NumViewAlloc = m_inputParams->numViews;
//	MSDK_SAFE_DELETE_ARRAY(m_MVCSeqDesc.View);
//	m_MVCSeqDesc.View = new mfxMVCViewDependency[m_MVCSeqDesc.NumViewAlloc];
//	MSDK_CHECK_POINTER(m_MVCSeqDesc.View, MFX_ERR_MEMORY_ALLOC);
//	for (mfxU32 i = 0; i < m_MVCSeqDesc.NumViewAlloc; ++i)
//	{
//		MSDK_ZERO_MEMORY(m_MVCSeqDesc.View[i]);
//		m_MVCSeqDesc.View[i].ViewId = (mfxU16)i; // set view number as view id
//	}
//
//	// set up dependency for second view
//	m_MVCSeqDesc.View[1].NumAnchorRefsL0 = 1;
//	m_MVCSeqDesc.View[1].AnchorRefL0[0] = 0;     // ViewId 0 - base view
//
//	m_MVCSeqDesc.View[1].NumNonAnchorRefsL0 = 1;
//	m_MVCSeqDesc.View[1].NonAnchorRefL0[0] = 0;  // ViewId 0 - base view
//	// viewId array
//	m_MVCSeqDesc.NumViewId = m_inputParams->numViews;
//	m_MVCSeqDesc.NumViewIdAlloc = m_inputParams->numViews;
//	MSDK_SAFE_DELETE_ARRAY(m_MVCSeqDesc.ViewId);
//	m_MVCSeqDesc.ViewId = new mfxU16[m_MVCSeqDesc.NumViewIdAlloc];
//	MSDK_CHECK_POINTER(m_MVCSeqDesc.ViewId, MFX_ERR_MEMORY_ALLOC);
//	for (mfxU32 i = 0; i < m_MVCSeqDesc.NumViewIdAlloc; ++i)
//	{
//		m_MVCSeqDesc.ViewId[i] = (mfxU16)i;
//	}
//	// create a single operation point containing all views
//	m_MVCSeqDesc.NumOP = 1;
//	m_MVCSeqDesc.NumOPAlloc = 1;
//	MSDK_SAFE_DELETE_ARRAY(m_MVCSeqDesc.OP);
//	m_MVCSeqDesc.OP = new mfxMVCOperationPoint[m_MVCSeqDesc.NumOPAlloc];
//	MSDK_CHECK_POINTER(m_MVCSeqDesc.OP, MFX_ERR_MEMORY_ALLOC);
//	for (mfxU32 i = 0; i < m_MVCSeqDesc.NumOPAlloc; ++i)
//	{
//		MSDK_ZERO_MEMORY(m_MVCSeqDesc.OP[i]);
//		m_MVCSeqDesc.OP[i].NumViews = m_inputParams->numViews;
//		m_MVCSeqDesc.OP[i].NumTargetViews = m_inputParams->numViews;
//		m_MVCSeqDesc.OP[i].TargetViewId = m_MVCSeqDesc.ViewId; // points to mfxExtMVCSeqDesc::ViewId
//	}
//	return MFX_ERR_NONE;
//}
//
//void CEncodingPipe::CopySurfaceToBitstream(mfxFrameSurface1& surface, mfxBitstream& bitstream)
//{
//
//}
//
//void CEncodingPipe::EncodeLoop()
//{
//	mfxStatus sts = MFX_ERR_NONE;
//	mfxFrameSurface1 *in_sourface = NULL;
//	mfxBitstream *in_bitstream = NULL;
//
//	while (!m_bExit)
//	{
//		if (m_listInput==NULL)
//			continue;
//		if (!m_bInitEncode)
//		{
//			sts = InitEncoder();
//			if (MFX_ERR_NONE!=sts)
//				continue;
//		}
//		switch (m_listInput_Type)
//		{
//		case SourceType_Bitstream:
//		{
//			in_bitstream = (mfxBitstream*)m_listInput->queGet();
//			if (in_bitstream==NULL)
//				continue;
//			break;
//		}
//		case SourceType_Surface_Dec:
//		case SourceType_Surface_Vpp:
//		case SourceType_Surface_Enc:
//		{
//			in_sourface = (mfxFrameSurface1*)m_listInput->queGet();
//			if (in_sourface==NULL)
//				continue;
//			break;
//		}
//		default:
//		{
//			in_sourface = NULL;
//			in_bitstream = NULL;
//			break;
//		}
//		}
//
//		sts = RunEncoding(in_sourface);
//		{
//			static int time1 = 0, fps = 0;
//			int time2 = GetTickCount();
//			time1 = time1 == 0 ? time2 : time1;
//			if (time2 - time1 >= 1000)
//			{
//				printf("Encode-------------FPS=%d\n", fps);
//				fps = 0;
//				time1 = time2;
//			}
//			fps += 1;
//		}
//		if (in_bitstream)
//			m_listInput->quePut(in_bitstream);
//		else if (in_sourface)
//			m_listInput->quePut(in_sourface);
//		in_bitstream = NULL;
//		in_sourface = NULL;
//	}
//	m_bExit = false;
//}
//
///*ÏàÁÚ·¨*/
//mfxStatus CEncodingPipe::sclrLink_sclrProcess(mfxFrameSurface1 *src, mfxFrameSurface1 *dst)
//{
//	if (NULL == src || NULL == dst)
//		return MFX_ERR_NULL_PTR;
//
//	mfxStatus status = MFX_ERR_NONE;
//
//	mfxFrameInfo& pSrcInfo = src->Info;
//	mfxFrameData& pSrcData = src->Data;
//	mfxFrameInfo& pDstInfo = dst->Info;
//	mfxFrameData& pDstData = dst->Data;
//	unsigned int x0Start = pSrcInfo.CropX;
//	unsigned int y0Start = pSrcInfo.CropY;
//	unsigned int w0 = pSrcInfo.CropW;
//	unsigned int h0 = pSrcInfo.CropH;
//	unsigned int pitch0 = pSrcData.Pitch;
//
//	//pthread_mutex_lock(&info->lock);
//	unsigned int x1Start = 0;
//	unsigned int y1Start = 0;
//	unsigned int w1 = pSrcInfo.Width;
//	unsigned int h1 = pSrcInfo.Height;
//	//pthread_mutex_unlock(&info->lock);
//
//	unsigned int pitch1 = pDstData.Pitch;
//	unsigned char* pSrcY = pSrcData.Y;
//	unsigned char* pSrcY1 = pSrcData.Y;
//	unsigned char* pSrcUV = pSrcData.UV;
//	unsigned char* pSrcUV1 = pSrcData.UV;
//	unsigned char* pDstY = pDstData.Y;
//	unsigned char* pDstUV = pDstData.UV;
//	unsigned char* p1Y = NULL;
//	unsigned char* p1UV = NULL;
//	float fw = float(w0 - 1) / (w1 - 1);
//	float fh = float(h0 - 1) / (h1 - 1);
//	float x0, y0;
//	unsigned int y1, x1;
//
//	if ((x1Start + w1) > 1920 || (y1Start + h1) > 1080)
//	{
//		//		nslog(NS_ERROR,"x:%d,y:%d,w:%d,h:%d,\n",x1Start,y1Start,w1,h1);
//		return MFX_ERR_NULL_PTR;
//	}
//
//	unsigned int* arr_Yx = new unsigned int[1920];
//	unsigned int* arr_Yy = new unsigned int[1080];
//
//	unsigned int* arr_UVx = new unsigned int[1920];
//	unsigned int* arr_UVy = new unsigned int[1080];
//
//	if (arr_Yx == NULL || arr_Yy == NULL || arr_UVx == NULL || arr_UVy == NULL)
//	{
//		int x = 0;
//	}
//
//	pDstInfo.CropX = x1Start;
//	pDstInfo.CropY = y1Start;
//	pDstInfo.CropW = w1;
//	pDstInfo.CropH = h1;
//
//	for (unsigned int x = 0; x < (0 + w1); x++)
//	{
//		x0 = x*fw;
//		arr_Yx[x] = int(x0 + 0.5);
//
//		unsigned int tmpx = (x >> 1);
//		x0 = tmpx*fw;
//		arr_UVx[x] = int(x0 + 0.5);
//	}
//
//	for (unsigned int y = 0; y < (0 + h1); y++)
//	{
//		y0 = y*fh;
//		arr_Yy[y] = int(y0 + 0.5);
//		if ((y % 2) == 0)
//		{
//			unsigned int tmpY = (y >> 1);
//			y0 = tmpY*fh;
//			arr_UVy[tmpY] = int(y0 + 0.5);
//		}
//	}
//
//	for (unsigned int y = y1Start; y < (y1Start + h1); y++)
//	{
//		y1 = arr_Yy[y - y1Start];
//		p1Y = pDstY + y*pitch1 + x1Start;
//		pSrcY1 = pSrcY + pitch0*(y1 + y0Start) + x0Start;
//
//		if (((y - y1Start) % 2) == 0)
//		{
//			unsigned int tmpY = ((y - y1Start) >> 1);
//			y1 = arr_UVy[tmpY];
//			p1UV = pDstUV + (y >> 1)*pitch1 + x1Start;
//			pSrcUV1 = pSrcUV + pitch0*(y1 + (y0Start >> 1)) + x0Start;
//		}
//
//		for (unsigned int x = x1Start; x < (x1Start + w1); x++)
//		{
//			x1 = arr_Yx[x - x1Start];
//
//			unsigned char* p11 = pSrcY1 + x1;
//			*p1Y = (unsigned char)(*p11);
//			p1Y++;
//
//			if (((y - y1Start) % 2) == 0)
//			{
//				x1 = arr_UVx[x - x1Start];
//
//				p11 = pSrcUV1 + (x1 << 1);
//				if (((x - x1Start) % 2) != 0)
//				{
//					p11 = p11 + 1;
//				}
//
//				*p1UV = (unsigned char)(*p11);
//				p1UV++;
//			}
//		}
//
//	}
//
//	delete[]arr_Yx;
//	delete[]arr_Yy;
//
//	delete[]arr_UVx;
//	delete[]arr_UVy;
//
//	return status;
//}
//
//mfxStatus CEncodingPipe::RunEncoding(mfxFrameSurface1* surface)
//{
//	static mfxStatus sts = MFX_ERR_NONE;
//	static mfxBitstream* out_bs = NULL;
//	static mfxSyncPoint sync_point = NULL;
//	mfxU16 index = GetFreeSurface(m_pSurfaces, m_Response.NumFrameActual);
//	if (out_bs == NULL)
//	{
//		out_bs = new mfxBitstream;
//		MSDK_CHECK_POINTER(out_bs, MFX_ERR_NULL_PTR);
//		memset(out_bs, 0, sizeof(mfxBitstream));
//		InitMfxBitstream(out_bs, 1920*1080);
//	}
//	//sts = sclrLink_sclrProcess(surface, &m_pSurfaces[index]);
//	//m_pSurfaces[index].Data.Y = surface->Data.Y;
//	//m_pSurfaces[index].Data.UV = surface->Data.UV;
//
//	while (!m_bExit)
//	{
//		//sts = m_pmfxEnc->EncodeFrameAsync(NULL, &m_pSurfaces[index], out_bs, &sync_point);
//		sts = m_pmfxEnc->EncodeFrameAsync(NULL, surface, out_bs, &sync_point);
//		sts = MFX_ERR_NONE == sts ? m_mfxSession->SyncOperation(sync_point, MSDK_DEC_WAIT_INTERVAL) : sts;	
//		if (MFX_ERR_NONE < sts && !sync_point)
//		{
//			if (MFX_WRN_DEVICE_BUSY == sts)
//				MSDK_SLEEP(1); // wait if device is busy
//		}
//		else if (MFX_ERR_NONE < sts && sync_point)
//		{
//			sts = MFX_ERR_NONE;
//			break;
//		}
//		else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)
//		{
//			out_bs->DataLength = out_bs->DataOffset = 0;
//			sts = MFX_ERR_NONE;
//			break;
//		}
//		else if (MFX_ERR_NONE == sts)
//		{
//			mfxU8* pData = out_bs->Data + out_bs->DataOffset;
//			pData += 10;
//			if (((*pData) & 0x7) == 0x7)
//				int keyFrame = 1;
//
//			static FILE* file = fopen("C:/Users/XJ/Desktop/yuv.yuv", "w+");
//			fwrite(out_bs->Data + out_bs->DataOffset, out_bs->DataLength, 1, file);
//			fflush(file);
//			out_bs->DataLength = out_bs->DataOffset = 0;
//			break;
//		}
//		else
//		{
//			break;
//		}
//	}
//	sts = MFX_ERR_NONE;
//	return MFX_ERR_NONE;
//}