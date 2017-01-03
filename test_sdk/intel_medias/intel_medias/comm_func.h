#pragma once

#include "mfxvideo++.h"
#include "sample_defs.h"
#include "comm_def.h"
#include <psapi.h>

//static MFXVideoSession* intelmedia_session_create(sInputParams *pParams)
//{
//	static MFXVideoSession* session = NULL;
//	if (pParams)
//	{
//		session = new MFXVideoSession;
//		mfxStatus sts = MFX_ERR_NONE;
//		mfxInitParam initPar;
//		mfxExtThreadsParam threadsPar;
//		mfxExtBuffer* extBufs[1];
//		mfxVersion version;
//		bool needInitExtPar = false;
//
//		MSDK_ZERO_MEMORY(initPar);
//		MSDK_ZERO_MEMORY(threadsPar);
//
//		// we set version to 1.0 and later we will query actual version of the library which will got leaded
//		initPar.Version.Major = 1;
//		initPar.Version.Minor = 1;//1.1°æ±¾
//
//		init_ext_buffer(threadsPar);
//
//		if (pParams->nThreadsNum)
//		{
//			threadsPar.NumThread = pParams->nThreadsNum;
//			needInitExtPar = true;
//		}
//		if (pParams->SchedulingType)
//		{
//			threadsPar.SchedulingType = pParams->SchedulingType;
//			needInitExtPar = true;
//		}
//		if (pParams->Priority)
//		{
//			threadsPar.Priority = pParams->Priority;
//			needInitExtPar = true;
//		}
//		if (needInitExtPar)
//		{
//			extBufs[0] = (mfxExtBuffer*)&threadsPar;
//			initPar.ExtParam = extBufs;
//			initPar.NumExtParam = 1;
//		}
//		initPar.GPUCopy = pParams->gpuCopy;
//
//		// Init session
//		if (pParams->bUseHWLib)
//		{
//			initPar.Implementation = MFX_IMPL_HARDWARE_ANY;
//			// if d3d11 surfaces are used ask the library to run acceleration through D3D11
//			// feature may be unsupported due to OS or MSDK API version
//			if (enmMemType_D3D11 == pParams->memType)
//				initPar.Implementation |= MFX_IMPL_VIA_D3D11;
//			sts = session->InitEx(initPar);
//			// MSDK API version may not support multiple adapters - then try initialize on the default
//			if (MFX_ERR_NONE != sts)
//			{
//				initPar.Implementation = (initPar.Implementation & !MFX_IMPL_HARDWARE_ANY) | MFX_IMPL_HARDWARE;
//				sts = session->InitEx(initPar);
//			}
//		}
//		else
//		{
//			initPar.Implementation = MFX_IMPL_SOFTWARE;
//			sts = session->InitEx(initPar);
//		}
//		if (sts == MFX_ERR_NONE)
//		{
//			sts = MFXQueryVersion(*session, &version); // get real API version of the loaded library
//			if (sts == MFX_ERR_NONE)
//			{
//				if (pParams->MVC_flags&MVC_ENABLED && !CheckVersion(&version, MSDK_FEATURE_MVC))
//				{
//					msdk_printf(MSDK_STRING("error: MVC is not supported in the %d.%d API version\n"), version.Major, version.Minor);
//					goto fail;
//				}
//				if (pParams->MVC_flags&MVC_VIEWOUTPUT && !CheckVersion(&version, MSDK_FEATURE_MVC_VIEWOUTPUT))
//				{
//					msdk_printf(MSDK_STRING("error: MVC Viewoutput is not supported in the %d.%d API version\n"), version.Major, version.Minor);
//					goto fail;
//				}
//				if ((pParams->codecId == MFX_CODEC_JPEG) && !CheckVersion(&version, MSDK_FEATURE_JPEG_DECODE))
//				{
//					msdk_printf(MSDK_STRING("error: Jpeg is not supported in the %d.%d API version\n"), version.Major, version.Minor);
//					goto fail;
//				}
//				if ((pParams->codecId == MFX_CODEC_JPEG) && !CheckVersion(&version, MSDK_FEATURE_JPEG_ENCODE)) {
//					msdk_printf(MSDK_STRING("error: Jpeg is not supported in the %d.%d API version\n"),	version.Major, version.Minor);
//					goto fail;
//				}
//				if (pParams->bLowLat && !CheckVersion(&version, MSDK_FEATURE_LOW_LATENCY))
//				{
//					msdk_printf(MSDK_STRING("error: Low Latency mode is not supported in the %d.%d API version\n"), version.Major, version.Minor);
//					goto fail;
//				}
//				if ((pParams->nRateControlMethod == MFX_RATECONTROL_LA) && !CheckVersion(&version, MSDK_FEATURE_LOOK_AHEAD)) {
//					msdk_printf(MSDK_STRING("error: Look ahead is not supported in the %d.%d API version\n"), version.Major, version.Minor);
//					goto fail;
//				}
//				return session;
//			}
//		}
//	fail:
//		session->Close();
//		delete session;
//		session = NULL;
//	}
//	return session;
//}

