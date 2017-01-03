#pragma once
#include "windows.h"
#include <d3d11.h>
#include <string>
#include <map>
#include <vector>
#include <queue>
using namespace std;

#include <mfxdefs.h>
#include <mfxplugin.h>
#include <mfxjpeg.h>
#include <mfxstructures.h>
#include <mfxenc.h>


typedef void* handle_t;
typedef void(*intelmedia_decoder_tex2d_callback)(void* decoder_handle, ID3D11Texture2D* tex2d, void* reserve);
#define MFX_FOURCC_DUMP MFX_MAKEFOURCC('D','U','M','P')
#define MSDK_FILENAME_MAXLEN 1024
// 1 ms provides better result in range [0..5] ms
#define DEVICE_WAIT_TIME 1
#define TIME_TO_SLEEP  1
#define MSDK_SURFACE_WAIT_INTERVAL 20000
#define MSDK_DEVICE_FREE_WAIT_INTERVAL 30000

enum enmMemType 
{
	enmMemType_System = 0x00,
	enmMemType_D3D9 =  0x01, 
	enmMemType_D3D11 = 0x02,
};

enum enmPluginType
{
	enmPluginType_Null=0,
	enmPluginType_GUID=1,
	enmPluginType_File=2,
};

enum SourceType
{
	SourceType_Null,
	SourceType_Bitstream,
	SourceType_Surface_Dec,
	SourceType_Surface_Vpp,
	SourceType_Surface_Enc,
};

enum 
{
	MVC_DISABLED = 0x0,
	MVC_ENABLED = 0x1,
	MVC_VIEWOUTPUT = 0x2,    // 2 output bitstreams
};

enum enmInputParams
{
	enmInputParams_Base,
	enmInputParams_Dec,
	enmInputParams_Enc,
};

enum PipelineMode
{
	Native = 0,        // means that pipeline is based depends on the cmd parameters (decode/encode/transcode)
	Sink,              // means that pipeline makes decode only and put data to shared buffer
	Source,            // means that pipeline makes vpp + encode and get data from shared buffer
	VppComp,           // means that pipeline makes vpp composition + encode and get data from shared buffer
	VppCompOnly,       // means that pipeline makes vpp composition and get data from shared buffer
	VppCompOnlyEncode  // means that pipeline makes vpp composition + encode and get data from shared buffer
};
enum EFieldCopyMode
{
	FC_T2T = 1,
	FC_T2B = 2,
	FC_B2T = 4,
	FC_B2B = 8,
	FC_FR2FR = 16
};
struct sVppCompDstRect
{
	sVppCompDstRect(){ memset(this, 0, sizeof(sVppCompDstRect)); }
	mfxU32 DstX;
	mfxU32 DstY;
	mfxU32 DstW;
	mfxU32 DstH;
};

struct PreEncAuxBuffer
{
	mfxEncodeCtrl     encCtrl;
	mfxU16            Locked;
	mfxENCInput       encInput;
	mfxENCOutput      encOutput;
};

struct ExtendedSurface
{
	mfxFrameSurface1 *pSurface;
	PreEncAuxBuffer  *pCtrl;
	mfxSyncPoint      Syncp;
};

struct ExtendedBS
{
	ExtendedBS() : IsFree(true), Syncp(NULL), pCtrl(NULL)
	{
		memset(&Bitstream, 0, sizeof(mfxBitstream));
		Surface1 = Surface2 = NULL;
	};
	bool IsFree;
	mfxBitstream Bitstream;
	mfxSyncPoint Syncp;
	PreEncAuxBuffer* pCtrl;
	mfxFrameSurface1* Surface1;
	mfxFrameSurface1* Surface2;
};

struct sPlugin
{
	enum sPluginType
	{
		PluginLoad_GUID=1,
		PluginLoad_FILE=2,
	};

	sPlugin(){ memset(this, 0, sizeof(sPlugin)); }
	mfxPluginUID pluginGuid;
	char         pluginPath[MSDK_FILENAME_MAXLEN];
	sPluginType  pluginType;
};

struct sInputParams
{
	sInputParams()
	{ 
		memset(this, 0, sizeof(sInputParams)); 
		priority = MFX_PRIORITY_NORMAL;
		//libType = MFX_IMPL_SOFTWARE;
		bUseHW = false;
		pVppCompDstRects = NULL; 
		//m_hwdev = NULL;
		DenoiseLevel = -1;
		DetailLevel = -1;
	}
	// session parameters
	bool         bIsJoin;
	mfxPriority  priority;
	// common parameters
	//mfxIMPL libType;  // Type of used mediaSDK library
	bool   bUseHW;
	bool   bIsPerf;   // special performance mode. Use pre-allocated bitstreams, output
	mfxU16 nThreadsNum; // number of internal session threads number

	mfxU32 EncodeId; // type of output coded video
	mfxU32 DecodeId; // type of input coded video

	enmMemType  MemType;

	TCHAR  strSrcFile[MSDK_FILENAME_MAXLEN]; // source bitstream file
	TCHAR  strDstFile[MSDK_FILENAME_MAXLEN]; // destination bitstream file

	// specific encode parameters
	mfxU16 nTargetUsage;//MFX_TARGETUSAGE_BALANCED
	mfxF64 dFrameRate;
	mfxF64 dEncoderFrameRate;
	mfxU32 nBitRate;
	mfxU16 nQuality; // quality parameter for JPEG encoder
	mfxU16 nDstWidth;  // destination picture width, specified if resizing required
	mfxU16 nDstHeight; // destination picture height, specified if resizing required

