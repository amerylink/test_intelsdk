#ifndef SAMPLE_VPP_H_
#define SAMPLE_VPP_H_

//#include "linkCommon.h"

#include "sample_vpp_utils.h"
// #ifndef WIN32
// #else
// #include <windows.h>
// Uint32 iStartTime;
// #endif

struct sVppDataInfo
{
	  mfxU16  x;
	  mfxU16  y;
	  mfxU16  cropw;
	  mfxU16  croph;
	  mfxU16  w;
	  mfxU16  h;
	  mfxU16  pitch;  
};

struct sVppPosInfo
{
	  mfxU16  CropX;
	  mfxU16  CropY;
	  mfxU16  CropW;
	  mfxU16  CropH;	
	  mfxU16  Width;
	  mfxU16  Height;	
};

struct sVppUsrParams
{
	sVppPosInfo srcPos;
	sVppPosInfo dstPos;
	mfxU16        numStreams;
	/* Video Enhancement Algorithms */
	sDIParam            deinterlaceParam;
	sDenoiseParam       denoiseParam;
	sDetailParam        detailParam;
	sProcAmpParam       procampParam;
	sVideoAnalysisParam vaParam;
	sIStabParam         istabParam;
	sCompositionParam   compositionParam;
	sFrameRateConversionParam frcParam;
	sAppResources appResources;
};

struct sVppUsrBuffParams
{
	mfxFrameSurface1 *pInSurfaces;
	mfxFrameSurface1 *pOutSurfaces;
};

//Hardware vpp Initial
int API_IntelMedia_SDK_HW_VppInitial(void *lp);

//Hardware vpp Release
int API_IntelMedia_SDK_HW_VppRelease(void *lp);

//Hardware vpp Reset
int API_IntelMedia_SDK_HW_VppReset(void *lp);

//Hardware vpp Frame
int API_IntelMedia_SDK_HW_VppProcessFrame(void *lp,void *buffInfo);

#ifdef WIN32
unsigned int vpp_get_run_time(void);
void	vpp_set_run_time();
#endif

#endif
