#ifndef _SWMS_LINK_H_
#define _SWMS_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"
#include "SDK_codec.h"
#include "sample_vpp.h"

#define SWMS_LINK_MAX_ALLOC_BUFF_NUM 10
#define SWMS_LINK_DEFAULT_ALLOC_BUFF_NUM 10
#define SWMS_LINK_MSG_NUM					10
#define SYSTEM_SW_MS_MAX_WIN           (10)
#define SYSTEM_SW_MS_MAX_CH_ID         (10)

#define ALG_LINK_OSD_MAX_WINDOWS (5)


/**
  \brief OSD window parameter's
*/
typedef struct
{
	Uint8  *AlpBuff[2][2];
	Uint8  *addr[2][2];
	/**< OSD buffer pointer

	MUST be physical address

	When data format is YUV422
	addr[0][0] - YUV422 plane pointer

	When data format is YUV420
	addr[0][0] - Y-plane pointer
	addr[0][1] - C-plane pointer

	Other arrary element not used.
	*/

	Uint32 format;
	/**< SYSTEM_DF_YUV420SP_UV or SYSTEM_DF_YUV422I_YUYV

	SYSTEM_DF_YUV422I_YUYV
	- ONLY SUPPORTED in DM816x, DM814x

	SYSTEM_DF_YUV420SP_UV
	- SUPPORTED in ALL platforms
	*/

	Uint32 startX;
	/**< Start position of window in X direction,
	specified in pixels,
	relative to start of video window,
	must be multiple of 2
	*/

	Uint32 startY;
	/**< Start position of window in Y direction,
	specified in lines,
	relative to start of video window,
	must be multiple of 2
	*/

	Uint32 width;
	/**< Width of window,
	specified in pixels,
	must be multiple of 4
	*/

	Uint32 height;
	/**< Height of window,
	specified in pixels,
	must be multiple of 4
	*/


	Uint32 lineOffset;
	/**<
	Line offset in pixels,
	must be >= width, must be multiple of 4,
	recommended to be multiple of 32 for DDR efficiency
	*/

	Uint32 globalAlpha;
	/**<
	8-bit global Alpha Value, used only if Alpha window is not enabled,
	Set to 0 to disable alpha blend.

	0   : Min Alpha, show only video
	0x80: Max Alpha, show only OSD

	Supported in DM816x, DM814x
	- on per CH, per Window basis.
	- can be changed dynamically

	Supported in DM810x with the following restrictions,
	- Value specified for CH0, WIN0 is applied for all CHs and all windows
	- Value specified during create time is used and cannot be updated at run-time
	*/

	Uint32 transperencyEnable;
	/**<
	TRUE: enable transperency,

	when OSD pixel = colorKey
	then
	    video is shown
	else
	    OSD pixel is blend with Video taking into account globalAlpha

	FALSE: disable transperency

	OSD pixel is always blended with Video taking into account globalAlpha

	Supported in DM816x, DM814x
	- on per CH, per Window basis.
	- can be changed dynamically

	Supported in DM810x with the following restrictions,
	- Value specified for CH0, WIN0 is applied for all CHs and all windows
	- Value specified during create time is used and cannot be updated at run-time
	*/

	Uint32 enableWin;
	/**< TRUE: Enable display of OSD window for the channel
	FALSE: Disable display of OSD window for the channel
	*/

	Uint32  colorKey[3];
	/**< Color key for Y, U, V.
	This is used when AlgLink_OsdWindowPrm.transperencyEnable = TRUE
	*/

	Bool	   updateParm;

} AlgLink_OsdWindowPrm;

/**
    \brief OSD channel - window parameters

    Specifies OSD parameters that can be changed dynamically
    on a per channel basis

    See structure/field details for restrictions applicable for DM810x platform.
*/
typedef struct AlgLink_OsdChWinParams
{
	//Uint32 chId;
	/**< OSD channel number.
	     Valid values: 0 .. ALG_LINK_OSD_MAX_CH-1
	*/

	Uint32 numWindows;
	/**< Number of OSD windows for this channel.
	     Valid values: 0..ALG_LINK_OSD_MAX_WINDOWS
	*/

	AlgLink_OsdWindowPrm winPrm[ALG_LINK_OSD_MAX_WINDOWS];
	/**< OSD window parameters */

} AlgLink_OsdChWinParams;

typedef struct AlgLink_OsdParams
{
	Bool					     enableOsd;
	AlgLink_OsdWindowPrm      winRtPrm[ALG_LINK_OSD_MAX_WINDOWS];
	AlgLink_OsdChWinParams  osdCreateWinParams;
} AlgLink_OsdParams;