	bool bEnableDeinterlacing;
	mfxU16 DeinterlacingMode;    // MFX_DEINTERLACING_ADVANCED_NOREF
	int DenoiseLevel;  // 0<=x<=100   
	int DetailLevel;  // 0<=x<=100
	mfxU16 FRCAlgorithm;//frame rate conversion algorithm: MFX_FRCALGM_PRESERVE_TIMESTAMP
	EFieldCopyMode fieldProcessingMode;

	mfxU16 nAsyncDepth; // asyncronous queue

	PipelineMode eMode;
	PipelineMode eModeExt;

	mfxU32 numSurf4Comp;

	mfxU16 nSlices; // number of slices for encoder initialization
	mfxU16 nMaxSliceSize; //maximum size of slice

	mfxU16 WinBRCMaxAvgKbps;
	mfxU16 WinBRCSize;
	mfxU16 BufferSizeInKB;
	mfxU16 GopPicSize;
	mfxU16 GopRefDist;
	mfxU16 NumRefFrame;

	// MVC Specific Options
	bool   bIsMVC; // true if Multi-View-Codec is in use
	mfxU32 numViews; // number of views for Multi-View-Codec

	mfxU16 nRotationAngle; // if specified, enables rotation plugin in mfx pipeline
	TCHAR  strVPPPluginDLLPath[MSDK_FILENAME_MAXLEN]; // plugin dll path and name

	sPlugin decoderPluginParams;
	sPlugin encoderPluginParams;

	mfxU32 nTimeout; // how long transcoding works in seconds
	mfxU32 nFPS; // limit transcoding to the number of frames per second

	mfxU32 statisticsWindowSize;

	bool bLABRC; // use look ahead bitrate control algorithm
	mfxU16 nLADepth; // depth of the look ahead bitrate control  algorithm
	bool bEnableExtLA;
	bool bEnableBPyramid;
	mfxU16 nRateControlMethod;//MFX_RATECONTROL_CQP
	mfxU16 nQPI;
	mfxU16 nQPP;
	mfxU16 nQPB;

	bool bOpenCL;
	mfxU16 reserved[4];

	mfxU16 nVppCompDstX;
	mfxU16 nVppCompDstY;
	mfxU16 nVppCompDstW;
	mfxU16 nVppCompDstH;

	mfxU32 DecoderFourCC;
	mfxU32 EncoderFourCC;

	sVppCompDstRect* pVppCompDstRects;

	bool bUseOpaqueMemory;
	bool bForceSysMem;
	mfxU16 nGpuCopyMode;

	mfxU16 nRenderColorForamt; /*0 NV12 - default, 1 is ARGB*/

	mfxI32  monitorType;
	bool shouldUseGreedyFormula;
	bool enableQSVFF;   //MFX_CODINGOPTION_ON
	//CHWDevice             *m_hwdev;
};

class sStrQue
{
public:
	sStrQue()
	{
		vecList.clear();
		if (vecList1)
			vecList1->clear();
		lock = NULL;
		bBind = false;
	}
	~sStrQue()
	{
		if (lock)
			EnterCriticalSection(lock);
		vecList.clear();
		if (vecList1)
			vecList1->clear();
		if (lock)
			LeaveCriticalSection(lock);
	}
	void* queGet()
	{
		while (true)
		{
			static UINT64 time = GetTickCount64(), time1 = 0;
			if (vecList.size() > 0)
			{
				if (lock)
					EnterCriticalSection(lock);
				void* ptr = vecList[0]; 
				vecList.erase(vecList.begin());
				if (lock)
					LeaveCriticalSection(lock);
				return ptr;
			}
			else
			{
				Sleep(2);
				time1 = GetTickCount64();
				if (time1 - time > 50)
				{
					time = time1;
					break;
				}
			}
		}
		return NULL;
	}
	void quePut(void* value)
	{
		if (lock)
			EnterCriticalSection(lock);
#ifdef _DEBUG
		for (int i = 0; i < (int)vecList1->size(); i++)
		{
			if (value == (*vecList1)[i])
				break;
		}
#endif 
		vecList1->push_back(value);
		if (lock)
			LeaveCriticalSection(lock);
	}

protected:
	vector<void*>        vecList;
	vector<void*>*       vecList1;
	CRITICAL_SECTION*    lock;
	bool                 bBind;//绑定到下一个流程的标识
};
class sStrQueEx : public sStrQue
{
public:
	virtual void connect(sStrQueEx* other)
	{
		vecList1 = &other->vecList;
		lock = other->lock;
	}
	void Bind(bool bind = true){ bBind = bind; }
	bool IsBinded(){ return bBind; }
};
class sStrQueExt : public sStrQueEx
{
public:
	sStrQueExt()
	{
		lock = new CRITICAL_SECTION;
		if (lock)
			InitializeCriticalSection(lock);
	}
	~sStrQueExt()
	{
		if (lock)
		{
			DeleteCriticalSection(lock);
			delete lock;
			lock = NULL;
		}
	}
	virtual void connect(sStrQueExt* other)
	{
		vecList1 = &other->vecList;
	}
	void initList(vector<void*> list)
	{
		if (lock)
			EnterCriticalSection(lock);
		vecList = list;
		if (lock)
			LeaveCriticalSection(lock);
	}

	void quePutSelf(void* value)
	{
		if (lock)
			EnterCriticalSection(lock);
#ifdef _DEBUG
		for (int i = 0; i < (int)vecList.size(); i++)
		{
			if (value == vecList[i])
				break;
		}
#endif 
		vecList.push_back(value); 
		if (lock)
			LeaveCriticalSection(lock);
	}
};