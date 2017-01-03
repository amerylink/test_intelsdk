#ifndef _REACH_VIDEO_RECV_H_
#define _REACH_VIDEO_RECV_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

//#include "EncDefine.h" 
#include "linkVideo.h"
#include "SDK_codec.h"
#include "../inc/dec_link/decLink.h"
#include "../inc/enc_link/encLink.h"
#include "../inc/merge_link/mergeLink.h"
#include "../inc/sclr_link/sclrlink.h"
#include "../inc/dup_link/dupLink.h"
#include "../inc/alg_link/algLink.h"
#include "../inc/swms_link/swmsLink.h"
#include "../inc/nullsrc_link/nullsrcLink.h"
#include "encoder.h"


#define BITSOUT_LINK_IP_LEN 20
		
/*码流接收的数据信息*/
typedef struct{
	Uint32 QueLen;
	LINK_TskHndl tskHandle;							//任务句柄
	Bool			tskEnable;							//启动任务
	Int8               ip[BITSOUT_LINK_IP_LEN];
	Int32		chNo;
	Int32		ipcProcType;
	Int32		port;
	char			usrName[20];
	char			passWd[20];
//	enum_EncProtocolType           ProtocolType;
	Bitstream_Buf BitstreamBuff[VIDBITSTREAM_MAX_BITSTREAM_BUFS];
	LINK_EmpFullQueHndl BistEmpFullQueHndl;				//空满包队列句柄
	Int32     RecvFrameOffset;
	Uint32   frameCount;
	Bitstream_Buf *pCurBitstreamBuff;
	long		sdk;
}BITSOUTLINK_INFO;


#ifndef XL
#define XL

//reachApp
#define APP_TSK_PRI 50
#define DEC_NUM		1

enum
{
	LIVE_VGA_INDEX = 0,
	LIVE_MH_INDEX = 1,
	LIVE_ML_INDEX = 2,
	LIVE_IPC1_INDEX = 3,
	LIVE_IPC2_INDEX = 4,
	LIVE_AUDIO_INDEX = 5,
	RECORD_VGA_INDEX = 6,
	RECORD_MH_INDEX = 7,
	RECORD_ML_INDEX = 8,
	RECORD_IPC1_INDEX = 9,
	RECORD_IPC2_INDEX = 10,
	RECORD_PPT_INDEX = 11,
	RECORD_AUDIO_INDEX = 12,
	MODULE_MAX
};

typedef struct
{
	Int32 port;  //发送端口号
	Int32 index;//发送index,与port对应
}VIDEO_PORT_INFO;


typedef struct
{

 	ALG_LINK_INFO algLinkInfo;
	// 
 	DUP_LINK_INFO dupLinkInfo[2];
	// 
 	SWMS_LINK_INFO swmsLinkInfo;
	// 
 	SCLR_LINK_INFO sclrLinkInfo[3];
	// 
	// 	//SCLR_SET_INFO sclrSetInfo[2];
	// 
 	NULLSRC_LINK_INFO nullSrcLinkInfo;
 
 	ENC_LINK_INFO encLinkInfo[ENC_NUM];
 
 	DEC_LINK_INFO decLinkInfo[DEC_NUM];

	BITSOUTLINK_INFO bitsOutLinkInfo[48];

 	MERGE_LINK_INFO mergeLinkInfo;
 
// 	SELECT_LINK_INFO selectLinkInfo;
// 	SelectLink_OutQueChInfo selectOutQueChInfo;
// 	ADCAPTURE_LINK_INFO adcaptureLinkInfo;
// 	AUDIOENC_LINK_INFO audioencLinkInfo;

	Int32 SCD_PPT_status;

	void *RtspHDLiveHandle;
	void *RtspSDLiveHandle;

}Edukit_App_T;




#endif


Int32 bitsOutLink_enable(void* arg);
Int32 bitsOutLink_init(void* arg);
Int32 bitsOutLink_disable(void* arg);
Int32 bitsOutLink_uninit(void* arg);

#endif