class BitstreamProcessor
{
public:
	BitstreamProcessor() {}
	virtual ~BitstreamProcessor() {}
	virtual mfxStatus PrepareBitstream() = 0;
	virtual mfxStatus GetInputBitstream(mfxBitstream **pBitstream) = 0;
	virtual mfxStatus ProcessOutputBitstream(mfxBitstream* pBitstream) = 0;
};

class FileBitstreamProcessor : public BitstreamProcessor
{
public:
	FileBitstreamProcessor()
	{
		MSDK_ZERO_MEMORY(m_Bitstream);
		m_Bitstream.TimeStamp = (mfxU64)-1;
	}
	virtual ~FileBitstreamProcessor()
	{
		if (m_pFileReader.get())
			m_pFileReader->Close();
		if (m_pFileWriter.get())
			m_pFileWriter->Close();
		WipeMfxBitstream(&m_Bitstream);
	}
	virtual mfxStatus Init(msdk_char  *pStrSrcFile, msdk_char  *pStrDstFile)
	{
		mfxStatus sts;
		if (pStrSrcFile)
		{
			m_pFileReader.reset(new CSmplBitstreamReader());
			sts = m_pFileReader->Init(pStrSrcFile);
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}

		if (pStrDstFile && *pStrDstFile)
		{
			m_pFileWriter.reset(new CSmplBitstreamWriter);
			sts = m_pFileWriter->Init(pStrDstFile);
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}

		sts = InitMfxBitstream(&m_Bitstream, 1024 * 1024);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		return MFX_ERR_NONE;
	}
	virtual mfxStatus PrepareBitstream() { return MFX_ERR_NONE; }
	virtual mfxStatus GetInputBitstream(mfxBitstream **pBitstream)
	{
		mfxStatus sts = m_pFileReader->ReadNextFrame(&m_Bitstream);
		if (MFX_ERR_NONE == sts)
		{
			*pBitstream = &m_Bitstream;
			return sts;
		}
		return sts;
	}
	virtual mfxStatus ProcessOutputBitstream(mfxBitstream* pBitstream)
	{
		if (m_pFileWriter.get())
			return m_pFileWriter->WriteNextFrame(pBitstream, false);
		else
			return MFX_ERR_NONE;
	}

protected:
	std::auto_ptr<CSmplBitstreamReader> m_pFileReader;
	std::auto_ptr<CSmplBitstreamWriter> m_pFileWriter;
	mfxBitstream m_Bitstream;
private:
	DISALLOW_COPY_AND_ASSIGN(FileBitstreamProcessor);
};