typedef struct
{
    Uint32 channelNum;
    /* channel associated with this window */

    Uint32 bufAddrOffset; /* NOT TO BE SET BY USER */

    Uint32 startX;
    Uint32 startY;
    Uint32 width;
    Uint32 height;

    Uint32 bypass; /* TRUE/FALSE - Low Cost Scaling/DEI enable disable */
     
} SwMsLink_LayoutWinInfo;

typedef struct
{
    Uint32 onlyCh2WinMapChanged;
    /* FALSE: Layout is changed
         TRUE: Layout is not changed only Channels mapped to windows are changed
     */

    Uint32 numWin;
    /**< Display Layout Number of Windows */

    SwMsLink_LayoutWinInfo winInfo[SYSTEM_SW_MS_MAX_WIN];
    /**< Display Layout Individual window coordinates */

    Uint32  ch2WinMap[SYSTEM_SW_MS_MAX_CH_ID];
    /**< Display Layout Channel to Window  Mapping - NOT TO BE SET BY USER */

    Uint32 outputFPS;
    /**< Rate at which output frames should be generated,
         should typically be equal to the display rate

         Example, for 60fps display, outputFPS should be 60
    */
    Uint32 prev_outputFPS;

} SwMsLink_LayoutPrm;

typedef struct SwMsLink_chObj
{
	LINK_EmpFullQueHndl inQue;
   	FRAME_Buf *pCurInFrame;
    Bool         isPlaybackChannel;
    Bool chRtInInfoUpdate;
    Bool chRtInfoUpdate;
} SwMsLink_chObj;

/*swms link 设置参数*/
typedef struct{
	Uint32 dstX;
	Uint32 dstY;
	Uint32 dstWidth;
	Uint32 dstHeight;
}SWMS_SET_INFO;

/*swms link 创建参数*/
typedef struct{
	Uint32 maxOutRes;
	SwMsLink_LayoutPrm layoutPrm;
	SwMsLink_LayoutPrm rtlayoutPrm;
	sVppUsrParams vppUsrParams[SYSTEM_SW_MS_MAX_WIN];
	Uint32 queLen;
	Uint32 timerPeriod;/*周期处理时间ms*/
}SWMS_CREATE_INFO;

/*swms link的数据信息*/
typedef struct{
	Bool			tskEnable;							//启动任务
	Bool			exitTsk;								//退出任务
	Uint32	idx;
	pthread_mutex_t lock;
	SWMS_CREATE_INFO createInfo;                     //创建参数
	sAPI_FrameSurfaceAllocParams AllocFramePara;		//分配frame参数
	LINK_TskHndl tskHandle;							//任务句柄
	LINK_TskHndl tskHandle_msg;	
	LINK_EmpFullQueHndl *PreEmpFullQueHndl;	//帧空满包队列句柄
	FRAME_Buf 			frameBuff[SWMS_LINK_MAX_ALLOC_BUFF_NUM];
	LINK_EmpFullQueHndl EmpFullQueHndl;				//空满包队列句柄
	LINK_EmpFullQueHndl MsgQueHandle;
	SwMsLink_chObj chObj[SYSTEM_SW_MS_MAX_CH_ID];
	Uint32			     Msg[SWMS_LINK_MSG_NUM];
	FRAME_Buf 			midFrameBuff;
	/*osd*/
	AlgLink_OsdParams osdParams;
}SWMS_LINK_INFO;

Int32 swmsLink_osdParaInit(AlgLink_OsdParams *Info);

Int32 swmsLink_createParaInit(SWMS_CREATE_INFO *createInfo);
Int32 swmsLink_create(SWMS_LINK_INFO *info);
Int32 swmsLink_release(SWMS_LINK_INFO *info);
Int32 swmsLink_enable(SWMS_LINK_INFO *info);
Int32 swmsLink_disable(SWMS_LINK_INFO *info);
Int32 swmsLink_setParam(SWMS_LINK_INFO *info,SWMS_SET_INFO *setPara);
Int32 swmsLink_connect(SWMS_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);
void swmsLink_printf(SWMS_LINK_INFO *info);
Int32 SwMsLink_drvSwitchLayout(SWMS_LINK_INFO *info, SwMsLink_LayoutPrm *pram);
Int32 swmsLink_setOsdWinParam(SWMS_LINK_INFO *info,AlgLink_OsdWindowPrm *Para,Int32 winId);
#endif