class ExtendedBSStore
{
public:
	explicit ExtendedBSStore(mfxU32 size)
	{
		m_pExtBS.resize(size);
	}
	virtual ~ExtendedBSStore()
	{
		for (mfxU32 i = 0; i < m_pExtBS.size(); i++)
			MSDK_SAFE_DELETE_ARRAY(m_pExtBS[i].Bitstream.Data);
		m_pExtBS.clear();

	}
	ExtendedBS* GetNext()
	{
		for (mfxU32 i = 0; i < m_pExtBS.size(); i++)
		{
			if (m_pExtBS[i].IsFree)
			{
				m_pExtBS[i].IsFree = false;
				return &m_pExtBS[i];
			}
		}
		return NULL;
	}
	void Release(ExtendedBS* pBS)
	{
		for (mfxU32 i = 0; i < m_pExtBS.size(); i++)
		{
			if (&m_pExtBS[i] == pBS)
			{
				m_pExtBS[i].IsFree = true;
				m_pExtBS[i].Surface1 = m_pExtBS[i].Surface2 = NULL;
				return;
			}
		}
		return;
	}
protected:
	std::vector<ExtendedBS> m_pExtBS;

private:
	DISALLOW_COPY_AND_ASSIGN(ExtendedBSStore);
};







static mfxU16 FourCCToChroma(mfxU32 fourCC)
{
	switch (fourCC)
	{
	case MFX_FOURCC_NV12:
	case MFX_FOURCC_P010:
		return MFX_CHROMAFORMAT_YUV420;
	case MFX_FOURCC_NV16:
	case MFX_FOURCC_P210:
	case MFX_FOURCC_YUY2:
		return MFX_CHROMAFORMAT_YUV422;
	case MFX_FOURCC_RGB4:
		return MFX_CHROMAFORMAT_YUV444;
	}
	return MFX_CHROMAFORMAT_YUV420;
}

static void SumAllocRequest(mfxFrameAllocRequest  &curReq, mfxFrameAllocRequest  &newReq)
{
	curReq.NumFrameSuggested = curReq.NumFrameSuggested + newReq.NumFrameSuggested;
	curReq.NumFrameMin = curReq.NumFrameSuggested;
	curReq.Type = curReq.Type | newReq.Type;

	if ((curReq.Type & MFX_MEMTYPE_SYSTEM_MEMORY) && ((curReq.Type & 0xf0) != MFX_MEMTYPE_SYSTEM_MEMORY))
		curReq.Type = (mfxU16)(curReq.Type & (~MFX_MEMTYPE_SYSTEM_MEMORY));
	if ((curReq.Type & MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET) && ((curReq.Type & 0xf0) != MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET))
		curReq.Type = (mfxU16)(curReq.Type & (~MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET));

	if (curReq.Info.Width == 0)
	{
		curReq.Info = newReq.Info;
	}
	else
	{
		curReq.Info.Width = curReq.Info.Width < newReq.Info.Width ? newReq.Info.Width : curReq.Info.Width;
		curReq.Info.Height = curReq.Info.Height < newReq.Info.Height ? newReq.Info.Height : curReq.Info.Height;
	}
}

static mfxStatus CorrectAsyncDepth(mfxFrameAllocRequest &curReq, mfxU16 asyncDepth)
{
	mfxStatus sts = MFX_ERR_NONE;
	if (curReq.NumFrameSuggested < asyncDepth)
	{
		sts = MFX_ERR_MEMORY_ALLOC;
	}
	else
	{
		// The request holds summary of required surfaces numbers from 2 components and
		// asyncDepth is included twice. Here we patch surfaces number removing
		// one asyncDepth.
		curReq.NumFrameSuggested = curReq.NumFrameSuggested - asyncDepth;
		curReq.NumFrameMin = curReq.NumFrameSuggested;
	}
	return sts;
}

// This function either performs synchronization using provided syncpoint, or just waits for predefined time if syncpoint is already 0 (this usually happens if syncpoint was already processed)
static void WaitForDeviceToBecomeFree(MFXVideoSession& session, mfxSyncPoint& syncPoint, mfxStatus& currentStatus)
{
	// Wait 1ms will be probably enough to device release
	if (syncPoint) {
		mfxStatus stsSync = session.SyncOperation(syncPoint, DEVICE_WAIT_TIME);
		if (MFX_ERR_NONE == stsSync) {
			// retire completed sync point (otherwise we may start active polling)
			syncPoint = NULL;
		}
		else if (stsSync < 0) {
			currentStatus = stsSync;
		}
	}
	else {
		MSDK_SLEEP(DEVICE_WAIT_TIME);
	}
}

static void SetSurfaceAuxIDR(ExtendedSurface& extSurface, PreEncAuxBuffer* encAuxCtrl, bool bInsertIDR)
{
	if (bInsertIDR)
	{
		if (extSurface.pCtrl)
			extSurface.pCtrl->encCtrl.FrameType = MFX_FRAMETYPE_I | MFX_FRAMETYPE_IDR | MFX_FRAMETYPE_REF;
		else
			extSurface.pCtrl = encAuxCtrl;
	}
	else
	{
		if (extSurface.pCtrl)
		{
			if (extSurface.pCtrl != encAuxCtrl)
				extSurface.pCtrl->encCtrl.FrameType = 0;
			else
				extSurface.pCtrl = NULL;
		}
	}
}

static bool PrintDllInfo(msdk_char* buf, mfxU32 buf_size, sInputParams* pParams)
{
#if defined(_WIN32) || defined(_WIN64)
	HANDLE   hCurrent = GetCurrentProcess();
	HMODULE *pModules;
	DWORD    cbNeeded;
	int      nModules;
	if (NULL == EnumProcessModules(hCurrent, NULL, 0, &cbNeeded))
		return false;

	nModules = cbNeeded / sizeof(HMODULE);

	pModules = new HMODULE[nModules];
	if (NULL == pModules)
	{
		return false;
	}
	if (NULL == EnumProcessModules(hCurrent, pModules, cbNeeded, &cbNeeded))
	{
		delete[]pModules;
		return false;
	}

	for (int i = 0; i < nModules; i++)
	{
		GetModuleFileName(pModules[i], buf, buf_size);
		if (_tcsstr(buf, MSDK_STRING("libmfxhw")) && pParams->bUseHW)
		{
			delete[]pModules;
			return true;
		}
		else if (_tcsstr(buf, MSDK_STRING("libmfxsw")) && !pParams->bUseHW)
		{
			delete[]pModules;
			return true;
		}

	}
	delete[]pModules;
	return false;
#else
	return false;
#endif
}

static void PrintInfo(mfxU32 session_number, sInputParams* pParams, mfxVersion *pVer)
{
	msdk_char buf[2048];
	MSDK_CHECK_POINTER_NO_RET(pVer);

	msdk_printf(MSDK_STRING("MFX %s Session %d API ver %d.%d parameters: \n"),
		!pParams->bUseHW ? MSDK_STRING("SOFTWARE") : MSDK_STRING("HARDWARE"),
		session_number,
		pVer->Major,
		pVer->Minor);

	if (0 == pParams->DecodeId)
		msdk_printf(MSDK_STRING("Input  video: From parent session\n"));
	else
		msdk_printf(MSDK_STRING("Input  video: %s\n"), CodecIdToStr(pParams->DecodeId).c_str());

	// means that source is parent session
	if (0 == pParams->EncodeId)
		msdk_printf(MSDK_STRING("Output video: To child session\n"));
	else
		msdk_printf(MSDK_STRING("Output video: %s\n"), CodecIdToStr(pParams->EncodeId).c_str());
	if (PrintDllInfo(buf, MSDK_ARRAY_LEN(buf), pParams))
		msdk_printf(MSDK_STRING("MFX dll: %s\n"), buf);
	msdk_printf(MSDK_STRING("\n"));
}

